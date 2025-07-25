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

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <thread>
#include <chrono>

namespace Herkus
{
    using namespace boost::interprocess;

    namespace
    {
        const int32_t kSharedMemorySize = 65536;
        const std::string kSharedMemoryName = "HerkusBusSharedMemory";
        const std::string kMessageQueueName = "HerkusMessageQueue";
        const std::string kIpcMutexName = "HerkusIpcMutex";
        const std::string kIpcConditionVariableName = "HerkusIpcConditionVariable";
    }

    HerkusBusImpl::HerkusBusImpl() : shared_memory_segment_{open_or_create, kSharedMemoryName.c_str(), kSharedMemorySize},
                                     message_queue_{shared_memory_segment_.find_or_construct<shared_mem_message_deque>(kMessageQueueName.c_str())(shared_mem_allocator(shared_memory_segment_.get_segment_manager()))},
                                     ipc_mtx_{shared_memory_segment_.find_or_construct<interprocess_mutex>(kIpcMutexName.c_str())()},
                                     ipc_condition_variable_{shared_memory_segment_.find_or_construct<interprocess_condition>(kIpcConditionVariableName.c_str())()}
    {
        bus_event_loop_thread_ = std::thread([this]() {
            while (!stop_listener_event_loop_) {
                scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
                if(message_queue_->empty()) {
                    ipc_condition_variable_->wait(lock);
                    if(!stop_listener_event_loop_) {
                        break;
                    }
                }
                while (!message_queue_->empty()) {
                    Message msg = message_queue_->front();
                    message_queue_->pop_front();
                    lock.unlock();

                    auto parsed_msg = json::parse(msg.payload);
                    auto it = subscribers_callbacks_.find(msg.topic);
                    if (it != subscribers_callbacks_.end()) {
                        for (const auto &callback : it->second) {
                            callback(msg.topic, parsed_msg);    
                        }
                    } 
                    lock.lock(); 
                }
            }
        });
    }

    HerkusBusImpl::~HerkusBusImpl()
    {
        stop_listener_event_loop_ = false;
        scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
        ipc_condition_variable_->notify_all();
        if (bus_event_loop_thread_.joinable()) {
            bus_event_loop_thread_.join();
        }
    }

    void HerkusBusImpl::publish(const std::string &topic, const json &message_payload)
    {
        std::string payload = message_payload.dump();
        scoped_lock<interprocess_mutex> lock(*ipc_mtx_);
        message_queue_->emplace_back(Message{topic, payload});
        ipc_condition_variable_->notify_one();
    }

    void HerkusBusImpl::subscribe(const std::string &topic, callback call_bck)
    {
        subscribers_callbacks_[topic].push_back(std::move(call_bck));
    }
} // namespace Herkus