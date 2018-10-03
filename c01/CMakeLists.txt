cmake_minimum_required(VERSION 3.8)
project(c01)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(c01 ${SOURCE_FILES})