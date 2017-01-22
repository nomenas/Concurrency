//
// Created by Naum Puroski on 20/01/2017.
//

#include <thread>
#include <iostream>

namespace safe {

    template <typename T>
    class InterfaceHolder;

    template <typename T>
    class Interface : public T {
        friend class InterfaceHolder<T>;

    public:
        Interface(T* item) : _item{item}, _shared_this{this, [](Interface<T>*){}} {}

        ~Interface() {
            std::unique_lock<std::recursive_mutex> lock{*_mutex};
            _shared_this.reset();
        }


        // custom implementation needed
        Interface<T>(Interface<T>&) = delete;
        Interface<T>(Interface<T>&&) = delete;
        Interface<T>& operator=(Interface<T>&) = delete;
        Interface<T>& operator=(Interface<T>&&) = delete;

    private:
        T* _item;
        std::shared_ptr<Interface<T>> _shared_this;
        std::shared_ptr<std::recursive_mutex> _mutex{new std::recursive_mutex};
    };

    template <typename T>
    class InterfaceHolder {
        T* _unsafe_interface = nullptr;
        Interface<T>* _safe_interface = nullptr;
        std::weak_ptr<Interface<T>> _weak_this;
        std::shared_ptr<std::recursive_mutex> _mutex;

    public:
        InterfaceHolder(T* interface) : _unsafe_interface{interface}, _mutex{new std::recursive_mutex} {}

        InterfaceHolder(Interface<T>* interface) : _safe_interface{interface} {
            if (_safe_interface) {
                _weak_this = _safe_interface->_shared_this;
                _mutex = _safe_interface->_mutex;
            }
        }

        T* interface() const {
            std::unique_lock<std::recursive_mutex> lock{*_mutex};
            return _unsafe_interface ? _unsafe_interface : (_weak_this.lock().get() ? _safe_interface->_item : nullptr);
        }

        template <typename F>
        auto call(F f) const -> decltype(std::bind(f, std::placeholders::_1)(_unsafe_interface)) {
            std::unique_lock<std::recursive_mutex> lock{*_mutex};
            return interface() ?
                   std::bind(f, interface())() :
                   decltype(std::bind(f, std::placeholders::_1)(_unsafe_interface))();
        };

        template <typename F, typename... Args>
        auto call(F f, Args&&... args) -> decltype(std::bind(f, std::placeholders::_1, std::forward<Args>(args)...)(_unsafe_interface)) {
            std::unique_lock<std::recursive_mutex> lock{*_mutex};
            return interface() ?
                   std::bind(f, interface(), std::forward<Args>(args)...)() :
                   decltype(std::bind(f, std::placeholders::_1, std::forward<Args>(args)...)(_unsafe_interface))();
        };
    };

    template<typename T>
    InterfaceHolder<T> create_interface_holder(T* interface) {
        auto safe_interface = dynamic_cast<Interface<T>*>(interface);
        return safe_interface ? InterfaceHolder<T>{safe_interface} : InterfaceHolder<T>{interface};
    }
}

// Client

class ConcreteInterface {
public:
    virtual ~ConcreteInterface() = default;
    virtual void foo() = 0;
};

class ConcreteClass : public safe::Interface<ConcreteInterface> {
public:
    ConcreteClass() : safe::Interface<ConcreteInterface>{this} {}

    void foo() override {
        std::cout << "* ConcreteClass::foo" << std::endl;
    }
};

// Worker

class Worker {
public:
    void do_something(ConcreteInterface* interface) {
        // you can keep this object in some container
        auto interface_holder = safe::create_interface_holder(interface);

        std::cout << "test interface_holder call method" << std::endl;
        interface_holder.call(&ConcreteInterface::foo);

        // everywhere in backend code developer works with holder
        // it caries information if callback object is still valid or not
        std::thread{[=]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << "task finished, try to call callback if it is still available (in this case nothing is expected happen)" << std::endl;
            interface_holder.call(&ConcreteInterface::foo);
        }}.detach();
    }
};

// Test

void test_interface_callback_safety() {
    std::unique_ptr<ConcreteInterface> interface{new ConcreteClass{}};

    Worker worker;
    worker.do_something(interface.get());
    std::cout << "removing interface prior receiving callback" << std::endl;
    interface.reset();

    std::this_thread::sleep_for(std::chrono::seconds(2));
}