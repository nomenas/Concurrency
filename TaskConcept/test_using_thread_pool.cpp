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
    CompoundTask task1{5};
    CompoundTask task2{10};

    ThreadPool pool{5};
    pool.run(&task1, [&task1_done](Task*){task1_done.set_value();});
    pool.run(&task2, [&task2_done](Task*){task2_done.set_value();});
    task1_done.get_future().wait();
    task2_done.get_future().wait();
    std::cout << "task1 result: " << task1.result() << std::endl;
    std::cout << "task2 result: " << task2.result() << std::endl;

    return 0;
}
