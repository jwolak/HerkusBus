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

#include <algorithm>
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
const unsigned int kDefaultWorkerThreads = 0;
}  // namespace

HerkusBusImpl::HerkusBusImpl()
    : bus_event_loop_thread_{},
      stop_listener_event_loop_{false},
      worker_threads_{},
      task_queue_{},
      task_queue_mutex_{},
      task_queue_cv_{},
      stop_worker_threads_{false},
      subscribers_callbacks_{},
      subscribers_mutex_{} {
  boost::interprocess::shared_memory_object::remove(kSharedMemoryName.c_str());
  spdlog::debug("Shared memory segment removed [{0}:{1}]", __FILENAME__, __LINE__);
  shared_memory_segment_ = managed_shared_memory{open_or_create, kSharedMemoryName.c_str(), kSharedMemorySize};
  message_queue_ = shared_memory_segment_.find_or_construct<shared_mem_message_deque>(kMessageQueueName.c_str())(
      shared_mem_allocator(shared_memory_segment_.get_segment_manager()));
  ipc_mtx_ = shared_memory_segment_.find_or_construct<interprocess_mutex>(kIpcMutexName.c_str())();
  ipc_condition_variable_ = shared_memory_segment_.find_or_construct<interprocess_condition>(kIpcConditionVariableName.c_str())();

  const unsigned int worker_count = kDefaultWorkerThreads == 0 ? std::max(1u, std::thread::hardware_concurrency()) : std::max(1u, kDefaultWorkerThreads);
  spdlog::debug("Create worker thread pool (size={0})... [{1}:{2}]", worker_count, __FILENAME__, __LINE__);
  worker_threads_.reserve(worker_count);
  for (unsigned int i = 0; i < worker_count; ++i) {
    worker_threads_.emplace_back([this]() {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(task_queue_mutex_);
          task_queue_cv_.wait(lock, [this]() { return stop_worker_threads_.load() || !task_queue_.empty(); });
          if (stop_worker_threads_.load() && task_queue_.empty()) {
            return;
          }
          task = std::move(task_queue_.front());
          task_queue_.pop();
        }
        task();
      }
    });
  }

  spdlog::debug("Create bus event loop thread... [{0}:{1}]", __FILENAME__, __LINE__);
  bus_event_loop_thread_ = std::thread([this]() {
    while (!stop_listener_event_loop_.load()) {
      scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
      spdlog::debug("Event loop checks if message queue is empty... [{0}:{1}]", __FILENAME__, __LINE__);
      if (message_queue_->empty()) {
        spdlog::debug("No message in queue [{0}:{1}]", __FILENAME__, __LINE__);
        spdlog::debug("Waiting... [{0}:{1}]", __FILENAME__, __LINE__);
        ipc_condition_variable_->wait(lock);
        if (stop_listener_event_loop_.load()) {
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
        const auto callbacks = [&] {
          std::lock_guard<std::mutex> lock(subscribers_mutex_);
          auto it = subscribers_callbacks_.find(msg.topic);
          if (it == subscribers_callbacks_.end()) {
            return std::vector<subscriber_callback>{};
          }
          return it->second;
        }();
        for (const auto& callback : callbacks) {
          EnqueueTask([callback, topic = msg.topic, parsed_msg]() mutable { callback(topic, parsed_msg); });
        }
        lock.lock();
      }
    }
  });
}

HerkusBusImpl::~HerkusBusImpl() {
  stop_listener_event_loop_.store(true);
  spdlog::debug("Notify bus event loop to be stopped [{0}:{1}]", __FILENAME__, __LINE__);
  ipc_condition_variable_->notify_all();

  if (bus_event_loop_thread_.joinable()) {
    bus_event_loop_thread_.join();
  }

  stop_worker_threads_.store(true);
  task_queue_cv_.notify_all();
  for (auto& worker : worker_threads_) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  boost::interprocess::shared_memory_object::remove(kSharedMemoryName.c_str());
  spdlog::debug("Shared memory segment removed [{0}:{1}]", __FILENAME__, __LINE__);
}

void HerkusBusImpl::EnqueueTask(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(task_queue_mutex_);
    if (stop_worker_threads_.load()) {
      return;
    }
    task_queue_.push(std::move(task));
  }
  task_queue_cv_.notify_one();
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