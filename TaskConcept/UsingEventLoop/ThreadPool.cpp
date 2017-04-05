//
// Created by Naum Puroski on 27/03/2017.
//

#include "ThreadPool.h"

ThreadPool& ThreadPool::globalInstance() {
    static ThreadPool _instance;
    return _instance;
}

ThreadPool::ThreadPool(ThreadPool& thread_pool, int number_of_threads) : _number_of_reserved_threads(number_of_threads) {
    for (int i = 0; i < number_of_threads; ++i) {
        thread_pool.execute(std::bind(&ThreadPool::event_loop, this));
    }
}

ThreadPool::ThreadPool(int number_of_threads /* = 4*/) {
    for (int i = 0; i < number_of_threads; ++i) {
        _threads.push_back(std::thread(std::bind(&ThreadPool::event_loop, this)));
    }
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::execute(Task task) {
    std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
    _tasks.push(task);
    _tasks_condition.notify_one();
}

void ThreadPool::stop(Task cancel_tasks /* = Task() */) {
    {
        std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
        _stopped = true;
        _tasks_condition.notify_all();
    }

    if (cancel_tasks) {
        cancel_tasks();
    }

    std::for_each(_threads.begin(), _threads.end(), [](std::thread& thread) {thread.join();});
    std::unique_lock<std::mutex> lock{_finish_thread_mutex};
    _finish_thread_condition.wait(lock, [this]()-> bool{return finished_threads >= _number_of_reserved_threads;});
}

void ThreadPool::event_loop() {
    while (!_stopped) {
        auto function = pop_first();

        if (function && !_stopped) {
            function();
        }
    }

    std::unique_lock<std::mutex> lock{_finish_thread_mutex};
    ++finished_threads;
    _finish_thread_condition.notify_all();
}

ThreadPool::Task ThreadPool::pop_first() {
    std::unique_lock<std::mutex> lock{_tasks_mutex};
    Task return_value;

    if (!_stopped && _tasks.empty()) {
        _tasks_condition.wait(lock);
    }

    if (!_stopped && _tasks.size()) {
        return_value = _tasks.front();
        _tasks.pop();
    }

    return return_value;
}