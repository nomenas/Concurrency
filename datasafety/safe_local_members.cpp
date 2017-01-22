//
// Created by Naum Puroski on 21/01/2017.
//

#include <memory>
#include <thread>
#include <iostream>

static void do_test_locals_using_shared_pointers() {
    auto local_data = std::make_shared<int>(5);

    std::thread{[=](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << *local_data << std::endl;
    }}.detach();
}

void test_locals_using_shared_pointers() {
    do_test_locals_using_shared_pointers();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

static void do_test_locals_using_weak_pointer_checker() {
    auto local_data = 5;
    auto shared_scope = std::make_shared<char>();
    std::weak_ptr<char> weak_scope = shared_scope;

    // dont capture shared_scope
    std::thread{[weak_scope, local_data](){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (weak_scope.lock()) {
            std::cout << local_data << std::endl;
        } else {
            std::cout << "out of scope" << std::endl;
        }
    }}.detach();
}

void test_locals_using_weak_pointer_checker() {
    do_test_locals_using_weak_pointer_checker();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}