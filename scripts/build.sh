#!/bin/bash

################################################################################
#                           HerkusBus Build Script
################################################################################
#
# Usage:
#   ./build.sh debug       - Build in Debug mode
#   ./build.sh release     - Build in Release mode
#   ./build.sh all         - Build Debug + Release + Run tests
#   ./build.sh clean       - Clean build directory
#
# This script:
#   - Creates build/debug and build/release directories
#   - Configures CMake with appropriate flags
#   - Compiles and links the HerkusBus library
#   - Collects build statistics (time, size, warnings)
#   - Displays results in a table format
#
################################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
SCRIPTS_DIR="${PROJECT_ROOT}/scripts"

# Architecture (fixed to x64)
CPU_ARCH="x64"

# Compiler flags
CFLAGS_DEBUG="-O0 -g3 -Wall -fmessage-length=0 -Wno-psabi -pthread -lrt"
CFLAGS_RELEASE="-O3 -Wall -fmessage-length=0 -Wno-psabi -pthread -lrt"

# Statistics array
declare -A BUILD_STATS

################################################################################
#                            Helper Functions
################################################################################

# Print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Check if dependencies are installed
check_dependencies() {
    print_info "Checking dependencies..."
    
    local missing_deps=0
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_warning "CMake is not installed"
        missing_deps=1
    else
        print_success "CMake found: $(cmake --version | head -n1)"
    fi
    
    # Check for Make
    if ! command -v make &> /dev/null; then
        print_warning "Make is not installed"
        missing_deps=1
    else
        print_success "Make found"
    fi
    
    # Check for g++
    if ! command -v g++ &> /dev/null; then
        print_warning "g++ is not installed"
        missing_deps=1
    else
        print_success "g++ found: $(g++ --version | head -n1)"
    fi
    
    # Check for Boost using dpkg (more reliable)
    if ! dpkg -l | grep -q "libboost-all-dev"; then
        print_warning "Boost is not installed"
        missing_deps=1
    else
        print_success "Boost found"
    fi
    
    # Check for GTest using dpkg
    if ! dpkg -l | grep -q "libgtest-dev"; then
        print_warning "GTest is not installed"
        missing_deps=1
    else
        print_success "GTest found"
    fi
    
    if [ $missing_deps -eq 1 ]; then
        print_error "Missing dependencies detected!"
        print_info "To install missing dependencies, run:"
        print_info "  sudo apt-get update"
        print_info "  sudo apt-get install -y build-essential cmake libboost-all-dev libgtest-dev"
        exit 1
    fi
}


# Build function for a specific configuration
build_config() {
    local config=$1  # "debug" or "release"
    local build_path="${BUILD_DIR}/${config}"
    
    print_info "========================================="
    print_info "Building ${config^^} configuration..."
    print_info "========================================="
    
    # Create build directory
    mkdir -p "${build_path}"
    cd "${build_path}"
    
    # Select compiler flags based on configuration
    if [ "$config" = "debug" ]; then
        local cmake_build_type="Debug"
        local cmake_flags="${CFLAGS_DEBUG}"
    else
        local cmake_build_type="Release"
        local cmake_flags="${CFLAGS_RELEASE}"
    fi
    
    # Configure CMake
    print_info "Configuring CMake for ${config^^}..."
    cmake \
        -DCMAKE_BUILD_TYPE="${cmake_build_type}" \
        -DCPU_ARCH="${CPU_ARCH}" \
        -DHERKUS_BUS_EXAMPLES=ON \
        -DHERKUS_BUS_BUILD_SHARED=ON \
        -DCMAKE_CXX_FLAGS_DEBUG="${CFLAGS_DEBUG}" \
        -DCMAKE_CXX_FLAGS_RELEASE="${CFLAGS_RELEASE}" \
        "${PROJECT_ROOT}" > /dev/null 2>&1
    
    if [ $? -ne 0 ]; then
        print_error "CMake configuration failed for ${config}"
        return 1
    fi
    
    # Clean previous build artifacts
    make clean > /dev/null 2>&1 || true
    
    # Build and measure time
    print_info "Compiling ${config^^}..."
    local start_time=$(date +%s%3N)
    
    # Capture build output to analyze warnings
    local build_log="${build_path}/build.log"
    make 2>&1 | tee "${build_log}"
    
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Build failed for ${config}"
        return 1
    fi
    
    local end_time=$(date +%s%3N)
    local build_time=$(echo "scale=2; ($end_time - $start_time) / 1000" | bc)
    
    # Get library size
    local lib_path="${build_path}/lib/libHerkusBus.so"
    if [ ! -f "${lib_path}" ]; then
        # Library might be in a different location
        lib_path=$(find "${build_path}" -name "libHerkusBus.so" 2>/dev/null | head -n1)
    fi
    
    local lib_size="N/A"
    if [ -f "${lib_path}" ]; then
        lib_size=$(du -h "${lib_path}" | cut -f1)
    fi
    
    # Count warnings
    local warning_count=0
    if [ -f "${build_log}" ]; then
        warning_count=$(grep -c "warning:" "${build_log}" || echo "0")
    fi
    
    # Store statistics
    BUILD_STATS["${config}_time"]="${build_time}s"
    BUILD_STATS["${config}_size"]="${lib_size}"
    BUILD_STATS["${config}_warnings"]="${warning_count}"
    
    print_success "${config^^} build completed successfully"
    print_info "Build time: ${build_time}s"
    print_info "Library size: ${lib_size}"
    print_info "Warnings: ${warning_count}"
    
    return 0
}

