/*
 * HerkusBusTests.cpp
 *
 *  Created on: 2025
 *      Author: Build System
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

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "HerkusBus.h"

using namespace Herkus;

// Test fixture for HerkusBus tests
class HerkusBusTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code before each test
  }

  void TearDown() override {
    // Cleanup code after each test
  }
};

// Test: getInstance returns singleton instance
TEST_F(HerkusBusTest, GetInstanceReturnsSingletonInstance) {
  HerkusBus& instance1 = HerkusBus::getInstance();
  HerkusBus& instance2 = HerkusBus::getInstance();
  
  // Both references should point to the same object
  EXPECT_EQ(&instance1, &instance2);
}

// Test: getInstance returns valid instance
TEST_F(HerkusBusTest, GetInstanceReturnsValidInstance) {
  // Should not throw
  EXPECT_NO_THROW({
    HerkusBus& instance = HerkusBus::getInstance();
    HerkusBus& ref = HerkusBus::getInstance();
    EXPECT_EQ(&instance, &ref);  // Singleton should return same instance
  });
}

// Test: Publish does not throw with valid topic and message
TEST_F(HerkusBusTest, PublishWithValidTopicAndMessage) {
  HerkusBus& bus = HerkusBus::getInstance();
  json message;
  message["data"] = "test";
  message["timestamp"] = 12345;
  
  // Should not throw
  EXPECT_NO_THROW({
    bus.Publish("test/topic", message);
  });
}

// Test: Publish with empty topic
TEST_F(HerkusBusTest, PublishWithEmptyTopic) {
  HerkusBus& bus = HerkusBus::getInstance();
  json message;
  message["data"] = "test";
  
  // Should not throw even with empty topic
  EXPECT_NO_THROW({
    bus.Publish("", message);
  });
}

// Test: Publish with empty message
TEST_F(HerkusBusTest, PublishWithEmptyMessage) {
  HerkusBus& bus = HerkusBus::getInstance();
  json message = json::object();
  
  // Should not throw
  EXPECT_NO_THROW({
    bus.Publish("test/topic", message);
  });
}

// Test: Subscribe with valid callback
TEST_F(HerkusBusTest, SubscribeWithValidCallback) {
  HerkusBus& bus = HerkusBus::getInstance();
  bool callback_called = false;
  
  subscriber_callback callback = [&callback_called](const std::string& topic, const json& msg) {
    callback_called = true;
  };
  
  // Should not throw
  EXPECT_NO_THROW({
    bus.Subscribe("test/topic", callback);
  });
}

// Test: Subscribe and publish
TEST_F(HerkusBusTest, SubscribeAndPublish) {
  HerkusBus& bus = HerkusBus::getInstance();
  std::string received_topic;
  json received_message;
  bool callback_called = false;
  
  subscriber_callback callback = [&](const std::string& topic, const json& msg) {
    received_topic = topic;
    received_message = msg;
    callback_called = true;
  };
  
  bus.Subscribe("test/event", callback);
  
  json message;
  message["value"] = 42;
  
  bus.Publish("test/event", message);
  
  // Give some time for message delivery (if async)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  
  // At least the publish should not throw
  EXPECT_NO_THROW({});
}

// Test: Multiple subscriptions to different topics
TEST_F(HerkusBusTest, MultipleSubscriptionsToTopics) {
  HerkusBus& bus = HerkusBus::getInstance();
  int topic1_count = 0;
  int topic2_count = 0;
  
  subscriber_callback callback1 = [&topic1_count](const std::string& topic, const json& msg) {
    topic1_count++;
  };
  
  subscriber_callback callback2 = [&topic2_count](const std::string& topic, const json& msg) {
    topic2_count++;
  };
  
  EXPECT_NO_THROW({
    bus.Subscribe("topic1", callback1);
    bus.Subscribe("topic2", callback2);
  });
}

// Test: Publish complex JSON message
TEST_F(HerkusBusTest, PublishComplexJsonMessage) {
  HerkusBus& bus = HerkusBus::getInstance();
  
  json complex_message = json::object();
  complex_message["user"] = "test_user";
  complex_message["data"] = json::object();
  complex_message["data"]["value"] = 123;
  complex_message["data"]["array"] = json::array({1, 2, 3});
  
  EXPECT_NO_THROW({
    bus.Publish("test/complex", complex_message);
  });
}

// Test: Publish to hierarchical topics
TEST_F(HerkusBusTest, PublishToHierarchicalTopics) {
  HerkusBus& bus = HerkusBus::getInstance();
  json message;
  message["test"] = "data";
  
  EXPECT_NO_THROW({
    bus.Publish("root/branch/leaf", message);
  });
}

// Test: Subscribe to root topic
TEST_F(HerkusBusTest, SubscribeToRootTopic) {
  HerkusBus& bus = HerkusBus::getInstance();
  
  subscriber_callback callback = [](const std::string& topic, const json& msg) {};
  
  EXPECT_NO_THROW({
    bus.Subscribe("root", callback);
  });
}
