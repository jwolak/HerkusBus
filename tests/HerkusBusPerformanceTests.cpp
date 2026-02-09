#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../api/HerkusBus.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace Herkus;

// Get the project root directory by finding the perf_test folder
std::string getProjectRoot() {
  std::filesystem::path current = std::filesystem::current_path();

  // Try going up directories to find HerkusBus project root
  while (current.has_parent_path()) {
    std::filesystem::path perf_test_path = current / "perf_test";
    if (std::filesystem::exists(perf_test_path) && std::filesystem::is_directory(perf_test_path)) {
      return current.string();
    }
    current = current.parent_path();
  }

  // If not found, assume we're in the project root
  return std::filesystem::current_path().string();
}

// Performance test configuration
struct TestConfig {
  std::vector<int> message_counts = {100, 500, 1000, 1200};
  std::vector<std::string> message_sizes = {"small", "medium", "large"};
  std::vector<int> payload_bytes = {100, 1024, 10240};  // 100B, 1KB, 10KB
  int warmup_messages = 50;
  std::chrono::milliseconds wait_timeout = std::chrono::milliseconds(10000);
  std::string output_dir;

  TestConfig() { output_dir = getProjectRoot() + "/perf_test"; }
};

// Generate realistic JSON payload of specified size
json generateRealisticPayload(int target_bytes) {
  json payload;

  // Create a base structure with realistic data
  payload["topic"] = "performance/test/message";
  payload["timestamp"] = 1234567890;
  payload["source"] = "HerkusBusPerformanceTest";
  payload["message_id"] = 12345;

  // Create nested object with various data types
  json metadata;
  metadata["device_id"] = "device_001";
  metadata["location"]["latitude"] = 52.2297;
  metadata["location"]["longitude"] = 21.0122;
  metadata["location"]["city"] = "Warsaw";
  metadata["location"]["country"] = "Poland";
  metadata["sensors"]["temperature"] = 23.5;
  metadata["sensors"]["humidity"] = 65.2;
  metadata["sensors"]["pressure"] = 1013.25;

  payload["metadata"] = metadata;

  // Add filler data to reach target size
  std::string base_str = payload.dump();
  if (static_cast<int>(base_str.length()) < target_bytes) {
    int remaining = target_bytes - static_cast<int>(base_str.length());
    std::string filler(remaining, 'x');
    payload["filler"] = filler;
  }

  return payload;
}

struct ReceiveState {
  std::atomic<int> received{0};
  std::atomic<int> target{0};
  std::atomic<bool> promise_set{false};
  std::mutex promise_mutex;
  std::shared_ptr<std::promise<void>> promise;
};

