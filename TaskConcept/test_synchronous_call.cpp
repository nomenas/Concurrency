#include <iostream>

#include "ConcreteTasks.h"

int main() {
    std::cout << "result: " << CompoundTask(10).run().wait<CompoundTask>().result() << std::endl;

    CompoundTask task(20);
    task.run().wait();
    std::cout << "result: " << task.result() << std::endl;

    std::promise<void> promise;
    CompoundTask taskWaitCallback(30, [&promise](Task*){promise.set_value();});
    taskWaitCallback.run();
    promise.get_future().wait();
    std::cout << "result: " << taskWaitCallback.result() << std::endl;

    return 0;
}