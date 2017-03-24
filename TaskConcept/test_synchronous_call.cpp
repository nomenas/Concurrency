#include <iostream>

#include "ConcreteTasks.h"

int main() {
    // inline call
    {
        std::cout << "result: " << create_task<CompoundTask>(10)->run().get_results<CompoundTask>().sum() << std::endl;
    }

    // wait to finish
    {
        auto task = create_task<CompoundTask>(20);
        task->run().wait();
        std::cout << "result: " << task->sum() << std::endl;
    }

    // using callback as notification when task has been executed
    {
        std::promise<void> promise;
        auto task = create_task<CompoundTask>(30, [&promise](Task*){promise.set_value();});
        task->run();
        promise.get_future().wait();
        std::cout << "result: " << task->sum() << std::endl;
    }

    return 0;
}