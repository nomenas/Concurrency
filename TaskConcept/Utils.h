//
// Created by Naum Puroski on 24/03/2017.
//

#ifndef TASKCONCEPT_UTILS_H
#define TASKCONCEPT_UTILS_H

#include <functional>

template <typename T>
struct Trait {
    static T default_value(){return T{};};
};

template <>
struct Trait<void> {
    static void default_value(){};
};

template <typename T>
auto safe_call(T function) -> decltype(function()){
#ifdef __EXCEPTIONS
    try {
        return function();
    } catch (...) {
        // do nothing
    }
#else
    return function();
#endif  // __EXCEPTIONS

    return Trait<decltype(function())>::default_value();
}

#endif //TASKCONCEPT_UTILS_H
