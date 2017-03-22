#include <iostream>

#include "Task.h"

class SingleTask : public Task {
public:
    SingleTask(int elements) : _elements(elements) {

    }

    ~SingleTask() {
        _thread->join();
    }

    void execute(Callback callback) override {
        _callback = callback;
        _thread.reset(new std::thread([this, callback](){
            handler();
        }));
    }

    const std::vector<int>& result() const {
        return _result;
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

    void execute(Callback callback) override {
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
    void execute(Callback callback) override {

        task<SingleTask>(10).execute([this, callback](Task* task){
            SingleTask* single_task = static_cast<SingleTask*>(task);
            _counter = 0;
            _agregate_result = 0;
            _expected_results = single_task->result().size();

            for (auto item : single_task->result()) {
                this->task<ParallelTask>(item).execute([this, callback](Task* task) {
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
    std::atomic<int> _counter;
    int _expected_results = 0;
    std::atomic<int> _agregate_result;
};


int main() {
    std::cout << "result: " << CompoundTask().get_result<CompoundTask>().result() << std::endl;
    return 0;
}