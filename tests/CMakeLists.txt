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

message(STATUS "Tests configuration completed")
