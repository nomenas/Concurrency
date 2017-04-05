//
// Created by Naum Puroski on 23/03/2017.
//

#ifndef TASKCONCEPT_THREADPOOL_H
#define TASKCONCEPT_THREADPOOL_H

#include <queue>
#include <thread>

class ThreadPool {
public:
    using Task = std::function<void()>;
    static ThreadPool& globalInstance();

    ThreadPool(ThreadPool& thread_pool, int number_of_threads = 1);
    ThreadPool(int number_of_threads = 4);
    ~ThreadPool();

    void execute(Task task);
    void stop(Task cancel_tasks = Task());

protected:
    void event_loop();
    std::function<void()> pop_first();

private:
    bool _stopped = false;
    std::vector<std::thread> _threads;
    std::queue<Task> _tasks;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_condition;
    const int _number_of_reserved_threads = 0;
    int finished_threads = 0;
    std::mutex _finish_thread_mutex;
    std::condition_variable _finish_thread_condition;
};

#endif //TASKCONCEPT_THREADPOOL_H
