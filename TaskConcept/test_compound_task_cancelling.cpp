//
// Created by Naum Puroski on 24/03/2017.
//

#include "Task.h"

class SingleTask : public Task {
public:
    SingleTask(Callback<SingleTask> callback = Callback<SingleTask>())
            : Task(create_task_callback(callback)) {
    }

    ~SingleTask() {

    }

protected:
    void execute() override {

    }
};

class ParallelTask : public Task {
public:
    ParallelTask(Callback<ParallelTask> callback = Callback<ParallelTask>())
            : Task(create_task_callback(callback)) {
    }

    ~ParallelTask() {
    }

private:
    void execute() override {
    }
};

class CompoundTask : public Task {
public:
    CompoundTask(Callback<CompoundTask> callback = Callback<CompoundTask>())
            : Task(create_task_callback(callback)) {}
protected:

    void execute() override {
        create_task<SingleTask>([this](SingleTask* task){

            for (int i = 0; i < 10; ++i) {
                create_task<ParallelTask>([this](ParallelTask* task) {
                    if (_counter == 10) {
                        mark_as_done();
                    }
                }).run();
            }
        }).run();
    }

private:
    int _counter =0;
};

int main () {
    return 0;
}