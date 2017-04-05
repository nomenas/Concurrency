//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASK_H
#define WITHTASKEXECUTOR_TASK_H

#include "ThreadPool.h"

class Task {
public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() {
        cancel();
        done();
    }

    void run(Callback callback = Callback()) {
        if (!_thread_pool) {
            _thread_pool = std::make_shared<ThreadPool>(ThreadPool::globalInstance(), 1);
        }
        _callback = callback;
        execute();
    }

    void cancel() {
        std::vector<std::shared_ptr<Task>> active_tasks;

        _thread_pool->stop([&](){
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
        Executor(std::shared_ptr<ThreadPool> thread_pool, std::shared_ptr<Task> task) : _thread_pool(thread_pool), _task(task) {};

        void execute(Callback callback) {
            auto thread_pool = _thread_pool;
            auto task_callback = [thread_pool, callback](Task* task) {
                thread_pool->execute([callback, task](){callback(task);});
            };
            auto task = _task;
            task->_thread_pool = thread_pool;
            _thread_pool->execute([task, task_callback](){task->run(task_callback);});
        }
    private:
        std::shared_ptr<Task> _task;
        std::shared_ptr<ThreadPool> _thread_pool;
    };

    template<typename T, typename... Args>
    Executor create_task(Args... args) {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.emplace_back(new T{std::forward<Args>(args)...});
        return Executor(_thread_pool, _tasks.back());
    }

private:
    // child tasks
    std::mutex _tasks_mutex;
    std::vector<std::shared_ptr<Task>> _tasks;
    std::shared_ptr<ThreadPool> _thread_pool;

    // callback
    std::atomic<bool> _is_done{false};
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASK_H
