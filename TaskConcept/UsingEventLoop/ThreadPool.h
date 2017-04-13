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
    static ThreadPool& global();

    ThreadPool(ThreadPool& thread_pool, int number_of_threads = 1);
    ThreadPool(int number_of_threads = 4);
    ~ThreadPool();

    void execute(Task task);
    void stop(Task cancel_tasks = Task());

protected:
    void event_loop(bool is_reused);
    std::function<void()> pop_first();

private:
    bool _stopped = false;
    std::vector<std::thread> _threads;
    std::queue<Task> _tasks;
    std::mutex _tasks_mutex;
    std::condition_variable _tasks_condition;
    int _number_of_active_reused_threads = 0;
    std::mutex _thread_managment_mutex;
    std::condition_variable _finish_thread_condition;
};

#endif //TASKCONCEPT_THREADPOOL_H
