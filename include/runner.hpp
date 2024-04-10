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
    runner() {}

    std::vector<std::thread> threads;

    bool started = false;

public:
    bool       running    = false;
    bool       terminated = false;
    std::mutex mtx;

    /// A thread-safe queue of task chains
    std::queue<task_chain>                              task_chains;
    std::unordered_map<task_ptr, std::vector<task_ptr>> succ;
    std::unordered_set<task_ptr>                        all_tasks;
    std::unordered_set<task_ptr>                        visited;
    std::unordered_set<task_ptr>                        done_tasks;

    std::condition_variable has_task;
    std::condition_variable done;

    task_ptr find_next(task_ptr);

    /// Start the runner with a number of threads
    ///
    /// Only run once
    static void boot(size_t num_threads = 4);

    /// Add requirement: run a before b
    ///
    /// Call this function after calling run()
    static void add_order(task_ptr a, task_ptr b);

    static void add_task(task_ptr a);

    /// Commit and run all the tasks
    static void commit();

    /// Wait for all the tasks to finish so that we can commit again
    static void wait();

    static void join();

    static runner &get() {
        static runner instance;
        return instance;
    }

    runner(runner const &)         = delete;
    void operator=(runner const &) = delete;
};
