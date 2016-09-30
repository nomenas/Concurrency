#include <iostream>
#include <queue>
#include <condition_variable>
#include <thread>

class Queue
{
public:
    void push(int value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_container.push(value);
    }

    int pop(long ms) {
        std::unique_lock<std::mutex> lock(m_mutex);

        if (m_container.empty()) {
            m_conditional.wait_for(lock, std::chrono::milliseconds(ms), [&](){return !m_container.empty();});
        }

        int returnValue = 0;
        if (!m_container.empty()) {
            returnValue = m_container.front();
            m_container.pop();
        }

        return returnValue;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_conditional;
    std::queue<int> m_container;
};

int main() {
    Queue queue;
    const auto epoch = std::chrono::steady_clock::now();
    const auto maxExecutionTime = 100000;

    std::thread thread1([&]() {
        while (1) {
            const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - epoch).count();
            queue.push(elapsedTime);
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            if (elapsedTime > maxExecutionTime) break;
        }
    });
    std::thread thread2([&]() {
        while (1) {
            const auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - epoch).count();

            std::cout << queue.pop(1500) << std::endl;
            if (elapsedTime > maxExecutionTime) break;
        }
    });

    thread1.join();
    thread2.join();

    return 0;
}