#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include "task.hpp"

class runner {
private:
    // A thread-safe queue of task chains
    std::queue<task_chain>   task_chains;
    std::mutex               mtx;
    std::condition_variable  has_task;
    std::vector<std::thread> threads;

    void do_tasks();

public:
    static bool running;
    /// Start the runner with a number of threads
    ///
    /// Only run once
    static void start(uint num_threads = 4);

    /// Add requirement: run a before b
    ///
    /// Call this function after calling run()
    static void add_order(task_ptr a, task_ptr b);

    /// Run all the tasks
    static void run();

    /// Wait for all the tasks to finish and then stop the runners
    static void wait_and_stop();
};
