#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "HerkusBus.h"
#include "MockHerkusBus.h"

using namespace Herkus;
using ::testing::_;
using ::testing::Invoke;
using ::testing::SaveArg;

TEST(HerkusBusMockTest, PublishAndSubscribeWithGMock) {
  auto mock = std::make_unique<MockHerkusBus>();
  MockHerkusBus* mock_ptr = mock.get();
  HerkusBus::SetGlobalBus(std::move(mock));

  subscriber_callback saved_cb;

  EXPECT_CALL(*mock_ptr, Subscribe("topic1", _)).WillOnce(SaveArg<1>(&saved_cb));

  EXPECT_CALL(*mock_ptr, Publish("topic1", _)).WillOnce(Invoke([&](const std::string& topic, const json& msg) {
    if (saved_cb) saved_cb(topic, msg);
  }));

  IHerkusBus& bus = HerkusBus::GetBusInterface();

  bool called = false;
  bus.Subscribe("topic1", [&called](const std::string& topic, const json& msg) { called = true; });
  bus.Publish("topic1", json::object({{"k", "v"}}));

  EXPECT_TRUE(called);
}
