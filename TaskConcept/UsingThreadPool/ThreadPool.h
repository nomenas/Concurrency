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

    ThreadPool(ThreadPool& thread_pool, int number_of_reserverd_threads);
    ThreadPool(int number_of_threads = 4);
    ~ThreadPool();

    ThreadPool& execute(std::function<void()> task);
    void stop();

protected:
    void event_loop();
    std::function<void()> pop_first();

private:
    bool _stopped = false;
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _tasks;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_condition;
};

#endif //TASKCONCEPT_THREADPOOL_H
