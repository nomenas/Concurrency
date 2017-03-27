cmake_minimum_required(VERSION 3.7)
project(TaskConcept)

set(CMAKE_CXX_STANDARD 11)

add_executable(SynchronousCallTest test_synchronous_call.cpp)

add_executable(ThreadPoolTest test_using_thread_pool.cpp)

add_executable(TestCompoundTaskCanceling test_compound_task_cancelling.cpp)