//
// Created by Naum Puroski on 22/03/2017.
//

#ifndef TASKCONCEPT_TASK_H
#define TASKCONCEPT_TASK_H

#include "TaskExecutor.h"

#include <functional>
#include <vector>
#include <numeric>
#include <future>
#include <utility>

class Task {
public:
    using Callback = std::function<void(Task*)>;

    explicit Task(Callback callback) : _callback(std::move(callback)) {}

    virtual ~Task() {
        cancel();
    }

    // run task and provide callback that would be called when tasks finishes
    Task& run(bool force_synchronous_mode = false) {
        if (_executor && !force_synchronous_mode) {
            _executor->execute(this);
        } else {
            execute();
        }

        return *this;
    }

    // run task and wait for results
    template <typename T = Task>
    T& wait() {
        _promise.get_future().wait();
        return *static_cast<T*>(this);
    }

    // cancel tasks
    virtual void cancel() {
        _canceled = true;

        if (_executor) {
            _executor->cancel(this);
        }
    }

    void set_executor(TaskExecutor* executor) {
        _executor = executor;
    }

    bool is_canceled() const {
        return _canceled;
    }

protected:

    virtual void execute() = 0;

    template<typename T, typename... Args>
    Task& create_task(Args... args) {
        std::unique_ptr<T> sub_task{new T{args...}};
        sub_task->set_executor(_executor);
        Task& return_value = *sub_task;
        _sub_tasks.push_back(std::move(sub_task));
        return return_value;
    }

    void mark_as_done() {
        _promise.set_value();
        if (_callback) {
            _callback(this);
        }
    }

private:
    bool _canceled = false;
    Callback _callback;
    std::promise<void> _promise;
    std::vector<std::unique_ptr<Task>> _sub_tasks;
    TaskExecutor* _executor = nullptr;
};

#endif //TASKCONCEPT_TASK_H
