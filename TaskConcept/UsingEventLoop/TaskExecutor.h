//
// Created by Naum Puroski on 05/04/2017.
//

#ifndef USINGTHREADPOOL_TASKEXECUTOR_H
#define USINGTHREADPOOL_TASKEXECUTOR_H

#include <functional>

#include "ThreadPool.h"

class TaskExecutor {
public:
    void execute(std::function<void()> task) {
        _thread_pool.execute(std::move(task));
    }

    void stop(std::function<void()> cancel_tasks = std::function<void()>()) {
        _thread_pool.stop(std::move(cancel_tasks));
    }

private:
    ThreadPool _thread_pool{ThreadPool::global(), 1};
};

#endif //USINGTHREADPOOL_TASKEXECUTOR_H
