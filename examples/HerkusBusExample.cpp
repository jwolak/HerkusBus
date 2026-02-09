#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <thread>

#include "../api/HerkusBus.h"

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