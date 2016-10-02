#include <thread>
#include <iostream>

void lockTwiceInRow() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    try {
        lock.lock();
        std::cout << "lockTwiceInRow passed!" << std::endl;
    } catch (const std::system_error& error) {
        std::cout << "exception: " << error.what() << std::endl;
    }
}

void lockUnlock10000InRow() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    for (int i = 0; i < 10000; ++i) {
        try {
            lock.unlock();
            lock.lock();
        } catch (const std::system_error& error) {
            std::cout << "exception: " << error.what() << std::endl;
            break;
        }

        if (i == 9999) {
            std::cout << "everything is fine!" << std::endl;
        }
    }
}

void unlockFromDifferentThread() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    std::thread thread1 {
        [&lock]() {
            try {
                lock.lock();
            } catch (const std::system_error& error) {
                std::cout << "lock.lock() has thrown " << error.what() << std::endl;
            }

            try {
                std::cout << "is locked owned? " << lock.owns_lock() << std::endl;
                lock.unlock();
                std::cout << "lock is unlocked, try lock again? " << lock.try_lock() << std::endl;
            } catch (std::system_error error) {
                std::cout << "execption: " << error.what() << std::endl;
            }
        }
    };

    thread1.join();
}

void unlockWithOtherLocker_defer_lock_t() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    std::thread thread1 {
        [&mutex]() {
            std::unique_lock<std::mutex> lock{mutex, std::defer_lock_t{}};
// deadlock here
//            try {
//                lock.lock();
//            } catch (const std::system_error& error) {
//                std::cout << "lock.lock() has thrown " << error.what() << std::endl;
//            }

            try {
                std::cout << "is locked owned? " << lock.owns_lock() << std::endl;
                lock.unlock();
                std::cout << "lock is unlocked, try lock again? " << lock.try_lock() << std::endl;
            } catch (std::system_error error) {
                std::cout << "execption: " << error.what() << std::endl;
            }
        }
    };

    thread1.join();
}

void unlockWithOtherLocker_adopt_lock_t() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    std::thread thread1 {
        [&mutex]() {
            std::unique_lock<std::mutex> lock{mutex, std::adopt_lock_t{}};

            try {
                lock.lock();
            } catch (const std::system_error& error) {
                std::cout << "lock.lock() has thrown " << error.what() << std::endl;
            }

            try {
                std::cout << "is locked owned? " << lock.owns_lock() << std::endl;
                lock.unlock();
                std::cout << "lock is unlocked, try lock again? " << lock.try_lock() << std::endl;
            } catch (std::system_error error) {
                std::cout << "execption: " << error.what() << std::endl;
            }
        }
    };

    thread1.join();
}

void unlockWithOtherLocker_try_to_lock_t() {
    std::mutex mutex;
    std::unique_lock<std::mutex> lock{mutex};

    std::thread thread1 {
        [&mutex]() {
            std::unique_lock<std::mutex> lock{mutex, std::try_to_lock_t{}};

// deadlock!
//                try {
//                    lock.lock();
//                } catch (const std::system_error& error) {
//                    std::cout << "lock.lock() has thrown " << error.what() << std::endl;
//                }

            try {
                std::cout << "is locked owned? " << lock.owns_lock() << std::endl;
                lock.unlock();
                std::cout << "lock is unlocked, try lock again? " << lock.try_lock() << std::endl;
            } catch (std::system_error error) {
                std::cout << "execption: " << error.what() << std::endl;
            }
        }
    };

    thread1.join();
}

int main() {
    std::cout << "* Lock Twice In Row Test" << std::endl;
    lockTwiceInRow();
    std::cout << "* Lock Unlock 10000 in row" << std::endl;
    lockUnlock10000InRow();
    std::cout << "* Unlock from different thread test" << std::endl;
    unlockFromDifferentThread();
    std::cout << "* Unlock with other locker which does not acquire ownership of the mutex" << std::endl;
    unlockWithOtherLocker_defer_lock_t();
    std::cout << "* Unlock with other locker which assumes the calling thread already has ownership of the mutex" << std::endl;
    unlockWithOtherLocker_adopt_lock_t();
    std::cout << "* Unlock with other locker which tryies to acquire ownership of the mutex without blocking" << std::endl;
    unlockWithOtherLocker_try_to_lock_t();

    return 0;
}