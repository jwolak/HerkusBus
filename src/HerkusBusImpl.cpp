/*
 * HerkusBusImpl.h
 *
 *  Created on: 2025
 *      Author: Janusz Wolak
 */

/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2025, Janusz Wolak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "HerkusBusImpl.h"

#include <spdlog/spdlog.h>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <chrono>
#include <thread>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

namespace Herkus {
using namespace boost::interprocess;

namespace {
const int32_t kSharedMemorySize = 65536;  // 64 KB
const std::string kSharedMemoryName = "HerkusBusSharedMemory";
const std::string kMessageQueueName = "HerkusMessageQueue";
const std::string kIpcMutexName = "HerkusIpcMutex";
const std::string kIpcConditionVariableName = "HerkusIpcConditionVariable";
}  // namespace

HerkusBusImpl::HerkusBusImpl() : bus_event_loop_thread_{}, stop_listener_event_loop_{false}, subscribers_callbacks_{}, subscribers_mutex_{} {
  spdlog::set_level(spdlog::level::debug);
  boost::interprocess::shared_memory_object::remove(kSharedMemoryName.c_str());
  spdlog::debug("Shared memory segment removed [{0}:{1}]", __FILENAME__, __LINE__);
  shared_memory_segment_ = managed_shared_memory{open_or_create, kSharedMemoryName.c_str(), kSharedMemorySize};
  message_queue_ = shared_memory_segment_.find_or_construct<shared_mem_message_deque>(kMessageQueueName.c_str())(
      shared_mem_allocator(shared_memory_segment_.get_segment_manager()));
  ipc_mtx_ = shared_memory_segment_.find_or_construct<interprocess_mutex>(kIpcMutexName.c_str())();
  ipc_condition_variable_ = shared_memory_segment_.find_or_construct<interprocess_condition>(kIpcConditionVariableName.c_str())();

  spdlog::debug("Create bus event loop thread... [{0}:{1}]", __FILENAME__, __LINE__);
  bus_event_loop_thread_ = std::thread([this]() {
    while (!stop_listener_event_loop_) {
      scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
      spdlog::debug("Event loop checks if message queue is empty... [{0}:{1}]", __FILENAME__, __LINE__);
      if (message_queue_->empty()) {
        spdlog::debug("No message in queue [{0}:{1}]", __FILENAME__, __LINE__);
        spdlog::debug("Waiting... [{0}:{1}]", __FILENAME__, __LINE__);
        ipc_condition_variable_->wait(lock);
        if (stop_listener_event_loop_) {
          spdlog::debug("Event loop stopped [{0}:{1}]", __FILENAME__, __LINE__);
          break;
        }
      }
      spdlog::debug("Message queue is not empty... [{0}:{1}]", __FILENAME__, __LINE__);
      while (!message_queue_->empty()) {
        spdlog::debug("New message in queue [{0}:{1}]", __FILENAME__, __LINE__);
        Message msg = message_queue_->front();
        spdlog::debug("Get message from queue [{0}:{1}]", __FILENAME__, __LINE__);
        message_queue_->pop_front();
        spdlog::debug("Message removed from queue as already taken [{0}:{1}]", __FILENAME__, __LINE__);
        lock.unlock();

        spdlog::debug("Parsing message... [{0}:{1}]", __FILENAME__, __LINE__);
        auto parsed_msg = json::parse(msg.payload);

        spdlog::debug("Call callbacks for all subscribers subscribed on topic: [{0}] [{1}:{2}]", msg.topic, __FILENAME__, __LINE__);
        auto it = [&] {
          std::lock_guard<std::mutex> lock(subscribers_mutex_);
          return subscribers_callbacks_.find(msg.topic);
        }();
        if (it != subscribers_callbacks_.end()) {
          for (const auto& callback : it->second) {
            callback(msg.topic, parsed_msg);
          }
        }
        lock.lock();
      }
    }
  });
}

HerkusBusImpl::~HerkusBusImpl() {
  stop_listener_event_loop_ = true;
  scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
  spdlog::debug("Notify bus event loop to be stopped [{0}:{1}]", __FILENAME__, __LINE__);
  ipc_condition_variable_->notify_all();
  if (bus_event_loop_thread_.joinable()) {
    bus_event_loop_thread_.join();
  }
}

void HerkusBusImpl::Publish(const std::string& topic, const json& message_payload) {
  spdlog::debug("Publish message on topic: {0} [{1}:{2}]", topic, __FILENAME__, __LINE__);
  std::string payload = message_payload.dump();
  scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
  spdlog::debug("Add message to message queue [{0}:{1}]", __FILENAME__, __LINE__);
  message_queue_->emplace_back(Message{topic, payload});
  spdlog::debug("Notify bus event loop about new published message... [{0}:{1}]", __FILENAME__, __LINE__);
  ipc_condition_variable_->notify_one();
}

void HerkusBusImpl::Subscribe(const std::string& topic, subscriber_callback sub_callback) {
  spdlog::debug("Subscribe on topic: {0} [{1}:{2}]", topic, __FILENAME__, __LINE__);
  std::lock_guard<std::mutex> lock(subscribers_mutex_);
  spdlog::debug("Add new subscriber callback to callbacks list [{0}:{1}]", topic, __FILENAME__, __LINE__);
  subscribers_callbacks_[topic].push_back(std::move(sub_callback));
}
}  // namespace Herkus