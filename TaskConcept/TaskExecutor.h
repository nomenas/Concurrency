//
// Created by Naum Puroski on 22/03/2017.
//

#ifndef TASKCONCEPT_TASKEXECUTOR_H
#define TASKCONCEPT_TASKEXECUTOR_H

class Task;

class TaskExecutor {
public:
    using Callback = std::function<void(Task*)>;

    virtual void run(Task* task, Callback callback) = 0;
    virtual void cancel(Task* task) = 0;
};

#endif //TASKCONCEPT_TASKEXECUTOR_H
