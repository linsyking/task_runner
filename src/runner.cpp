#include "runner.hpp"
#include <iostream>

void task_single_runner() {}

void runner::boot(size_t num_threads) {
    if (started) {
        std::cerr << "runner::boot() can only be called once\n";
        return;
    }
    started = true;
    threads.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(task_single_runner);
    }
}

void runner::add_order(task_ptr a, task_ptr b) {
    if (!started) {
        std::cerr << "runner::add_order() can only be called after boot()\n";
        return;
    }
    if (running) {
        std::cerr << "runner::add_order() can only be called while not running\n";
        return;
    }
}

void runner::commit() {
    if (!started) {
        std::cerr << "runner::commit() can only be called after boot()\n";
        return;
    }
    if (running) {
        std::cerr << "runner::commit() can only be called while not running\n";
        return;
    }
    std::unique_lock<std::mutex> lock(mtx);
    running = true;
}

void runner::wait() {
    if (!started || !running) {
        std::cerr << "runner::wait() can only be called after commit()\n";
        return;
    }
    std::unique_lock<std::mutex> lock(mtx);
    while (!task_chains.empty()) {
        no_task.wait(lock);
    }
    // No more tasks
    running = false;
}
