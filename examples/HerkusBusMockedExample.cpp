#include <gmock/gmock.h>

#include <iostream>
#include <memory>

#include "../api/HerkusBus.h"
#include "spdlog/spdlog.h"

namespace HerkusBuxMockedExample {

class HerkusBusMock : public Herkus::IHerkusBus {
 public:
  MOCK_METHOD(void, Publish, (const std::string& topic, const Herkus::json& message_payload), (override));
  MOCK_METHOD(void, Subscribe, (const std::string& topic, Herkus::subscriber_callback sub_callback), (override));
};

class ClassWithMockedHerkusBus {
 public:
  ClassWithMockedHerkusBus(std::unique_ptr<Herkus::IHerkusBus> herkus_bus) : herkus_bus_(std::move(herkus_bus)) {}

  void MethodThatCallsHerkusBusPublish(const std::string& topic, const Herkus::json& message_payload) {
    spdlog::info("Publishing message to topic: {}", topic);
    herkus_bus_->Publish(topic, message_payload);
  }

  void MethodThatCallsHerkusBusSubscribe(const std::string& topic, Herkus::subscriber_callback sub_callback) {
    spdlog::info("Subscribing to topic: {}", topic);
    herkus_bus_->Subscribe(topic, std::move(sub_callback));
  }

 private:
  std::unique_ptr<Herkus::IHerkusBus> herkus_bus_;
};

}  // namespace HerkusBuxMockedExample

using namespace testing;

int main() {
  StrictMock<HerkusBuxMockedExample::HerkusBusMock>* mock_bus_mock = new StrictMock<HerkusBuxMockedExample::HerkusBusMock>();
  HerkusBuxMockedExample::ClassWithMockedHerkusBus class_with_mocked_bus =
      HerkusBuxMockedExample::ClassWithMockedHerkusBus(std::unique_ptr<Herkus::IHerkusBus>(mock_bus_mock));

  EXPECT_CALL(*mock_bus_mock, Publish("topic1", Herkus::json::object({{"key", "value"}})));
  EXPECT_CALL(*mock_bus_mock, Subscribe("topic2", _));

  class_with_mocked_bus.MethodThatCallsHerkusBusPublish("topic1", Herkus::json::object({{"key", "value"}}));
  class_with_mocked_bus.MethodThatCallsHerkusBusSubscribe("topic2", [](const std::string& topic, const Herkus::json& msg) {
    std::cout << "Received message on topic: " << topic << " with payload: " << msg.dump() << std::endl;
  });

  return 0;
}