// Measure time to publish messages
double publishMessages(int message_count, int payload_bytes, const std::string& size_name, const std::shared_ptr<ReceiveState>& state,
                       std::chrono::milliseconds wait_timeout) {
  auto& bus = HerkusBus::getInstance();
  std::string topic = "perf/test/" + size_name;
  json payload = generateRealisticPayload(payload_bytes);

  state->received.store(0);
  state->target.store(message_count);
  state->promise_set.store(false);
  auto run_promise = std::make_shared<std::promise<void>>();
  {
    std::lock_guard<std::mutex> lock(state->promise_mutex);
    state->promise = run_promise;
  }
  auto future = run_promise->get_future();

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < message_count; ++i) {
    payload["sequence"] = i;
    bus.Publish(topic, payload);

    // Adaptive delay to prevent shared memory buffer overflow
    if (payload_bytes >= 10240 && i % 30 == 29) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else if (i % 100 == 99) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  if (future.wait_for(wait_timeout) != std::future_status::ready) {
    std::cerr << "Timeout waiting for messages on topic: " << topic << std::endl;
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;

  return duration.count();
}

// Save results to CSV
void saveResults(const std::vector<std::vector<double>>& results, const TestConfig& config, const std::string& filename) {
  std::ofstream file(config.output_dir + "/" + filename);

  // CSV header
  file << "MessageCount";
  for (const auto& size_name : config.message_sizes) {
    file << "," << size_name;
  }
  file << "\n";

  // CSV data rows
  for (size_t i = 0; i < config.message_counts.size(); ++i) {
    file << config.message_counts[i];
    for (size_t j = 0; j < config.message_sizes.size(); ++j) {
      file << "," << std::fixed << std::setprecision(2) << results[i][j];
    }
    file << "\n";
  }

  file.close();
}

// Print summary to console and file
void printSummary(const std::vector<std::vector<double>>& results, const TestConfig& config) {
  std::ofstream summary_file(config.output_dir + "/summary.txt");

  auto print = [&](std::ostream& os) {
    os << "===============================================\n";
    os << "      HerkusBus Performance Test Results\n";
    os << "===============================================\n\n";

    os << "Test Configuration:\n";
    os << "  Message Counts: ";
    for (auto c : config.message_counts) os << c << " ";
    os << "\n";
    os << "  Message Sizes: ";
    for (auto s : config.message_sizes) os << s << " ";
    os << "\n";
    os << "  Payload Bytes: ";
    for (auto b : config.payload_bytes) os << b << "B ";
    os << "\n\n";

    os << "Results (Time in milliseconds):\n";
    os << std::left << std::setw(15) << "Messages" << std::setw(15) << "Small (100B)" << std::setw(15) << "Medium (1KB)" << std::setw(15) << "Large (10KB)"
       << "\n";
    os << std::string(60, '-') << "\n";

    for (size_t i = 0; i < config.message_counts.size(); ++i) {
      os << std::left << std::setw(15) << config.message_counts[i];
      for (size_t j = 0; j < config.message_sizes.size(); ++j) {
        os << std::setw(15) << std::fixed << std::setprecision(2) << results[i][j];
      }
      os << "\n";
    }

    os << std::string(60, '-') << "\n\n";

    os << "Performance Summary:\n";
    for (size_t i = 0; i < config.message_counts.size(); ++i) {
      for (size_t j = 0; j < config.message_sizes.size(); ++j) {
        double messages_per_sec = (config.message_counts[i] / results[i][j]) * 1000;
        os << "  " << config.message_counts[i] << " messages (" << config.message_sizes[j] << "): " << std::fixed << std::setprecision(2) << results[i][j]
           << " ms (" << std::fixed << std::setprecision(0) << messages_per_sec << " msg/sec)\n";
      }
    }

    os << "\n===============================================\n";
  };

  print(std::cout);
  print(summary_file);

  summary_file.close();
}

int main() {
  TestConfig config;

  auto& bus = HerkusBus::getInstance();

  std::cout << "HerkusBus Performance Tests Starting...\n";
  std::cout << "Output directory: " << config.output_dir << "\n\n";

  try {
    std::unordered_map<std::string, std::shared_ptr<ReceiveState>> receive_states;
    receive_states.reserve(config.message_sizes.size() + 1);

    receive_states.emplace("warmup", std::make_shared<ReceiveState>());
    for (const auto& size_name : config.message_sizes) {
      receive_states.emplace(size_name, std::make_shared<ReceiveState>());
    }

    for (const auto& [size_name, state] : receive_states) {
      std::string topic = "perf/test/" + size_name;
      bus.Subscribe(topic, [state](const std::string& topic_name, const Herkus::json& msg) {
        (void)topic_name;
        (void)msg;
        const int current = state->received.fetch_add(1) + 1;
        if (current >= state->target.load()) {
          if (!state->promise_set.exchange(true)) {
            std::shared_ptr<std::promise<void>> promise;
            {
              std::lock_guard<std::mutex> lock(state->promise_mutex);
              promise = state->promise;
            }
            if (promise) {
              promise->set_value();
            }
          }
        }
      });
    }

    // Warmup phase
    std::cout << "Warming up HerkusBus...\n";
    publishMessages(config.warmup_messages, 100, "warmup", receive_states.at("warmup"), config.wait_timeout);

    // Main test phase
    std::cout << "Running performance tests...\n";
    std::vector<std::vector<double>> results(config.message_counts.size(), std::vector<double>(config.message_sizes.size()));

    for (size_t i = 0; i < config.message_counts.size(); ++i) {
      for (size_t j = 0; j < config.message_sizes.size(); ++j) {
        std::cout << "  Testing " << config.message_counts[i] << " messages (" << config.message_sizes[j] << ")...\n";

        results[i][j] = publishMessages(config.message_counts[i], config.payload_bytes[j], config.message_sizes[j], receive_states.at(config.message_sizes[j]),
                                        config.wait_timeout);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // Save results
    saveResults(results, config, "perf_results.csv");
    printSummary(results, config);

    std::cout << "\nPerformance tests completed successfully!\n";
    std::cout << "Results saved to: " << config.output_dir << "/\n";

    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error during performance tests: " << e.what() << "\n";
    return 1;
  }
}
