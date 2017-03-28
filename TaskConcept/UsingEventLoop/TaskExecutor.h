//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASKEXECUTOR_H
#define WITHTASKEXECUTOR_TASKEXECUTOR_H

#include "ThreadPool.h"
#include "Task.h"

class TaskExecutor {
public:
    TaskExecutor();
    ~TaskExecutor();

    void stop();

protected:

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

private:
    std::mutex _tasks_mutex;
    std::vector<std::unique_ptr<Task>> _tasks;
    std::shared_ptr<ThreadPool> _thread_pool;
};

#endif //WITHTASKEXECUTOR_TASKEXECUTOR_H
