#include <iostream>

#include "ConcreteTasks.h"

int main() {
    std::cout << "result: " << CompoundTask(10).get_result<CompoundTask>().result() << std::endl;
    return 0;
}