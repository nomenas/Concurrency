cmake_minimum_required(VERSION 3.7)
project(datasafety)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp safe_interface.cpp safe_local_members_asynchronous.cpp safe_local_members.cpp)
add_executable(datasafety ${SOURCE_FILES})