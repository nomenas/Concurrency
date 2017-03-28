#include <iostream>
#include <future>

#include "TaskExecutor.h"
#include "Task.h"

class SimulationTask : public Task {
public:
    ~SimulationTask() {
        if (_thread) {
            _thread->join();
        }
    }

    void execute(int wait_for_ms, std::function<void()> function) {
        _thread.reset(new std::thread([this, wait_for_ms, function]() {
            {
                std::unique_lock<std::mutex> lock{_mutex};
                _condition.wait_for(lock, std::chrono::milliseconds(wait_for_ms), [this]()-> bool{return _condition_done;});
            }

            function();
        }));
    }

    void stop() override {
        std::lock_guard<std::mutex> lock{_mutex};
        _condition_done = true;
        _condition.notify_all();
    }

protected:
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _condition_done = false;
    std::unique_ptr<std::thread> _thread;
};

class CheckIPTask : public SimulationTask {
public:
    CheckIPTask(const std::string& ip) : _ip(ip) {}

    void execute() override {
        SimulationTask::execute(500, [this](){
            std::cout << "|" << std::endl;
            _is_bridge = _ip == "192.168.0.1" || _ip == "192.168.0.12";
            done();
        });
    }

    void stop() override {
        SimulationTask::stop();
    }

    const std::string& ip() const {
        return _ip;
    }

    bool is_bridge() const {
        return _is_bridge;
    }

private:
    std::string _ip;
    bool _is_bridge = false;
};

class UPNPSearchTask : public SimulationTask {
public:
    void execute() override {
        SimulationTask::execute(2000, [this](){
            _possible_ip_address = {"192.168.0.1", "192.168.0.11", "192.168.0.12", "192.168.0.31", "192.168.0.18"};
            done();
        });
    }

    void stop() override {
        SimulationTask::stop();
    }

    const std::vector<std::string>& possible_ip_addresses() const {
        return _possible_ip_address;
    }

private:
    std::vector<std::string> _possible_ip_address;
};

class DiscoverBridges : public TaskExecutor {
public:
    using Callback = std::function<void(const std::vector<std::string>&)>;

    ~DiscoverBridges() {
        stop();
    }

    void execute(Callback callback) {
        create_task<UPNPSearchTask>().execute([this, callback](Task* task) {
            auto search_task = static_cast<UPNPSearchTask*>(task);
            _num_of_check_ip_requests = search_task->possible_ip_addresses().size();

            for (const auto& ip : search_task->possible_ip_addresses()) {
                create_task<CheckIPTask>(ip).execute([this, callback](Task* task) {
                    auto check_ip_task = static_cast<CheckIPTask*>(task);
                    if (check_ip_task->is_bridge()) {
                        _avalible_bridges.push_back(check_ip_task->ip());
                    }

                    if (++_num_of_received_responses == _num_of_check_ip_requests) {
                        callback(_avalible_bridges);
                    }
                });
            }
        });
    }

private:
    int _num_of_received_responses = 0;
    int _num_of_check_ip_requests = 0;
    std::vector<std::string> _avalible_bridges;
};

int main() {
    std::cout << "==== Test canceling ====" << std::endl;
    auto start = std::chrono::steady_clock::now();
    {
        DiscoverBridges bridge_discovery;
        bridge_discovery.execute([](const std::vector<std::string>& bridges){});
    }
    std::cout << "canceled for " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start).count() << " microseconds\n" << std::endl;

    std::cout << "==== Test normal execution ====" << std::endl;
    std::promise<void> wait_results;
    DiscoverBridges bridge_discovery;
    bridge_discovery.execute([&wait_results](const std::vector<std::string>& bridges){
        std::cout << "Bridge discovery finished. The following bridges were found:" << std::endl;
        for (const auto& bridge : bridges) {
            std::cout << " " << bridge << std::endl;
        }

        wait_results.set_value();
    });

    wait_results.get_future().wait();
    return 0;
}