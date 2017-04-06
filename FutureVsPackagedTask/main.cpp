#include <iostream>

#include <future>
#include <vector>
#include <queue>

void test_destroy_future_async() {
    for (int i = 0; i < 10; ++i) {
        std::async(std::launch::async, [i]() {
            std::cout << std::this_thread::get_id() << " Long text for testing, check if everything is aligned. Test "
                      << i << std::endl;
        });
    }
}

class TaskExecutor {
public:
    TaskExecutor() : _thread(std::bind(&TaskExecutor::event_loop, this)) {
    }

    ~TaskExecutor() {
        {
            std::lock_guard<std::mutex> lock{_tasks_mutex};
            _stop = true;
            _tasks_condition.notify_all();
        }

        _thread.join();
    }

    void add_task(std::packaged_task<void()>&& task) {
        std::lock_guard<std::mutex> lock{_tasks_mutex};
        _queue.push(std::move(task));
        _tasks_condition.notify_one();
    }

private:
    void event_loop() {
        while (!_stop || (_stop && !_queue.empty())) {
            std::unique_lock<std::mutex> lock{_tasks_mutex};
            if (_queue.empty()) {
                _tasks_condition.wait(lock);
            }

            if (!_queue.empty()) {
                auto task = std::move(_queue.front());
                _queue.pop();

                lock.unlock();
                task();
            }
        }
    }

    std::thread _thread;
    std::queue<std::packaged_task<void()>> _queue;
    std::condition_variable _tasks_condition;
    std::mutex _tasks_mutex;
    bool _stop = false;
};

static TaskExecutor _executor;
void test_destroy_future_packaged_task() {
    for (int i = 0; i < 10; ++i) {
        auto task = std::packaged_task<void()>([i]() {
            std::cout << std::this_thread::get_id() << " Long text for testing, check if everything is aligned. Test " << i << std::endl;
        });
        auto task_future = task.get_future();
        _executor.add_task(std::move(task));
    }
}

void test_keep_future_async() {
    std::vector<std::future<void>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            std::cout << std::this_thread::get_id() << " Long text for testing, check if everything is aligned. Test " << i <<std::endl;
        }));
    }
}

int main() {
    std::cout << "===== destroy future async =====" << std::endl;
    test_destroy_future_async();
    std::cout << "===== keep future async =====" << std::endl;
    test_keep_future_async();
    std::cout << "===== destroy future packaged task =====" << std::endl;
    test_destroy_future_packaged_task();

    return 0;
}