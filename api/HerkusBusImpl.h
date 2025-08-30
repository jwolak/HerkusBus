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

#pragma once

#include <string>
#include <functional>
#include <memory>

#include "nlohmann/json.hpp"
#include "spdlog/sinks/rotating_file_sink.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/deque.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <thread>
#include <chrono>

namespace Herkus
{
    using json = nlohmann::json;
    using subscriber_callback = std::function<void(const std::string &topic, const json &msg)>;

    struct Message
    {
        std::string topic;
        std::string payload;
    };

    using shared_mem_allocator = boost::interprocess::allocator<Message, boost::interprocess::managed_shared_memory::segment_manager>;
    using shared_mem_message_deque = boost::interprocess::deque<Message, shared_mem_allocator>;

    class HerkusBusImpl
    {
    public:
        HerkusBusImpl();
        ~HerkusBusImpl();

        void Publish(const std::string &topic, const json &message_payload);
        void Subscribe(const std::string &topic, subscriber_callback sub_callback);

    private:
        boost::interprocess::managed_shared_memory shared_memory_segment_;
        shared_mem_message_deque *message_queue_;
        boost::interprocess::interprocess_mutex *ipc_mtx_;
        boost::interprocess::interprocess_condition *ipc_condition_variable_;
        std::thread bus_event_loop_thread_;
        bool stop_listener_event_loop_;
        std::unordered_map<std::string, std::vector<subscriber_callback>> subscribers_callbacks_;
        std::mutex subscribers_mutex_;
    };

} // namespac Herkus
