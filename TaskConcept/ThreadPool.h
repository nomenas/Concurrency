//
// Created by Naum Puroski on 23/03/2017.
//

#ifndef TASKCONCEPT_THREADPOOL_H
#define TASKCONCEPT_THREADPOOL_H

#include <queue>

#include "Task.h"
#include "TaskExecutor.h"

class ThreadPool : public TaskExecutor {
public:
    ThreadPool(int number_of_threads = 1)
            : _number_of_threads(1) {
        for (int i = 0; i < _number_of_threads; ++i) {
            _threads.push_back(std::thread(std::bind(&ThreadPool::event_loop, this)));
        }
    }

    ~ThreadPool() {
        stop();
        std::for_each(_threads.begin(), _threads.end(), [](std::thread& thread) {thread.join();});
    }

    ThreadPool& run(std::function<void()> task) {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.push(std::move(task));
        _tasks_condition.notify_one();

        return *this;
    }

    void run(Task* task, Callback callback) override {
        run([task, callback](){task->run(callback);});
    }

    void cancel(Task* task) override {
        // TODO
    }

    void stop() {
        _stopped = true;
        _tasks_condition.notify_all();
    }

protected:
    void event_loop() {
        while (!_stopped) {

            if (_tasks.empty()) {
                std::unique_lock<std::mutex> lock(_tasks_mutex);
                _tasks_condition.wait(lock);
            }

            {
                std::lock_guard<std::mutex> lock_guard{_tasks_mutex};

                if (_tasks.size()) {
                    _tasks.front()();
                    _tasks.pop();
                }
            }
        }
    }

private:
    bool _stopped = false;
    int _number_of_threads = 0;
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_condition;
};

#endif //TASKCONCEPT_THREADPOOL_H
