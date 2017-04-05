//
// Created by Naum Puroski on 05/04/2017.
//

#ifndef USINGTHREADPOOL_THREADPOOLTASKEXECUTOR_H
#define USINGTHREADPOOL_THREADPOOLTASKEXECUTOR_H

#include "TaskExecutor.h"
#include "ThreadPool.h"

template <int Number_Of_Threads = 1>
class ThreadPoolTaskExecutor : public TaskExecutor {
public:
    ThreadPoolTaskExecutor() : _thread_pool{ThreadPool::globalInstance(), Number_Of_Threads}{};

    void execute(std::function<void()> task) override {
        _thread_pool.execute(std::move(task));
    }

    void stop(std::function<void()> cancel_tasks = std::function<void()>()) override {
        _thread_pool.stop(std::move(cancel_tasks));
    }

private:
    ThreadPool _thread_pool;
};


#endif //USINGTHREADPOOL_THREADPOOLTASKEXECUTOR_H
