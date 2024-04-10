#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "task.hpp"

class runner {
private:
    runner();

    std::vector<std::thread> threads;

    bool started = false;

public:
    bool running = false;
    std::mutex mtx;

    /// A thread-safe queue of task chains
    std::queue<task_chain>                              task_chains;
    std::unordered_map<task_ptr, std::vector<task_ptr>> succ;
    std::unordered_set<task_ptr>                        all_tasks;
    std::unordered_set<task_ptr>                        visited;

    std::condition_variable has_task;
    std::condition_variable no_task;

    task_ptr find_next(task_ptr);

    /// Start the runner with a number of threads
    ///
    /// Only run once
    void boot(size_t num_threads = 4);

    /// Add requirement: run a before b
    ///
    /// Call this function after calling run()
    void add_order(task_ptr a, task_ptr b);

    /// Commit and run all the tasks
    void commit();

    /// Wait for all the tasks to finish so that we can commit again
    void wait();

    static runner &get() {
        static runner instance;
        return instance;
    }

    runner(runner const &)         = delete;
    void operator=(runner const &) = delete;
};
