//
// Created by Naum Puroski on 23/03/2017.
//

#ifndef TASKCONCEPT_CONCRETETASKS_H
#define TASKCONCEPT_CONCRETETASKS_H

#include "Task.h"

class SingleTask : public Task {
public:
    SingleTask(int elements, Callback<SingleTask> callback = Callback<SingleTask>())
            : Task(create_task_callback(callback))
            , _elements(elements) {

    }

    ~SingleTask() {
        _thread->join();
    }

        const std::vector<int>& result() const {
        return _result;
    }

protected:
    void execute() override {
        _thread.reset(new std::thread([this](){
            handler();
        }));
    }

private:
    void handler() {
        _result.resize(_elements);
        std::iota(_result.begin(), _result.end(), 0);
        mark_as_done();
    }

    int _elements = 0;
    std::vector<int> _result;
    std::unique_ptr<std::thread> _thread;
};

class ParallelTask : public Task {
public:
    ParallelTask(int value, Callback<ParallelTask> callback = Callback<ParallelTask>())
            : Task(create_task_callback(callback))
            , _value(value) {

    }

    ~ParallelTask() {
        _thread->join();
    }

    void execute() override {
        _thread.reset(new std::thread([this](){
            handler();
        }));
    }

    int value() const {
        return _value;
    }

    int result() const {
        return _result;
    }

private:
    void handler() {
        _result = _value * 2;
        mark_as_done();
    }

    int _value = 0;
    int _result = 0;
    std::unique_ptr<std::thread> _thread;
};

class CompoundTask : public Task {
public:
    CompoundTask(int value, Callback<CompoundTask> callback = Callback<CompoundTask>())
            : Task(create_task_callback(callback))
            , _value(value) {}

    void execute() override {

        create_task<SingleTask>(_value, [this](SingleTask* task){
            _counter = 0;
            _agregate_result = 0;
            _expected_results = task->result().size();

            for (auto item : task->result()) {
                create_task<ParallelTask>(item, [this](ParallelTask* task) {
                    ++_counter;
                    _agregate_result += task->result();

                    if (_counter == _expected_results) {
                        mark_as_done();
                    }
                }).run();
            }
        }).run();
    }

    int result() const {
        return _agregate_result;
    }

private:
    int _value = 0;
    std::atomic<int> _counter;
    int _expected_results = 0;
    std::atomic<int> _agregate_result;
};

#endif //TASKCONCEPT_CONCRETETASKS_H
