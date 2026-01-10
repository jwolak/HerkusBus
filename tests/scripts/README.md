# HerkusBus Performance Testing

## Overview

This directory contains the performance testing framework for the HerkusBus message queue system. It measures:

- **Message throughput**: How many messages per second can be sent with different payload sizes
- **Timing analysis**: Total time to send batches of messages (100, 500, 1000)
- **System metrics**: CPU usage and memory consumption during tests
- **Visual reports**: Graphs and HTML report summarizing the results

## Running Performance Tests

### Prerequisites

1. Build the project first:
   ```bash
   ./scripts/build.sh release
   ```

2. Install Python dependencies:
   ```bash
   pip install matplotlib pandas numpy
   ```

### Running Tests

From the project root directory:

```bash
bash tests/scripts/perf_test.sh
```

This will:
1. Build the performance test executable
2. Run performance tests with various message counts (100, 500, 1000) and sizes (100B, 1KB, 10KB)
3. Collect system metrics (CPU, memory) during the test
4. Generate performance graphs and an HTML report

### Test Results

All results are saved in the `perf_test/` directory at the project root:

- **perf_results.csv**: Raw timing data in CSV format
- **summary.txt**: Human-readable summary of performance metrics
- **system_metrics.csv**: System CPU and memory usage over time
- **timing_performance.png**: Graphs showing message timing and throughput
- **system_metrics.png**: Graphs showing CPU and memory usage
- **report.html**: Visual HTML report with all metrics and graphs

### Direct Test Execution

You can also run the performance test directly:

```bash
./build/release/bin/HerkusBusPerformanceTests
```

Or generate graphs from existing results:

```bash
python3 tests/scripts/generate_graphs.py perf_test
```

## Performance Test Configuration

The test configuration can be modified in [HerkusBusPerformanceTests.cpp](../HerkusBusPerformanceTests.cpp):

```cpp
struct TestConfig {
    std::vector<int> message_counts = {100, 500, 1000};        // Number of messages
    std::vector<std::string> message_sizes = {"small", "medium", "large"};
    std::vector<int> payload_bytes = {100, 1024, 10240};       // 100B, 1KB, 10KB
    int warmup_messages = 50;                                   // Warmup phase size
};
```

## Sample Results

Example performance metrics (Intel i7, Ubuntu Linux):

| Messages | Small (100B) | Medium (1KB) | Large (10KB) |
|----------|-------------|-------------|-------------|
| 100      | 12.82 ms (7800 msg/sec) | 13.60 ms (7350 msg/sec) | 25.54 ms (3915 msg/sec) |
| 500      | 62.88 ms (7951 msg/sec) | 75.93 ms (6585 msg/sec) | 151.42 ms (3302 msg/sec) |
| 1000     | 122.77 ms (8146 msg/sec) | 134.97 ms (7409 msg/sec) | 262.70 ms (3807 msg/sec) |

## Understanding Results

- **Throughput is higher for smaller messages**: Larger payloads require more time to serialize/deserialize
- **Consistent scalability**: Performance scales roughly linearly with message count
- **System metrics show**: CPU usage spikes during publishing, with memory remaining relatively stable

## Notes

- The `perf_test/` directory is git-ignored (see .gitignore)
- Tests use realistic JSON payloads with nested structures similar to production use
- Performance may vary depending on system load and available resources
- For reproducible results, run tests on an idle system
