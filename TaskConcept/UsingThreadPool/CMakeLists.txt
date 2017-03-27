cmake_minimum_required(VERSION 3.7)
project(UsingThreadPool)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ThreadPool.cpp TaskExecutor.cpp)
add_executable(UsingThreadPool ${SOURCE_FILES})