//
// Created by Naum Puroski on 23/03/2017.
//

#ifndef TASKCONCEPT_THREADPOOL_H
#define TASKCONCEPT_THREADPOOL_H

#include <queue>
#include <thread>

class ThreadPool {
public:
    static ThreadPool& globalInstance();

    ThreadPool(ThreadPool& thread_pool, int number_of_threads = 1);
    ThreadPool(int number_of_threads = 4);
    ~ThreadPool();

    void execute(std::function<void()> task);
    void stop(std::function<void()> cancel_tasks = std::function<void()>());

protected:
    void event_loop();
    std::function<void()> pop_first();

private:
    bool _stopped = false;
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_condition;
    const int _number_of_reserved_threads = 0;
    int finished_threads = 0;
    std::mutex _finish_thread_mutex;
    std::condition_variable _finish_thread_condition;
};

#endif //TASKCONCEPT_THREADPOOL_H
