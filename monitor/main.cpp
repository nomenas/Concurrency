// based on example from "C++ and Beyond 2012: Herb Sutter - C++ Concurrency" presentation
// http://stackoverflow.com/questions/15435330/monitort-class-implementation-in-c11-and-c03

#include <iostream>
#include <thread>

template <typename T>
class Monitor {
private:
    std::unique_ptr<T> m_obj;
    T& m_ref;
    mutable std::mutex m_mutex;

    template <typename F>
    auto operator()(F f) const -> decltype(f(m_ref)) {
        std::lock_guard<std::mutex> hold{m_mutex};
        return f(m_ref);
    }

public:
    Monitor() : m_obj(new T), m_ref(*m_obj.get()) {}
    Monitor(T& t_) : m_ref(t_) {}

    template <typename F>
    auto call(F f) -> decltype(std::bind(f, std::placeholders::_1)(m_ref)) {
        return (*this)(std::bind(f, std::placeholders::_1));
    };

    template <typename F, typename... Args>
    auto call(F f, Args&&... args) -> decltype(std::bind(f, std::placeholders::_1, std::forward<Args>(args)...)(m_ref)) {
        return (*this)(std::bind(f, std::placeholders::_1, std::forward<Args>(args)...));
    };
};

struct NonCopyable {
    NonCopyable() = default;
    NonCopyable(NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&) = delete;
};
class Foo
{
public:
    void voidMethod(int msWait) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msWait));
    }

    int intMethod(int msWait) const {
        std::this_thread::sleep_for(std::chrono::milliseconds(msWait));
        return msWait;
    }

    char methodWithReference(const NonCopyable& arg, NonCopyable& arg2, int msWait) {
        std::this_thread::sleep_for(std::chrono::milliseconds(msWait));
        return 'a';
    }

    float noArgumentMethod()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        return 4.5;
    }
};

int main() {
    Monitor<Foo> fooMonitor;
    const auto epoch = std::chrono::steady_clock::now();
    const auto maxExecutionTime = 100000;
    const auto elapsedTime = [&epoch](){
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - epoch).count();
    };

    std::thread thread1([&]() {
        while (elapsedTime() < maxExecutionTime) {
            fooMonitor.call(&Foo::voidMethod, 2000);
            std::cout << "thread 1 returned void" << std::endl;
        }
    });
    std::thread thread2([&]() {
        while (elapsedTime() < maxExecutionTime) {
            const auto result = fooMonitor.call(&Foo::intMethod, 1000);
            std::cout << "thread 2 returned " << result << std::endl;
        }
    });
    std::thread thread3([&]() {
        while (elapsedTime() < maxExecutionTime) {
            NonCopyable obj1;
            NonCopyable obj2;
            const auto result = fooMonitor.call(&Foo::methodWithReference, std::cref(obj1), std::ref(obj2), 3000);
            std::cout << "thread 3 returned " << result << std::endl;
        }
    });
    std::thread thread4([&]() {
        while (elapsedTime() < maxExecutionTime) {
            const auto result = fooMonitor.call(&Foo::noArgumentMethod);
            std::cout << "thread 4 returned " << result << std::endl;
        }
    });

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    return 0;
}