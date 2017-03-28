//
// Created by Naum Puroski on 24/03/2017.
//

#include <iostream>
#include "ConcreteTasks.h"


int main () {

    {
        auto start = std::chrono::steady_clock::now();
        {
            create_task<CompoundTask>(10)->run();
        }
        std::cout << "canceled for " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() << " microseconds" << std::endl;
    }

    {
        auto start = std::chrono::steady_clock::now();
        {
            create_task<BridgeDiscovery>(10)->run();
        }
        std::cout << "canceled for " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() << " microseconds" << std::endl;
    }

    return 0;
}