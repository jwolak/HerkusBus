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
#                                HerkusBus settings
#-------------------------------------------------------------------------------
set (HERKUS_BUS_VERSION 0.0.1)
project(HerkusBus VERSION ${HERKUS_BUS_VERSION})

set (OUTPUT_BINARY_NAME HerkusBus)
message(STATUS "Version: " ${HERKUS_BUS_VERSION})
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
#                                HerkusBus options
#-------------------------------------------------------------------------------
option(HERKUS_BUS_TESTS         "Build tests"           OFF)
option(HERKUS_BUS_EXAMPLES      "Build examples"        ON)
option(HERKUS_BUS_BUILD_SHARED  "Build shared lib"      ON)
option(HERKUS_BUS_BUILD_STATIC  "Build static lib"      OFF)

if(HERKUS_BUS_TESTS)
    message(STATUS "Build test enabled")
endif()

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
#                                HerkusBus api
#------------------------------------------------------------------------------------------
set(HERKUS_BUS_API ${CMAKE_CURRENT_SOURCE_DIR}/api)
message(STATUS "Set api directory: " ${HERKUS_BUS_API})

#------------------------------------------------------------------------------------------
#                                HerkusBus include
#------------------------------------------------------------------------------------------
set(HERKUS_BUS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
message(STATUS "Set include directory: " ${HERKUS_BUS_INCLUDE})

#-------------------------------------------------------------------------------
#                                HerkusBus source directory
#-------------------------------------------------------------------------------
set (HERKUS_BUS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
message(STATUS "Set source directory: " ${HERKUS_BUS_SRC_DIR})

#-------------------------------------------------------------------------------
#                                HerkusBus include
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
#                                HerkusBus source
#-------------------------------------------------------------------------------
set (SOURCE_LIST
	${HERKUS_BUS_SRC_DIR}/HerkusBus.cpp
	${HERKUS_BUS_SRC_DIR}/HerkusBusImpl.cpp
)
message(STATUS "\n *********** Source files list *********** \n")
foreach(src_file IN LISTS SOURCE_LIST)
    get_filename_component(file_name ${src_file} NAME)
    message(STATUS "  ${Magenta} ${file_name} ${ColourReset}")
endforeach()
message(STATUS "\n *********** End source files list *********** \n")

#-------------------------------------------------------------------------------
#                                HerkusBus links
#-------------------------------------------------------------------------------
link_directories(${LOGGER_LIBS_PATH})
message(STATUS "link_directories: " ${LOGGER_LIBS_PATH})


#-------------------------------------------------------------------------------
#                                HerkusBus target
#-------------------------------------------------------------------------------

if (HERKUS_BUS_BUILD_SHARED)
	include_directories( ${OUTPUT_BINARY_NAME} PRIVATE  ${HERKUS_BUS_INCLUDE} ${HERKUS_BUS_API})
	add_library( ${OUTPUT_BINARY_NAME} SHARED ${SOURCE_LIST})
	target_compile_definitions( ${OUTPUT_BINARY_NAME} PUBLIC HERKUS_BUS_SHARED_LIB)
endif()

if (HERKUS_BUS_BUILD_STATIC)
	include_directories( ${OUTPUT_BINARY_NAME} PRIVATE  ${HERKUS_BUS_INCLUDE} ${HERKUS_BUS_API})
	add_library( ${OUTPUT_BINARY_NAME} STATIC ${SOURCE_LIST})
endif()

#-------------------------------------------------------------------------------

if(CPU_ARCH STREQUAL "x64")
	target_link_libraries(${OUTPUT_BINARY_NAME} PUBLIC ${LOGGER_LIBS_PATH})
endif()

#-------------------------------------------------------------------------------
#                                HerkusBus target directories
#-------------------------------------------------------------------------------
install(DIRECTORY DESTINATION ${BUILD_DESTINATION}/bin)

if(CPU_ARCH STREQUAL "x64")
	install (TARGETS ${OUTPUT_BINARY_NAME}
		RUNTIME DESTINATION ${BUILD_DESTINATION}/bin
	)
endif()

#-------------------------------------------------------------------------------
#                                HerkusBus tests
#-------------------------------------------------------------------------------
if(HERKUS_BUS_TESTS)
    add_subdirectory(tests)
    message(STATUS "Tests subdirectory added")
endif(HERKUS_BUS_TESTS)

#------------------------------------------------------------------------------------------
#                                Project examples
#------------------------------------------------------------------------------------------
if(HERKUS_BUS_EXAMPLES)
	add_subdirectory(examples)
endif(HERKUS_BUS_EXAMPLES)

#------------------------------------------------------------------------------------------
#                                Project install
#------------------------------------------------------------------------------------------
install(FILES ${HERKUS_BUS_API}/HerkusBus.h
        DESTINATION include)

install(TARGETS ${OUTPUT_BINARY_NAME} DESTINATION lib)
install(FILES HerkusBusConfig.cmake DESTINATION lib/cmake/HerkusBus)