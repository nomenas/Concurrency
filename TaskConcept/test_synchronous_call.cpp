#include <iostream>

#include "ConcreteTasks.h"

int main() {
    std::cout << "result: " << CompoundTask(10).run_and_wait<CompoundTask>().result() << std::endl;
    return 0;
}