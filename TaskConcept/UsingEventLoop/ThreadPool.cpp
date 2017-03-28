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

ThreadPool& ThreadPool::execute(std::function<void()> task) {
    std::lock_guard<std::mutex> lock_guard{_tasks_mutex};
    _tasks.push(task);
    _tasks_condition.notify_one();

    return *this;
}

void ThreadPool::stop() {
    _stopped = true;
    _tasks_condition.notify_all();

    std::for_each(_threads.begin(), _threads.end(), [](std::thread& thread) {thread.join();});
    std::unique_lock<std::mutex> lock{_finish_thread_mutex};
    _finish_thread_condition.wait(lock, [this]()-> bool{return finished_threads >= _number_of_reserved_threads;});
}

void ThreadPool::event_loop() {
    while (!_stopped) {
        auto function = pop_first();

        if (function) {
            function();
        }
    }

    {
        std::unique_lock<std::mutex> lock{_finish_thread_mutex};
        ++finished_threads;
        _finish_thread_condition.notify_all();
    }
}

std::function<void()> ThreadPool::pop_first() {
    std::function<void()> return_value;
    std::unique_lock<std::mutex> lock{_tasks_mutex};

    if (_tasks.empty()) {
        _tasks_condition.wait(lock);
    }

    if (_tasks.size()) {
        return_value = _tasks.front();
        _tasks.pop();
    }

    return return_value;
}