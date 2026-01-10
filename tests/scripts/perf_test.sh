#!/bin/bash

# HerkusBus Performance Testing Script
# This script measures system metrics (CPU, memory) while running performance tests

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build/release"
PERF_TEST_DIR="${PROJECT_ROOT}/perf_test"
PERF_SCRIPTS_DIR="${SCRIPT_DIR}/scripts"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  HerkusBus Performance Testing Suite${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Function to get CPU stats from /proc/stat
get_cpu_stats() {
    awk '/^cpu / {print $2, $3, $4, $5, $6, $7, $8}' /proc/stat
}

# Function to get memory stats from /proc/meminfo
get_memory_stats() {
    awk '/^MemTotal/ {total=$2} /^MemAvailable/ {avail=$2} /^Buffers/ {buffers=$2} /^Cached/ {cached=$2} 
         END {used=total-avail; print total, avail, used, buffers, cached}' /proc/meminfo
}

# Monitor system during test execution
monitor_system() {
    local test_duration=$1
    local monitor_file="${PERF_TEST_DIR}/system_metrics.csv"
    
    # CSV header
    echo "Timestamp(ms),CPU_Usage(%),Memory_Used(MB),Memory_Available(MB)" > "${monitor_file}"
    
    # Initial CPU stats
    local prev_stats=$(get_cpu_stats)
    local prev_user=$(echo $prev_stats | awk '{print $1}')
    local prev_system=$(echo $prev_stats | awk '{print $3}')
    local prev_idle=$(echo $prev_stats | awk '{print $4}')
    local prev_total=$((prev_user + prev_system + prev_idle))
    
    local start_time=$(date +%s%N)
    local counter=0
    
    # Monitor for the specified duration (in milliseconds)
    while true; do
        local current_time=$(date +%s%N)
        local elapsed_ms=$(( (current_time - start_time) / 1000000 ))
        
        # Stop monitoring after test_duration
        if [ $elapsed_ms -gt $test_duration ]; then
            break
        fi
        
        # Get current CPU stats
        local curr_stats=$(get_cpu_stats)
        local curr_user=$(echo $curr_stats | awk '{print $1}')
        local curr_system=$(echo $curr_stats | awk '{print $3}')
        local curr_idle=$(echo $curr_stats | awk '{print $4}')
        local curr_total=$((curr_user + curr_system + curr_idle))
        
        # Calculate CPU usage
        local diff_idle=$((curr_idle - prev_idle))
        local diff_total=$((curr_total - prev_total))
        local cpu_usage=0
        if [ $diff_total -gt 0 ]; then
            cpu_usage=$(( 100 * (diff_total - diff_idle) / diff_total ))
        fi
        
        # Get memory stats
        local mem_stats=$(get_memory_stats)
        local mem_total=$(echo $mem_stats | awk '{print $1}')
        local mem_avail=$(echo $mem_stats | awk '{print $2}')
        local mem_used=$(echo $mem_stats | awk '{print $3}')
        local mem_used_mb=$((mem_used / 1024))
        local mem_avail_mb=$((mem_avail / 1024))
        
        # Record metrics
        echo "${elapsed_ms},${cpu_usage},${mem_used_mb},${mem_avail_mb}" >> "${monitor_file}"
        
        # Update previous stats
        prev_user=$curr_user
        prev_system=$curr_system
        prev_idle=$curr_idle
        prev_total=$curr_total
        
        sleep 0.1
    done
}

# Check if performance test executable exists
if [ ! -f "${BUILD_DIR}/bin/HerkusBusPerformanceTests" ]; then
    echo -e "${RED}Error: HerkusBusPerformanceTests executable not found${NC}"
    echo -e "${YELLOW}Build the project first using: ${PROJECT_ROOT}/scripts/build.sh${NC}"
    exit 1
fi

echo -e "${YELLOW}Starting performance tests...${NC}\n"

# Create perf_test directory if it doesn't exist
mkdir -p "${PERF_TEST_DIR}"

# Run performance test with system monitoring
{
    start_time=$(date +%s%N)
    
    # Start monitoring in background (estimate 60 seconds for the test)
    monitor_system 60000 &
    MONITOR_PID=$!
    
    # Run the test
    "${BUILD_DIR}/bin/HerkusBusPerformanceTests"
    TEST_RESULT=$?
    
    end_time=$(date +%s%N)
    test_duration=$(( (end_time - start_time) / 1000000 ))
    
    # Wait for monitor to finish (max 5 seconds after test ends)
    sleep 1
    kill $MONITOR_PID 2>/dev/null || true
    wait $MONITOR_PID 2>/dev/null || true
    
    exit $TEST_RESULT
}

TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "\n${GREEN}Performance tests completed successfully${NC}"
    
    # Generate graphs if Python script exists
    if [ -f "${PERF_SCRIPTS_DIR}/generate_graphs.py" ]; then
        echo -e "\n${YELLOW}Generating performance graphs...${NC}"
        python3 "${PERF_SCRIPTS_DIR}/generate_graphs.py" "${PERF_TEST_DIR}"
        
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}Graphs generated successfully${NC}"
        else
            echo -e "${RED}Warning: Failed to generate graphs${NC}"
        fi
    fi
    
    echo -e "\n${GREEN}All results saved to: ${PERF_TEST_DIR}/${NC}"
    echo -e "${BLUE}========================================${NC}\n"
else
    echo -e "\n${RED}Performance tests failed!${NC}"
    exit 1
fi
