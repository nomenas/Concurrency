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
    friend class TaskExecutor;

public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() {
        cancel();
    }

    // run task and provide callback that would be called when tasks finishes
    void run(Callback callback) {
        if (_executor) {
            _executor->run(this, callback);
        } else {
            execute(callback);
        }
    }

    // run task and continue without waiting for results
    void run() {
        run(Callback());
    }

    // cancel tasks
    virtual void cancel() {
        _canceled = true;

        if (_executor) {
            _executor->cancel(this);
        }
    }

    // run task and wait for results
    template <typename T>
    T& get_result() {
        std::promise<void> promise;
        execute([&promise](Task*){promise.set_value();});
        promise.get_future().wait();

        return *static_cast<T*>(this);
    }

    void set_executor(TaskExecutor* executor) {
        _executor = executor;
    }

    bool is_canceled() const {
        return _canceled;
    }

protected:
    virtual void execute(Callback callback) = 0;

    template<typename T, typename... Args>
    Task& task(Args... args) {
        std::unique_ptr<T> sub_task{new T{args...}};
        Task& return_value = *sub_task;
        _sub_tasks.push_back(std::move(sub_task));
        return return_value;
    };

private:
    bool _canceled = false;
    std::vector<std::unique_ptr<Task>> _sub_tasks;
    TaskExecutor* _executor = nullptr;
};

#endif //TASKCONCEPT_TASK_H
