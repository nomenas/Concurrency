cmake_minimum_required(VERSION 3.7)
project(TaskConcept)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES test_synchronous_call.cpp)
add_executable(TaskConcept ${SOURCE_FILES})