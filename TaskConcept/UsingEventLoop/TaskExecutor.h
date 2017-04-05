//
// Created by Naum Puroski on 05/04/2017.
//

#ifndef USINGTHREADPOOL_TASKEXECUTOR_H
#define USINGTHREADPOOL_TASKEXECUTOR_H

#include <functional>

class TaskExecutor {
public:
    virtual ~TaskExecutor() = default;

    virtual void execute(std::function<void()> task) = 0;
    virtual void stop(std::function<void()> cancel_tasks = std::function<void()>()) = 0;
};
#endif //USINGTHREADPOOL_TASKEXECUTOR_H
