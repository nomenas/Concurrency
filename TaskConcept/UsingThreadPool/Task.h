//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASK_H
#define WITHTASKEXECUTOR_TASK_H

class Task {
public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() = default;

    virtual void execute() = 0;
    virtual void stop() = 0;

    void set_completion_callback(Callback callback) {
        _callback = callback;
    }

    void wait() {
        std::unique_lock<std::mutex> unique_lock{_done_mutex};
        _done_condition.wait(unique_lock, [this](){return _is_done;});
    }

protected:
    void done() {
        if (_callback) {
            _callback(this);
        }

        std::lock_guard<std::mutex> lock_guard{_done_mutex};
        _is_done = true;
        _done_condition.notify_all();
    }

private:
    Callback _callback;

    bool _is_done = false;
    std::mutex _done_mutex;
    std::condition_variable _done_condition;
};

#endif //WITHTASKEXECUTOR_TASK_H
