cmake_minimum_required(VERSION 3.9)
project(testovaci2)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(testovaci2 main.cpp)