# Copyright (c) 2021 Qizhou Guo
# 
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT

cmake_minimum_required(VERSION 2.8)

project(chtho CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
  -g  
  -Wall  
  -std=c++11
  -pthread 
  -Wno-unused-parameter
  -DCHTHO_COLORED # colored ouput on terminal
)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

include_directories(${PROJECT_SOURCE_DIR}})
include_directories(.)
include_directories(chtho)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(chtho/base)
add_subdirectory(chtho/threads)
add_subdirectory(chtho/logging)
add_subdirectory(chtho/time)
add_subdirectory(chtho/net)

# add_subdirectory(chtho)
