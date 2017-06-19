//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASK_H
#define WITHTASKEXECUTOR_TASK_H

#include "TaskExecutor.h"

class Task {
public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() {
        cancel();
        done();
    }

    void run(Callback callback = Callback()) {
        if (!_is_run.exchange(true)) {
            if (!_task_executor) {
                _task_executor = std::make_shared<TaskExecutor>();
            }

            auto task = _this_task.lock();
            if (task) {
                auto task_executor = _task_executor;
                _callback = [task_executor, callback](Task* task) {
                    task_executor->execute([callback, task](){callback(task);});
                };
                task_executor->execute([task](){
                    task->execute();
                });
            } else {
                _callback = callback;
                execute();
            }
        }
    }

    void cancel() {
        _is_canceled = true;
        std::vector<std::shared_ptr<Task>> active_tasks;

        if (_task_executor) {
            _task_executor->stop([&](){
                {
                    std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
                    active_tasks = std::move(_tasks);
                }

                for (auto& task : active_tasks) {
                    task->stop();
                }
            });
        }

        done();
    }

    void wait() {
        if (!_is_done) {
            _callback_promise->get_future().wait();
        }
    }

    template <typename T>
    T& get_result() {
        run();
        wait();
        return static_cast<T&>(*this);
    }

    bool is_done() const {
        return _is_done;
    }

    bool is_run() const {
        return _is_run;
    }

    bool is_canceled() const {
        return _is_canceled;
    }

protected:
    virtual void execute() = 0;
    virtual void stop() {}

    void done() {
        auto shared_callback_promise = _callback_promise;
        if (!_is_done.exchange(true)) {
            if (_callback) {
                _callback(this);
            }
            shared_callback_promise->set_value();
        }
    }

    template<typename T, typename... Args>
    std::shared_ptr<Task> create_task(Args... args) {
        auto return_value = std::make_shared<T>(std::forward<Args>(args)...);
        return_value->_this_task = return_value;
        return_value->_task_executor = _task_executor;

        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.push_back(return_value);

        return return_value;
    }

private:
    std::vector<std::shared_ptr<Task>> _tasks;
    std::weak_ptr<Task> _this_task;
    std::mutex _tasks_mutex;
    std::shared_ptr<TaskExecutor> _task_executor;

    std::atomic<bool> _is_run{false};
    std::atomic<bool> _is_done{false};
    std::atomic<bool> _is_canceled{false};
    std::shared_ptr<std::promise<void>> _callback_promise = std::make_shared<std::promise<void>>();
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASK_H
