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

#include <gmock/gmock.h>

#include <iostream>
#include <memory>

#include "HerkusBus.hpp"
#include "spdlog/spdlog.h"

namespace HerkusBuxMockedExample {

class HerkusBusMock : public Herkus::IHerkusBus {
 public:
  MOCK_METHOD(void, Publish, (const std::string& topic, const Herkus::json& message_payload), (override));
  MOCK_METHOD(void, Subscribe, (const std::string& topic, Herkus::subscriber_callback sub_callback), (override));
};

class ClassWithHerkusBus {
 public:
  ClassWithHerkusBus() : ClassWithHerkusBus(Herkus::HerkusBus::getInstance()) {}

 protected:
  explicit ClassWithHerkusBus(Herkus::IHerkusBus& herkus_bus) : herkus_bus_(herkus_bus) {}

  void MethodThatCallsHerkusBusPublish(const std::string& topic, const Herkus::json& message_payload) {
    spdlog::info("Publishing message to topic: {}", topic);
    herkus_bus_.Publish(topic, message_payload);
  }

  void MethodThatCallsHerkusBusSubscribe(const std::string& topic, Herkus::subscriber_callback sub_callback) {
    spdlog::info("Subscribing to topic: {}", topic);
    herkus_bus_.Subscribe(topic, std::move(sub_callback));
  }

 private:
  Herkus::IHerkusBus& herkus_bus_;
};

class ClassWithMockedBus : public ClassWithHerkusBus {
 public:
  ClassWithMockedBus(Herkus::IHerkusBus& herkus_bus) : ClassWithHerkusBus(herkus_bus) {}

  using ClassWithHerkusBus::MethodThatCallsHerkusBusPublish;
  using ClassWithHerkusBus::MethodThatCallsHerkusBusSubscribe;
};

}  // namespace HerkusBuxMockedExample

using namespace testing;

int main() {
  StrictMock<HerkusBuxMockedExample::HerkusBusMock>* mock_bus_mock = new StrictMock<HerkusBuxMockedExample::HerkusBusMock>();
  HerkusBuxMockedExample::ClassWithMockedBus class_with_mocked_bus(*mock_bus_mock);
  HerkusBuxMockedExample::ClassWithHerkusBus class_with_not_mocked_herkus_bus;

  EXPECT_CALL(*mock_bus_mock, Publish("topic1", Herkus::json::object({{"key", "value"}})));
  EXPECT_CALL(*mock_bus_mock, Subscribe("topic2", _));

  class_with_mocked_bus.MethodThatCallsHerkusBusPublish("topic1", Herkus::json::object({{"key", "value"}}));
  class_with_mocked_bus.MethodThatCallsHerkusBusSubscribe("topic2", [](const std::string& topic, const Herkus::json& msg) {
    std::cout << "Received message on topic: " << topic << " with payload: " << msg.dump() << std::endl;
  });

  delete mock_bus_mock;

  return 0;
}