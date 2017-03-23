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
    static ThreadPool& globalInstance() {
        static ThreadPool _instance;
        return _instance;
    }

    ThreadPool(int number_of_threads = 4)
            : _number_of_threads(number_of_threads) {
        for (int i = 0; i < _number_of_threads; ++i) {
            _threads.push_back(std::thread(std::bind(&ThreadPool::event_loop, this)));
        }
    }

    ~ThreadPool() {
        stop();
        std::for_each(_threads.begin(), _threads.end(), [](std::thread& thread) {thread.join();});
    }

    ThreadPool& execute(std::function<void()> task) {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _tasks.push(std::move(task));
        _tasks_condition.notify_one();

        return *this;
    }

    void execute(Task* task) override {
        task->set_executor(this);
        execute([task](){task->run(true);});
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

            auto function = pop_first();

            if (function) {
                function();
            }
        }
    }

    std::function<void()> pop_first() {
        std::function<void()> return_value;
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};

        if (_tasks.size()) {
            return_value = _tasks.front();
            _tasks.pop();
        }

        return return_value;
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
