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

#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

#include "HerkusBus.hpp"

void PublishLoop(const std::string& topic, int threadId) {
  auto& bus = Herkus::HerkusBus::getInstance();
  for (int i = 0; i < 3; ++i) {
    Herkus::json payload;
    payload["ThreadId"] = threadId;
    payload["MessageId"] = i;
    bus.Publish(topic, payload);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main() {
  constexpr const char* kTopicName = "example_topic";
  constexpr int kThreadId_1 = 1;
  constexpr int kThreadId_2 = 2;
  constexpr int kMessagesPerThread = 3;
  constexpr int kTotalMessages = kMessagesPerThread * 2;
  constexpr auto kWaitTimeout = std::chrono::seconds(2);

  auto& bus = Herkus::HerkusBus::getInstance();

  std::promise<void> all_received;
  auto all_received_future = all_received.get_future();
  std::atomic<int> received_count{0};
  std::once_flag completion_flag;

  std::cout << "Subscribe to: " << kTopicName << std::endl;
  bus.Subscribe(kTopicName, [&](const std::string& topic, const Herkus::json& msg) {
    std::cout << "[Example] Received on [" << topic << "]: " << msg.dump() << std::endl;
    const int current = received_count.fetch_add(1) + 1;
    if (current == kTotalMessages) {
      std::call_once(completion_flag, [&]() { all_received.set_value(); });
    }
  });

  std::thread t1(PublishLoop, kTopicName, kThreadId_1);
  std::thread t2(PublishLoop, kTopicName, kThreadId_2);

  t1.join();
  t2.join();

  if (all_received_future.wait_for(kWaitTimeout) != std::future_status::ready) {
    std::cerr << "[Example] Timeout waiting for messages" << std::endl;
    return 1;
  }

  return 0;
}