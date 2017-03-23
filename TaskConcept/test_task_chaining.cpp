//
// Created by Naum Puroski on 23/03/2017.
//

#include <iostream>

#include "Task.h"
#include "ThreadPool.h"
#include "ConcreteTasks.h"

class exec {
public:
    exec(Task& task) {
        _recived_callbacks = 0;
        _pool.run(&task, std::bind(&exec::call_next, this, 0));
    }

    ~exec() {
        if (_recived_callbacks != _tasks.size() + 1) {
            std::unique_lock<std::mutex> lock{_mutex};
            _wait_for_done_condition.wait(lock, [this](){return _done;});
        }
    }

    exec& then(Task& task) {
        _tasks.push_back(&task);
        return *this;
    }

private:
    void call_next(int index) {
        ++_recived_callbacks;
        if (_tasks.size() > index) {
            _pool.run(_tasks[index], std::bind(&exec::call_next, this, index + 1));
        } else {
            _done = true;
            _wait_for_done_condition.notify_all();
        }

    }
    std::atomic<int> _recived_callbacks;
    std::mutex _mutex;
    bool _done = false;
    std::condition_variable _wait_for_done_condition;
    std::vector<Task*> _tasks;
    ThreadPool _pool = {5};
};

int main() {
    CompoundTask task1{20};
    CompoundTask task2{50};
    CompoundTask task3{100};
    CompoundTask task4{200};

    exec(task1).then(task2).then(task3).then(task4);

    std::cout << "task1 result: " << task1.result() << std::endl;
    std::cout << "task2 result: " << task2.result() << std::endl;
    std::cout << "task3 result: " << task3.result() << std::endl;
    std::cout << "task4 result: " << task4.result() << std::endl;
}