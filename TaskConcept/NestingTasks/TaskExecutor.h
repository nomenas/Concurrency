//
// Created by Naum Puroski on 22/03/2017.
//

#ifndef TASKCONCEPT_TASKEXECUTOR_H
#define TASKCONCEPT_TASKEXECUTOR_H

class Task;

class TaskExecutor {
public:
    virtual void execute(Task* task) = 0;
    virtual void cancel(Task* task) = 0;
};

#endif //TASKCONCEPT_TASKEXECUTOR_H
