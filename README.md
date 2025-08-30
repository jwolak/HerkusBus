# HerkusBus
Message data bus

# HerkusBus
**Experimental interprocess message bus library based on shared memory **

## Features

- message payload based on JSON
- shred memory
- powered by Boost

## Building for source
```sh
mkdir build
cd build
cmake CMakeLists.txt
make
```
## Install
```sh
$ sudo make install
or
# make install
```
## Example:

Include "HerkusBus.h" to source code:
```sh
See: examples/HerkusBusExample.cpp
```
```sh

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
                [](const std::string& topic, const Herkus::json& msg) { std::cout << "[Example] Received on [" << topic << "]: " << msg.dump() << std::endl; });
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::thread t1(PublishLoop, kTopicName, kThreadId_1);
  std::thread t2(PublishLoop, kTopicName, kThreadId_2);

  t1.join();
  t2.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  return 0;
}
```
```sh
Output:
 
[2025-08-30 18:35:46.738] [debug] Shared memory segment removed [HerkusBusImpl.cpp:69]
[2025-08-30 18:35:46.739] [debug] Create bus event loop thread... [HerkusBusImpl.cpp:76]
[2025-08-30 18:35:46.739] [debug] Event loop checks if message queue is empty... [HerkusBusImpl.cpp:80]
[2025-08-30 18:35:46.739] [debug] No message in queue [HerkusBusImpl.cpp:82]
[2025-08-30 18:35:46.739] [debug] Waiting... [HerkusBusImpl.cpp:83]
Subscribe to: example_topic
[2025-08-30 18:35:46.739] [debug] Subscribe on topic: example_topic [HerkusBusImpl.cpp:145]
[2025-08-30 18:35:46.739] [debug] Add new subscriber callback to callbacks list [example_topic:HerkusBusImpl.cpp]
[2025-08-30 18:35:46.840] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:46.840] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:46.840] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:46.840] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:46.840] [debug] Message queue is not empty... [HerkusBusImpl.cpp:90]
[2025-08-30 18:35:46.840] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:46.840] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:46.840] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:46.840] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:46.840] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:46.840] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:46.840] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":0,"ThreadId":1}
[2025-08-30 18:35:46.841] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:46.841] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:46.841] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:46.841] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:46.841] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":0,"ThreadId":2}
[2025-08-30 18:35:46.841] [debug] Event loop checks if message queue is empty... [HerkusBusImpl.cpp:80]
[2025-08-30 18:35:46.841] [debug] No message in queue [HerkusBusImpl.cpp:82]
[2025-08-30 18:35:46.841] [debug] Waiting... [HerkusBusImpl.cpp:83]
[2025-08-30 18:35:46.940] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:46.940] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:46.940] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:46.940] [debug] Message queue is not empty... [HerkusBusImpl.cpp:90]
[2025-08-30 18:35:46.940] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:46.940] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:46.940] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:46.940] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:46.940] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:46.940] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:46.940] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:46.940] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":1,"ThreadId":1}
[2025-08-30 18:35:46.940] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:46.940] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:46.940] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:46.940] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:46.941] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":1,"ThreadId":2}
[2025-08-30 18:35:46.941] [debug] Event loop checks if message queue is empty... [HerkusBusImpl.cpp:80]
[2025-08-30 18:35:46.941] [debug] No message in queue [HerkusBusImpl.cpp:82]
[2025-08-30 18:35:46.941] [debug] Waiting... [HerkusBusImpl.cpp:83]
[2025-08-30 18:35:47.040] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:47.041] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:47.041] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:47.040] [debug] Publish message on topic: example_topic [HerkusBusImpl.cpp:135]
[2025-08-30 18:35:47.041] [debug] Message queue is not empty... [HerkusBusImpl.cpp:90]
[2025-08-30 18:35:47.041] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:47.041] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:47.041] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:47.041] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:47.041] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":2,"ThreadId":1}
[2025-08-30 18:35:47.041] [debug] Event loop checks if message queue is empty... [HerkusBusImpl.cpp:80]
[2025-08-30 18:35:47.041] [debug] No message in queue [HerkusBusImpl.cpp:82]
[2025-08-30 18:35:47.041] [debug] Waiting... [HerkusBusImpl.cpp:83]
[2025-08-30 18:35:47.041] [debug] Add message to message queue [HerkusBusImpl.cpp:138]
[2025-08-30 18:35:47.041] [debug] Notify bus event loop about new published message... [HerkusBusImpl.cpp:140]
[2025-08-30 18:35:47.041] [debug] Message queue is not empty... [HerkusBusImpl.cpp:90]
[2025-08-30 18:35:47.041] [debug] New message in queue [HerkusBusImpl.cpp:92]
[2025-08-30 18:35:47.041] [debug] Get message from queue [HerkusBusImpl.cpp:94]
[2025-08-30 18:35:47.041] [debug] Message removed from queue as already taken [HerkusBusImpl.cpp:96]
[2025-08-30 18:35:47.041] [debug] Parsing message... [HerkusBusImpl.cpp:99]
[2025-08-30 18:35:47.041] [debug] Call callbacks for all subscribers subscribed on topic: [example_topic] [HerkusBusImpl.cpp:102]
[Example] Received on [example_topic]: {"MessageId":2,"ThreadId":2}
[2025-08-30 18:35:47.041] [debug] Event loop checks if message queue is empty... [HerkusBusImpl.cpp:80]
[2025-08-30 18:35:47.041] [debug] No message in queue [HerkusBusImpl.cpp:82]
[2025-08-30 18:35:47.041] [debug] Waiting... [HerkusBusImpl.cpp:83]
[2025-08-30 18:35:47.642] [debug] Notify bus event loop to be stopped [HerkusBusImpl.cpp:122]
[2025-08-30 18:35:47.642] [debug] Event loop stopped [HerkusBusImpl.cpp:86]
[2025-08-30 18:35:47.643] [debug] Shared memory segment removed [HerkusBusImpl.cpp:131]

```
## License

**BSD 3-Clause License**
<br/>Copyright (c) 2025, Janusz Wolak
<br/>All rights reserved.
