//
// Created by Naum Puroski on 27/03/2017.
//

#ifndef WITHTASKEXECUTOR_TASK_H
#define WITHTASKEXECUTOR_TASK_H

class Task {
    friend class CompoundTask;

public:
    using Callback = std::function<void(Task*)>;

    virtual ~Task() = default;

    void set_completion_callback(Callback callback) {
        _callback = callback;
    }

protected:
    virtual void execute() = 0;
    virtual void stop() = 0;

    void done() {
        if (!_is_done.exchange(true)) {
            if (_callback) {
                _callback(this);
            }
        }
    }

private:
    std::atomic<bool> _is_done{false};
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASK_H
