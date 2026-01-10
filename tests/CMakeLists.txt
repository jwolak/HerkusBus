#-------------------------------------------------------------------------------
#                                Tests CMakeLists.txt
#-------------------------------------------------------------------------------

# Find GTest package
find_package(GTest REQUIRED)

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
        GTest::GTest
        GTest::Main
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
