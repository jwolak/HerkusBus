#-------------------------------------------------------------------------------
#                                CMAKE settings
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.5)

string(ASCII 27 Esc)
set(Red "${Esc}[31m")
set(Green "${Esc}[32m")
set(Yellow "${Esc}[33m")
set(Blue "${Esc}[34m")
set(Magenta "${Esc}[35m")
set(ColourReset "${Esc}[m")

#-------------------------------------------------------------------------------
#                                Project settings
#-------------------------------------------------------------------------------
set (HERKUS_BUS_VERSION 0.0.1)
project(HerkusBusExample VERSION ${HERKUS_BUS_VERSION})

set (OUTPUT_BINARY_NAME HerkusBusExample-1)
message(STATUS "Binary name: " ${OUTPUT_BINARY_NAME})

message(STATUS "Processing CMakeLists.txt for: " ${PROJECT_NAME} " " ${VERSION})
message(STATUS "CMAKE_SOURCE_DIR:    " ${CMAKE_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPILER_FLAGS_DEBUG "-O0 -g3 -Wall -fmessage-length=0 -Wno-psabi -pthread -lrt")
set(COMPILER_FLAGS_RELEASE "-O0 -Wall -fmessage-length=0 -Wno-psabi -pthread -lrt")

message(STATUS "C++ standard set to: " ${CMAKE_CXX_STANDARD})
message(STATUS "Compilers flags for debug set to: " ${COMPILER_FLAGS_DEBUG})
message(STATUS "Compilers flags for r set to: " ${COMPILER_FLAGS_RELEASE})

# Set the C compiler flags for Debug build.
set(CMAKE_C_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})

# Set the C compiler flags for Release build.
set(CMAKE_C_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

# Set the CXX compiler flags for Debug build.
set(CMAKE_CXX_FLAGS_DEBUG ${COMPILER_FLAGS_DEBUG})

# Set the CXX compiler flags for Release build.
set(CMAKE_CXX_FLAGS_RELEASE ${COMPILER_FLAGS_RELEASE})

find_package(Boost REQUIRED COMPONENTS)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()
include_directories(${Boost_INCLUDE_DIRS})

#-------------------------------------------------------------------------------
#                                Project options
#-------------------------------------------------------------------------------
# Define the destination for the binary object
set (BUILD_DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Define the location/search path for the libraries
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "CPU arch is: " ${CPU_ARCH})
	message(STATUS "Build type is: " ${CMAKE_BUILD_TYPE})
	
	if(CPU_ARCH STREQUAL "x64")
		set (LOGGER_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/logger/x64/Debug/libspdlogd.so)
	endif()
	message(STATUS "Linking to: " ${LOGGER_LIBS_PATH})
	
else()
	
	if(CPU_ARCH STREQUAL "x64")
		set (LOGGER_LIBS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/logger/x64/Release/libspdlog.so)
	endif()
endif()

message(STATUS "Linking Logger library from: " ${LOGGER_LIBS_PATH})

#------------------------------------------------------------------------------------------
#                                Project include
#------------------------------------------------------------------------------------------
set(HERKUS_BUS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../api)
message(STATUS "Set include directory: " ${HERKUS_BUS_INCLUDE})

#-------------------------------------------------------------------------------
#                                Project source directory
#-------------------------------------------------------------------------------
set (HERKUS_BUS_EXAMPLE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
message(STATUS "Set source directory: " ${HERKUS_BUS_EXAMPLE_SRC_DIR})

#-------------------------------------------------------------------------------
#                                Project include
#-------------------------------------------------------------------------------
if(CPU_ARCH STREQUAL "x64")
 	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/logger/include
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mini/include
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/include
		${HERKUS_BUS_INCLUDE}
	)
else()
	include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/logger/include
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/include
		${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/json/include
		${HERKUS_BUS_INCLUDE}
	)
endif()

#-------------------------------------------------------------------------------
#                                Project source
#-------------------------------------------------------------------------------
set (SOURCE_LIST
	${HERKUS_BUS_EXAMPLE_SRC_DIR}/HerkusBusExample.cpp
)

message(STATUS "\n *********** Source files list *********** \n")
foreach(src_file IN LISTS SOURCE_LIST)
    get_filename_component(file_name ${src_file} NAME)
    message(STATUS "  ${Magenta} ${file_name} ${ColourReset}")
endforeach()
message(STATUS "\n *********** End source files list *********** \n")

#-------------------------------------------------------------------------------
#                                Project links
#-------------------------------------------------------------------------------
set(HERKUS_BUS_LIB_DIR ${PROJECT_SOURCE_DIR}/../build/libHerkusBus.so)

link_directories(${LOGGER_LIBS_PATH})
message(STATUS "link_directories: " ${LOGGER_LIBS_PATH})


#-------------------------------------------------------------------------------
#                                Project target
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCE_LIST})
target_link_libraries(${PROJECT_NAME} ${HERKUS_BUS_LIB_DIR} ${LOGGER_LIBS_PATH} pthread rt)