#-------------------------------------------------------------------------------
#                                Tests CMakeLists.txt
#-------------------------------------------------------------------------------

# Prefer system GTest, otherwise fetch with FetchContent
include(FetchContent)
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    message(STATUS "System GTest not found; fetching googletest...")
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
    set(GTEST_LIBRARIES gtest_main)
else()
    if(TARGET GTest::gtest_main)
        set(GTEST_LIBRARIES GTest::gtest_main)
    elseif(TARGET GTest::gtest)
        set(GTEST_LIBRARIES GTest::gtest)
    else()
        # Some distro packages install static libs without imported CMake targets.
        find_library(GTEST_MAIN_LIBRARY NAMES gtest_main gtest)

        if(GTEST_MAIN_LIBRARY)
            set(GTEST_LIBRARIES ${GTEST_MAIN_LIBRARY})
        else()
            message(FATAL_ERROR "GTest found but no compatible GTest library/target is available")
        endif()
    endif()
endif()

# Set test source files
set(TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/HerkusBusTests.cpp
)

# Create test executable
add_executable(HerkusBusTests ${TEST_SOURCES})

# Set output directory for test executable
set_target_properties(HerkusBusTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Link libraries
target_link_libraries(HerkusBusTests 
    PUBLIC 
        HerkusBus
        ${GTEST_LIBRARIES}
        pthread
)

# Include directories for tests
target_include_directories(HerkusBusTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../api
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/logger/include
)

#-------------------------------------------------------------------------------
#                     Performance Tests CMakeLists.txt
#-------------------------------------------------------------------------------

# Set performance test source files
set(PERF_TEST_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/HerkusBusPerformanceTests.cpp
)

# Create performance test executable
add_executable(HerkusBusPerformanceTests ${PERF_TEST_SOURCES})

# Set output directory for performance test executable
set_target_properties(HerkusBusPerformanceTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Link libraries
target_link_libraries(HerkusBusPerformanceTests 
    PUBLIC 
        HerkusBus
        pthread
)

# Include directories for performance tests
target_include_directories(HerkusBusPerformanceTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../api
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/logger/include
)

message(STATUS "Tests configuration completed")
message(STATUS "Performance tests configuration completed")
