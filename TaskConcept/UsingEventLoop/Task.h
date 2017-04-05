//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASK_H
#define WITHTASKEXECUTOR_TASK_H

#include "ThreadPoolTaskExecutor.h"

class Task {
public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() {
        cancel();
        done();
    }

    template <typename Executor = ThreadPoolTaskExecutor<1>>
    void run(Callback callback = Callback()) {
        if (!_task_executor) {
            _task_executor = std::make_shared<Executor>();
        }
        _callback = callback;
        execute();
    }

    void cancel() {
        std::vector<std::shared_ptr<Task>> active_tasks;

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

protected:
    virtual void execute() = 0;
    virtual void stop() {};

    void done() {
        if (!_is_done.exchange(true)) {
            if (_callback) {
                _callback(this);
            }
        }
    }

    class Executor {
    public:
        Executor(std::shared_ptr<TaskExecutor> task_executor, std::shared_ptr<Task> task) : _task_executor(task_executor), _task(task) {};

        void execute(Callback callback) {
            auto task_executor = _task_executor;
            auto task_callback = [task_executor, callback](Task* task) {
                task_executor->execute([callback, task](){callback(task);});
            };
            auto task = _task;
            task->_task_executor = task_executor;
            task_executor->execute([task, task_callback](){task->run(task_callback);});
        }
    private:
        std::shared_ptr<Task> _task;
        std::shared_ptr<TaskExecutor> _task_executor;
    };

    template<typename T, typename... Args>
    Executor create_task(Args... args) {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.emplace_back(new T{std::forward<Args>(args)...});
        return Executor(_task_executor, _tasks.back());
    }

private:
    // child tasks
    std::mutex _tasks_mutex;
    std::vector<std::shared_ptr<Task>> _tasks;
    std::shared_ptr<TaskExecutor> _task_executor;

    // callback
    std::atomic<bool> _is_done{false};
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASK_H
