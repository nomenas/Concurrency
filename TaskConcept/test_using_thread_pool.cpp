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
    auto task1 = create_task<SingleTask>(15);
    auto task2 = create_task<CompoundTask>(5, [&task1_done](Task*){task1_done.set_value();});
    auto task3 = create_task<CompoundTask>(10, [&task2_done](Task*){task2_done.set_value();});

    ThreadPool::globalInstance().execute(task1.get());
    ThreadPool::globalInstance().execute(task2.get());
    ThreadPool::globalInstance().execute(task3.get());
    task1_done.get_future().wait();
    task2_done.get_future().wait();
    std::cout << "task1 result size: " << task1->items().size() << std::endl;
    std::cout << "task2 result: " << task2->sum() << std::endl;
    std::cout << "task3 result: " << task3->sum() << std::endl;

    return 0;
}
