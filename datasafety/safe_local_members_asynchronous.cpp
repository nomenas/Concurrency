//
// Created by Naum Puroski on 21/01/2017.
//
#include <memory>
#include <iostream>
#include <thread>

class Foo {
public:
    Foo() : _shared_this{this, [](Foo*){}}{}

    ~Foo() {
        std::unique_lock<std::recursive_mutex> lock{*_mutex};
        _shared_this.reset();
    }

    void do_something() {
        std::weak_ptr<Foo> weak_this = _shared_this;
        auto mutex = _mutex;
        std::thread {[this, weak_this, mutex]() {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::unique_lock<std::recursive_mutex> lock{*mutex};
                auto shared_this = weak_this.lock();
                if (shared_this) {
                    shared_this->handle_results();
                } else {
                    std::cout << "out of scope" << std::endl;
                }
            }
        }.detach();
    }

    void handle_results() {
        std::cout << "handle_results" << std::endl;
    }

    // custom implementation needed
    Foo(Foo&) = delete;
    Foo(Foo&&) = delete;
    Foo& operator=(Foo&) = delete;
    Foo& operator=(Foo&&) = delete;

    // pimpl
    std::shared_ptr<Foo> _shared_this;
    std::shared_ptr<std::recursive_mutex> _mutex{new std::recursive_mutex};
};

class FooProxy {
public:
    FooProxy(Foo* foo) {
        if (foo) {
            _weak_subject = foo->_shared_this;
            _mutex = foo->_mutex;
        } else {
            _mutex.reset(new std::recursive_mutex);
        }
    }

    void handle_results() {
        std::unique_lock<std::recursive_mutex> lock{*_mutex};

        auto shared_subject = _weak_subject.lock();
        // data race here, somebody may remove object in meantime !
        if (shared_subject) {
            shared_subject->handle_results();
        } else {
            std::cout << "out of scope" << std::endl;
        }
    }

private:
    std::weak_ptr<Foo> _weak_subject;
    std::shared_ptr<std::recursive_mutex> _mutex;
};

void test_safe_local_members_asynchronous() {
    Foo{}.do_something();
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void test_proxy_thru_object_accessor() {
    std::unique_ptr<FooProxy> proxy;
    {
        Foo foo;
        proxy.reset(new FooProxy(&foo));
    }

    proxy->handle_results();
}