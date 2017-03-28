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

protected:
    void done() {
        if (_callback) {
            _callback(this);
        }
    }

private:
    Callback _callback;
};

#endif //WITHTASKEXECUTOR_TASK_H
