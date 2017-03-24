//
// Created by Naum Puroski on 24/03/2017.
//

#ifndef TASKCONCEPT_UTILS_H
#define TASKCONCEPT_UTILS_H

#include <functional>

void safe_call(std::function<void()> function) {
#ifdef __EXCEPTIONS
    try {
        function();
    } catch (...) {
        // do nothing
    }
#else
    function();
#endif  // __EXCEPTIONS
}

#endif //TASKCONCEPT_UTILS_H
