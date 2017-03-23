#include <iostream>

#include "ConcreteTasks.h"

int main() {
    std::cout << "result: " << CompoundTask(10).run().wait().get_results<CompoundTask>().sum() << std::endl;

    CompoundTask task(20);
    task.run().wait();
    std::cout << "result: " << task.sum() << std::endl;

    std::promise<void> promise;
    CompoundTask taskWaitCallback(30, [&promise](Task*){promise.set_value();});
    taskWaitCallback.run();
    promise.get_future().wait();
    std::cout << "result: " << taskWaitCallback.sum() << std::endl;

    return 0;
}