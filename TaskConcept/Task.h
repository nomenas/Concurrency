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

    virtual ~Task() {
        cancel();
    }

    virtual void execute(Callback callback) = 0;

    virtual void cancel() {
        _canceled = true;

        if (_executor) {
            _executor->cancel(this);
        }
    }

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

    template<typename T, typename... Args>
    Task& task(Args... args) {
        std::unique_ptr<T> sub_task{new T{args...}};
        Task& return_value = *sub_task;
        _sub_tasks.push_back(std::move(sub_task));

        if (_executor) {
            _executor->execute(sub_task.get());
        }

        return return_value;
    };

private:
    bool _canceled = false;
    std::vector<std::unique_ptr<Task>> _sub_tasks;
    TaskExecutor* _executor = nullptr;
};

#endif //TASKCONCEPT_TASK_H
