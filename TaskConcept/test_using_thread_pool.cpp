//
// Created by Naum Puroski on 23/03/2017.
//

#include <iostream>
#include <queue>

#include "ThreadPool.h"
#include "ConcreteTasks.h"

int main() {
    std::promise<void> task1_done;
    std::promise<void> task2_done;
    SingleTask task1{15};
    CompoundTask task2{5, [&task1_done](Task*){task1_done.set_value();}};
    CompoundTask task3{10, [&task2_done](Task*){task2_done.set_value();}};

    ThreadPool::globalInstance().execute(&task1);
    ThreadPool::globalInstance().execute(&task2);
    ThreadPool::globalInstance().execute(&task3);
    task1_done.get_future().wait();
    task2_done.get_future().wait();
    std::cout << "task1 result size: " << task1.result().size() << std::endl;
    std::cout << "task2 result: " << task2.result() << std::endl;
    std::cout << "task3 result: " << task3.result() << std::endl;

    return 0;
}
