//
// Created by Naum Puroski on 27/03/2017.
//
#include "CompoundTask.h"

CompoundTask::CompoundTask() : _thread_pool{std::make_shared<ThreadPool>(ThreadPool::globalInstance(), 1)} {}

CompoundTask::~CompoundTask() {
    stop();
}

void CompoundTask::stop() {
    for (auto& task : _tasks) {
        task->stop();
    }

    _thread_pool->stop();
}