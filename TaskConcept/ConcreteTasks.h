//
// Created by Naum Puroski on 23/03/2017.
//

#ifndef TASKCONCEPT_CONCRETETASKS_H
#define TASKCONCEPT_CONCRETETASKS_H

#include "Task.h"

class SingleTask : public Task {
public:
    SingleTask(int elements) : _elements(elements) {

    }

    ~SingleTask() {
        _thread->join();
    }

    const std::vector<int>& result() const {
        return _result;
    }

protected:
    void operator()(Callback callback) override {
        _callback = callback;
        _thread.reset(new std::thread([this, callback](){
            handler();
        }));
    }

private:
    void handler() {
        _result.resize(_elements);
        std::iota(_result.begin(), _result.end(), 0);
        _callback(this);
    }

    Callback _callback;
    int _elements = 0;
    std::vector<int> _result;
    std::unique_ptr<std::thread> _thread;
};

class ParallelTask : public Task {
public:
    ParallelTask(int value) : _value(value) {

    }

    ~ParallelTask() {
        _thread->join();
    }

    void operator()(Callback callback) override {
        _callback = std::move(callback);
        _thread.reset(new std::thread([this, callback](){
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
        _callback(this);
    }

    Callback _callback;
    int _value = 0;
    int _result = 0;
    std::unique_ptr<std::thread> _thread;
};

class CompoundTask : public Task {
public:
    CompoundTask(int value) : _value(value) {}

    void operator()(Callback callback) override {

        create_task<SingleTask>(_value).run([this, callback](Task* task){
            SingleTask* single_task = static_cast<SingleTask*>(task);

            _counter = 0;
            _agregate_result = 0;
            _expected_results = single_task->result().size();

            for (auto item : single_task->result()) {
                create_task<ParallelTask>(item).run([this, callback](Task* task) {
                    ++_counter;
                    _agregate_result += static_cast<ParallelTask*>(task)->result();

                    if (_counter == _expected_results) {
                        callback(this);
                    }
                });
            }
        });
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
