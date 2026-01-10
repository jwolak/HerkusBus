# HerkusBus
**Experimental library with implementation of interprocess message queue bus based on interprocess shared memory from Boost**

## Features

- message queue in shared memeory
- message payload in JSON
- publish–subscribe pattern
- powered by Boost
- comprehensive unit tests with Google Test
- automated build system with statistics tracking

## Building from source

### Quick Start with Build Script

The project includes an automated build script that simplifies the build process:

```bash
# Build Debug version (with -O0 -g3 optimization)
./scripts/build.sh debug

# Build Release version (with -O3 optimization)
./scripts/build.sh release

# Build both Debug and Release, then run unit tests
./scripts/build.sh all

# Clean build directory
./scripts/build.sh clean
```

### Build Script Features

- **Automated dependency checking**: Verifies CMake, Make, g++, Boost, and Google Test
- **Standardized directory structure**: Builds to `build/debug/` and `build/release/`
- **Build statistics**: Displays build time, binary size, and warning count
- **Tabular summary**: Compares Debug and Release builds side-by-side
- **Unit test execution**: Automatically runs tests when using `all` option

### Build Output Structure

```
build/
├── debug/
│   ├── lib/libHerkusBus.so      # Debug library
│   └── bin/HerkusBusExample     # Example application
└── release/
    ├── lib/libHerkusBus.so      # Release library
    ├── bin/HerkusBusExample     # Example application
    └── bin/HerkusBusTests       # Unit tests (with 'all' option)
```

### Dependencies

Before building, ensure the following dependencies are installed:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libboost-all-dev libgtest-dev
```

### Manual Build (Alternative)

If you prefer to build manually without the script:

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCPU_ARCH=x64 ..
make
```

## Install
```bash
sudo make install
```
## Example:

Include "HerkusBus.h" to source code:
```sh
See: examples/HerkusBusExample.cpp
```
```

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

## Performance Testing

HerkusBus includes a comprehensive performance testing framework that measures message throughput, latency, and system resource utilization.

### Test Framework Overview

The performance testing suite consists of:

1. **Dedicated C++ Performance Test Executable** (`HerkusBusPerformanceTests`)
   - Measures message publishing throughput with realistic JSON payloads
   - Tests variable message counts and payload sizes
   - Outputs timing data and performance metrics

2. **System Metrics Collection** (CPU, Memory)
   - Monitors system resource usage during tests
   - Parses `/proc/stat` and `/proc/meminfo` for accurate metrics
   - Lightweight monitoring with minimal overhead

3. **Automated Report Generation** (Python)
   - Generates timing performance graphs
   - Creates system resource usage visualizations
   - Produces HTML report with summary statistics

### Running Performance Tests

```bash
# From project root
bash tests/scripts/perf_test.sh

# Or run directly
./build/release/bin/HerkusBusPerformanceTests

# Generate graphs from existing results
python3 tests/scripts/generate_graphs.py perf_test
```

### Test Configuration

The performance tests measure message publishing throughput with the following parameters:

| Parameter | Values |
|-----------|--------|
| **Message Counts** | 100, 500, 1000, 1200 |
| **Payload Sizes** | Small (100B), Medium (1KB), Large (10KB) |
| **Payload Type** | Realistic JSON structures with nested objects |
| **Test Runs** | Multiple test sequences for each configuration |

### Performance Results

Measured on test system with comprehensive JSON payloads containing nested objects and mixed data types:

#### Message Publishing Time (milliseconds)

```
Messages    Small (100B)    Medium (1KB)    Large (10KB)
---------------------------------------------------------
100              112.79          115.39         147.29
500              165.95          171.32         339.96
1000             223.38          233.32         587.71
1200             246.61          259.28         691.81
```

#### Throughput (Messages per Second)

```
Messages    Small (100B)    Medium (1KB)    Large (10KB)
---------------------------------------------------------
100              887            867             679
500             3013           2918            1471
1000            4477           4286            1702
1200            4866           4628            1735
```

#### Performance Visualization

![Message Publishing Performance Graph](perf_test/timing_performance.png)

*The graph above shows two key metrics:*
- **Left panel**: Message publishing time (ms) for different batch sizes grouped by payload size
- **Right panel**: Throughput (messages/sec) - demonstrates how throughput scales with larger batches

### Performance Analysis

**Key Observations:**

1. **Throughput Scalability**: Message throughput increases significantly as batch size increases:
   - 100 messages: ~800-900 msg/sec
   - 500 messages: ~2900-3000 msg/sec
   - 1000+ messages: ~4200-4800 msg/sec

2. **Payload Size Impact**: Larger payloads (10KB) show reduced throughput:
   - Small payloads: Consistent ~4600 msg/sec at 1200 messages
   - Large payloads: Reduced to ~1700 msg/sec due to message serialization and queue processing

3. **Shared Memory Architecture**: The 64KB shared memory buffer limits concurrent message capacity:
   - Maximum sustainable batch size: ~1200 messages
   - Adaptive delays prevent buffer overflow
   - Messages are processed asynchronously by event loop

### Output Files

The performance test framework generates the following results in the `perf_test/` directory:

```
perf_test/
├── perf_results.csv           # Raw timing data in CSV format
├── summary.txt                # Human-readable summary
├── timing_performance.png      # Message timing visualization
├── system_metrics.png         # CPU/Memory usage graphs (if collected)
└── report.html                # Interactive HTML report
```

### Performance Testing Architecture

```
┌─────────────────────────────────────┐
│  perf_test.sh (Orchestrator)       │
│  - Monitors system metrics          │
│  - Runs performance executable      │
│  - Triggers graph generation        │
└────────┬──────────────────────────┬─┘
         │                          │
    ┌────▼─────────┐         ┌─────▼──────────┐
    │ C++ Test Exe │         │ System Monitor │
    │ - 100 msgs   │         │ - CPU stats    │
    │ - 500 msgs   │         │ - Memory info  │
    │ - 1000 msgs  │         │ - Timing data  │
    │ - 1200 msgs  │         └────────────────┘
    └────┬─────────┘
         │
    ┌────▼──────────────────┐
    │ CSV Files & Results   │
    │ - perf_results.csv    │
    │ - system_metrics.csv  │
    │ - summary.txt         │
    └────┬──────────────────┘
         │
    ┌────▼────────────────────────┐
    │ Python Graph Generator      │
    │ - Timing graphs             │
    │ - System metrics graphs     │
    │ - HTML report               │
    └─────────────────────────────┘
```

### Limitations & Notes

- **Shared Memory Size**: 64KB buffer limits concurrent messages
- **Test Duration**: Includes adaptive delays to prevent queue overflow
- **Timing Overhead**: Times include message serialization, queue operations, and callback dispatch
- **JSON Payloads**: Realistic nested structures (not minimal payloads)

### Performance Tips

For optimal performance with HerkusBus:

1. **Batch Processing**: Send messages in batches rather than individually
2. **Message Size**: Use smaller payloads for higher throughput
3. **Queue Management**: Allow time between message bursts for asynchronous processing
4. **Monitoring**: Use the built-in monitoring to track system impact


**BSD 3-Clause License**
<br/>Copylefts 2025, Janusz Wolak
<br/>No rights reserved
