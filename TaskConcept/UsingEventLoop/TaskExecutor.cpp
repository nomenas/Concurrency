//
// Created by Naum Puroski on 27/03/2017.
//
#include "TaskExecutor.h"

TaskExecutor::TaskExecutor() : _thread_pool{std::make_shared<ThreadPool>(ThreadPool::globalInstance(), 1)} {}

TaskExecutor::~TaskExecutor() {
    stop();
}

void TaskExecutor::stop() {
    for (auto& task : _tasks) {
        task->stop();
    }

    _thread_pool->stop();
}