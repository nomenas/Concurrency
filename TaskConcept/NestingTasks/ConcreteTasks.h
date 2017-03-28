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

    const std::vector<int>& items() const {
        return _result;
    }

protected:
    void execute() override {
        // simulate asynchronous interruptible action that takes 2 sec
        _thread.reset(new std::thread([this]() {
            {
                std::unique_lock<std::mutex> lock{_mutex};
                _condition.wait_for(lock, std::chrono::seconds(1), [this]()-> bool{return _condition_done;});
            }

            handler();
        }));
    }

    void stop() override {
        // simulate stop
        {
            {
                std::lock_guard<std::mutex> lock{_mutex};
                _condition_done = true;
                _condition.notify_all();
            }

            if (_thread) {
                _thread->join();
            }
        }

        // in normal case this method should call just one api like:
        // http_request->cancel();
    }

private:
    void handler() {
        // _condition_done is true just if task was not canceled i.e do this just when there are valid results
        if (!_condition_done) {
            _result.resize(_elements);
            std::iota(_result.begin(), _result.end(), 0);
        }

        // process response ...

        done();
    }

    // members used for simulation
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _condition_done = false;
    std::unique_ptr<std::thread> _thread;

    // task members
    int _elements = 0;
    std::vector<int> _result;
};

class ParallelTask : public Task {
public:
    ParallelTask(int value, Callback<ParallelTask> callback = Callback<ParallelTask>())
            : Task(create_task_callback(callback))
            , _value(value) {

    }

    int value() const {
        return _value;
    }

    int result() const {
        return _result;
    }

protected:
    void execute() override {
        // simulate asynchronous interruptible action that takes 2 sec
        _thread.reset(new std::thread([this]() {
            {
                std::unique_lock<std::mutex> lock{_mutex};
                _condition.wait_for(lock, std::chrono::milliseconds(100), [this]()-> bool{return _condition_done;});
            }

            handler();
        }));
    }

    void stop() override {
        // simulate stop
        {
            {
                std::lock_guard<std::mutex> lock{_mutex};
                _condition_done = true;
                _condition.notify_all();
            }

            if (_thread) {
                _thread->join();
            }
        }

        // in normal case this method should call just one api like:
        // http_request->cancel();
    }

private:
    void handler() {
        if (!_condition_done) {
            _result = _value * 2;
        }

        done();
    }

    // members used for simulation
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _condition_done = false;
    std::unique_ptr<std::thread> _thread;

    int _value = 0;
    int _result = 0;
};

class CompoundTask : public Task {
public:
    CompoundTask(int value, Callback<CompoundTask> callback = Callback<CompoundTask>())
            : Task(create_task_callback(callback))
            , _value(value) {}

    int sum() const {
        return _agregate_result;
    }

protected:
    void execute() override {
        run_task<SingleTask>(_value, [this](SingleTask* task){
            _agregate_result = 0;

            if (task->items().size() == 0) {
                done();
            } else {
                _counter = 0;
                _expected_results = task->items().size() - 1;

                for (auto item : task->items()) {
                    run_task<ParallelTask>(item, [this](ParallelTask* task) {
                        _agregate_result += task->result();

                        if (_counter.fetch_add(1) == _expected_results) {
                            done();
                        }
                    });
                }
            }
        });
    }

private:
    int _value = 0;
    std::atomic<int> _counter;
    int _expected_results = 0;
    std::atomic<int> _agregate_result;
};

using UPNPSearch = SingleTask;
using CheckIP = ParallelTask;

class BridgeDiscovery : public Task {
public:
    BridgeDiscovery(int value, Callback<CompoundTask> callback = Callback<CompoundTask>())
            : Task(create_task_callback(callback))
            , _value(value) {}

    int sum() const {
        return _agregate_result;
    }

protected:
    void execute() override {
        run_task<UPNPSearch>(_value, [this](UPNPSearch* task) {
            if (task->items().size() == 0) {
                done();
            } else {
                run_tasks<CheckIP>(task->items(), [this](const std::vector<CheckIP*>& tasks) {
                    for (auto task : tasks) {
                        _agregate_result += task->result();
                    }
                    done();
                });
            }
        });
    }

private:
    int _value = 0;
    int _agregate_result = 0;
};

#endif //TASKCONCEPT_CONCRETETASKS_H
