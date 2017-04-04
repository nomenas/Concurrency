//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASKEXECUTOR_H
#define WITHTASKEXECUTOR_TASKEXECUTOR_H

#include "ThreadPool.h"
#include "Task.h"

class CompoundTask {
public:
    using Callback = std::function<void(CompoundTask*)>;

    CompoundTask()
            : _thread_pool{std::make_shared<ThreadPool>(ThreadPool::globalInstance(), 1)} {
    }

    virtual ~CompoundTask() {
        stop();
        done();
    }

    void run(Callback callback = Callback()) {
        _callback = callback;
        execute();
    }

    void stop() {
        std::vector<std::unique_ptr<Task>> active_tasks;

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

    class Executor {
    public:
        Executor(std::shared_ptr<ThreadPool> thread_pool, Task* task) : _thread_pool(thread_pool), _task(task) {};

        void execute(Task::Callback callback) {
            auto thread_pool = _thread_pool;
            _task->set_completion_callback([thread_pool, callback](Task* task) {
                thread_pool->execute([callback, task](){callback(task);});
            });
            auto task = _task;
            _thread_pool->execute([task](){task->execute();});
        }
    private:
        Task* _task = nullptr;
        std::shared_ptr<ThreadPool> _thread_pool;
    };

    template<typename T, typename... Args>
    Executor create_task(Args... args) {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.emplace_back(new T{std::forward<Args>(args)...});
        return Executor(_thread_pool, _tasks.back().get());
    }

    void done() {
        if (!_is_done.exchange(true)) {
            if (_callback) {
                _callback(this);
            }
        }
    }

private:
    std::mutex _tasks_mutex;
    std::vector<std::unique_ptr<Task>> _tasks;
    std::shared_ptr<ThreadPool> _thread_pool;
    std::atomic<bool> _is_done{false};
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASKEXECUTOR_H