# Display build summary in table format
display_summary() {
    echo ""
    print_info "========================================="
    print_info "Build Summary"
    print_info "========================================="
    echo ""
    
    # Table header
    printf "%-15s | %-12s | %-12s | %-10s\n" "Build Type" "Time" "Size" "Warnings"
    printf "%-15s + %-12s + %-12s + %-10s\n" "===============" "============" "============" "=========="
    
    # Debug row
    if [ -n "${BUILD_STATS[debug_time]}" ]; then
        printf "%-15s | %-12s | %-12s | %-10s\n" "Debug" \
            "${BUILD_STATS[debug_time]}" \
            "${BUILD_STATS[debug_size]}" \
            "${BUILD_STATS[debug_warnings]}"
    fi
    
    # Release row
    if [ -n "${BUILD_STATS[release_time]}" ]; then
        printf "%-15s | %-12s | %-12s | %-10s\n" "Release" \
            "${BUILD_STATS[release_time]}" \
            "${BUILD_STATS[release_size]}" \
            "${BUILD_STATS[release_warnings]}"
    fi
    
    echo ""
}

# Build with tests enabled
build_with_tests() {
    local build_path="${BUILD_DIR}/release"
    
    print_info "========================================="
    print_info "Building Release with Tests..."
    print_info "========================================="
    
    mkdir -p "${build_path}"
    cd "${build_path}"
    
    # Configure CMake with tests enabled
    print_info "Configuring CMake with tests..."
    cmake \
        -DCMAKE_BUILD_TYPE="Release" \
        -DCPU_ARCH="${CPU_ARCH}" \
        -DHERKUS_BUS_EXAMPLES=ON \
        -DHERKUS_BUS_BUILD_SHARED=ON \
        -DHERKUS_BUS_TESTS=ON \
        -DCMAKE_CXX_FLAGS_DEBUG="${CFLAGS_DEBUG}" \
        -DCMAKE_CXX_FLAGS_RELEASE="${CFLAGS_RELEASE}" \
        "${PROJECT_ROOT}" > /dev/null 2>&1
    
    if [ $? -ne 0 ]; then
        print_error "CMake configuration failed with tests"
        return 1
    fi
    
    # Build
    print_info "Building with tests..."
    make 2>&1
    
    if [ $? -ne 0 ]; then
        print_error "Build with tests failed"
        return 1
    fi
    
    # Run tests
    print_info "========================================="
    print_info "Running Unit Tests..."
    print_info "========================================="
    
    if ! ctest --output-on-failure; then
        print_error "Some tests failed"
        return 1
    fi
    
    print_success "All tests passed"
    return 0
}

# Clean build directory
clean_build() {
    print_info "Cleaning build directory..."
    
    if [ -d "${BUILD_DIR}" ]; then
        rm -rf "${BUILD_DIR}"
        print_success "Build directory cleaned"
    else
        print_info "Build directory does not exist"
    fi
}

# Main script logic
main() {
    local command=${1:-help}
    
    case "${command}" in
        debug)
            check_dependencies
            build_config "debug"
            display_summary
            ;;
        release)
            check_dependencies
            build_config "release"
            display_summary
            ;;
        all)
            check_dependencies
            build_config "debug"
            build_config "release"
            
            # Build and run tests
            if ! build_with_tests; then
                print_warning "Tests encountered issues, but continuing..."
            fi
            
            display_summary
            ;;
        clean)
            clean_build
            ;;
        *)
            echo "HerkusBus Build Script"
            echo ""
            echo "Usage: $0 {debug|release|all|clean}"
            echo ""
            echo "Commands:"
            echo "  debug       Build in Debug mode with -O0 optimization"
            echo "  release     Build in Release mode with -O3 optimization"
            echo "  all         Build Debug + Release + Run unit tests"
            echo "  clean       Remove build directory"
            echo ""
            echo "Examples:"
            echo "  $0 debug"
            echo "  $0 release"
            echo "  $0 all"
            echo "  $0 clean"
            echo ""
            ;;
    esac
}

# Run main function
main "$@"
