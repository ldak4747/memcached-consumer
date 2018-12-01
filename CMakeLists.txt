cmake_minimum_required(VERSION 2.8)

project(firehose-consume-cpp-frame)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILECMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O -Wall -g -ggdb")
set(_GLIBCXX_USE_NANOSLEEP ON)
set(CMAKE_BUILD_TYPE "Debug")

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(GTEST_DIR "${BASE_DIR}/gtest/gtest-1.7.0")
set(LOG4CPLUS_DIR "${BASE_DIR}/log4cplus")
set(PROTOBUF_DIR "${BASE_DIR}/protobuf")

set(TEST_DIR "${PROJECT_DIR}/test")
set(SRC_DIR "${PROJECT_DIR}/src")
set(PROTO_DIR "${PROJECT_DIR}/proto")


add_subdirectory(${TEST_DIR})
add_subdirectory(${SRC_DIR})

