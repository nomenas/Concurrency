//
// Created by Naum Puroski on 22/03/2017.
//

#ifndef TASKCONCEPT_TASK_H
#define TASKCONCEPT_TASK_H

#include "TaskExecutor.h"
#include "Utils.h"

#include <functional>
#include <vector>
#include <numeric>
#include <future>
#include <utility>

template <typename T>
using Holder = std::unique_ptr<T, std::function<void(T*)>>;

class Task {
public:
    template <typename T>
    using Callback = std::function<void(T*)>;

    explicit Task(Callback<Task> callback) : _callback(std::move(callback)) {}

    virtual ~Task() {
        if (_executor) {
            _executor->cancel(this);
        }
    }

    void set_executor(TaskExecutor* executor) {
        _executor = executor;
    }

    // run task and provide callback that would be called when tasks finishes
    Task& run(bool force_synchronous_mode = false) {
        if (_executor && !force_synchronous_mode) {
            _executor->execute(this);
        } else {
            execute();
        }

        return *this;
    }

    // run task and wait for results
    Task& wait() {
        safe_call([this](){_promise.get_future().wait();});
        return *this;
    }

    template <typename T>
    const T& get_results() {
        return *static_cast<T*>(&wait());
    }

    void cancel() {
        bool is_stopped = false;
        do {
            // cancel current sub-tasks
            std::vector<Holder<Task>> pending_tasks;
            {
                std::lock_guard<std::mutex> lock{_sub_tasks_mutex};
                pending_tasks = std::move(_sub_tasks);
            }
            pending_tasks.clear();

            // stop task execution
            if (!is_stopped) {
                stop();
                is_stopped = true;
            }
        } while (!_is_done);
    }

protected:

    virtual void execute() = 0;
    virtual void stop() {};

    void mark_as_done() {
        if (_callback) {
            _callback(this);
        }
        _promise.set_value();
        _is_done = true;
    }

    template<typename T, typename... Args>
    Task& create_task(Args... args) {
        Holder<Task> sub_task{new T{std::forward<Args>(args)...}, [](Task* task){task->cancel(); delete task;}};
        sub_task->set_executor(_executor);
        Task& return_value = *sub_task;
        {
            std::lock_guard<std::mutex> lock{_sub_tasks_mutex};
            _sub_tasks.push_back(std::move(sub_task));
        }
        return return_value;
    }

    template<typename T, typename... Args>
    Task& run_task(Args... args) {
        return create_task<T>(std::forward<Args>(args)...).run();
    }

    template<typename T, typename Container>
    void run_tasks(const Container& container, std::function<void(const std::vector<T*>&)> callback) {
        _handled_results_counter = 0;

        std::vector<T*>* tasks = new std::vector<T*>();
        for (const auto& item : container) {
            tasks->push_back(static_cast<T*>(&run_task<T>(item, [this, container, callback, tasks](Task* task) {
                if (_handled_results_counter.fetch_add(1) == container.size() - 1) {
                    {
                        std::lock_guard<std::mutex> lock{_sub_tasks_mutex};
                        tasks->erase(std::remove_if(tasks->begin(), tasks->end(), [this](T* task) -> bool {
                            return _sub_tasks.end() == std::find_if(_sub_tasks.begin(), _sub_tasks.end(), [task](const Holder<Task>& holder) -> bool {
                                return holder.get() == task;
                            });
                        }), tasks->end());
                    }

                    callback(*tasks);
                    delete tasks;
                }
            })));
        }
    }

private:
    std::atomic<int> _handled_results_counter;
    int _expected_results = 0;

    Callback<Task> _callback;
    bool _is_done = false;
    std::promise<void> _promise;
    std::mutex _sub_tasks_mutex;
    std::vector<Holder<Task>> _sub_tasks;
    TaskExecutor* _executor = nullptr;
};

template <typename T>
Task::Callback<Task> create_task_callback(Task::Callback<T> callback) {
    return [callback](Task* task){if (callback) {callback(static_cast<T*>(task));}};
}

template<typename T, typename... Args>
Holder<T> create_task(Args... args) {
    return Holder<T>{new T{std::forward<Args>(args)...}, [](Task* task){task->cancel(); delete task;}};
}

template<typename T, typename... Args>
Holder<T> run_task(Args... args) {
    auto task = create_task<T>(std::forward<Args>(args)...);
    task->run();
    return task;
}

#endif //TASKCONCEPT_TASK_H
