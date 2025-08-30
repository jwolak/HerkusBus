#include <chrono>
#include <iostream>
#include <thread>

#include "../api/HerkusBus.h"

void PublishLoop(const std::string& topic, int threadId) {
  auto& bus = Herkus::HerkusBus::getInstance();
  for (int i = 0; i < 10; ++i) {
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

  auto& bus = Herkus::HerkusBus::getInstance();

  std::cout << "Subscribe to: " << kTopicName << std::endl;
  bus.Subscribe(kTopicName,
                [](const std::string& topic, const Herkus::json& msg) { std::cout << "Odebrano na [" << topic << "]: " << msg.dump() << std::endl; });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::thread t1(PublishLoop, kTopicName, kThreadId_1);
  std::thread t2(PublishLoop, kTopicName, kThreadId_2);

  t1.join();
  t2.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  return 0;
}