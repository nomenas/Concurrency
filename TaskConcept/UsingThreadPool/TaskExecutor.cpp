//
// Created by Naum Puroski on 27/03/2017.
//
#include "TaskExecutor.h"

TaskExecutor::TaskExecutor() : _thread_pool{1} {}

TaskExecutor::~TaskExecutor() {
    for(auto& task : _tasks) {
        task->stop();
    }

    _thread_pool.stop();
}