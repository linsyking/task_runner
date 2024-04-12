#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "task.hpp"

struct task_runtime {
    /// Tasks that are specified to run on a specific thread
    std::vector<std::vector<task_ex_ptr>> all_named_tasks;

    /// Tasks that are not specified to run on a specific thread
    std::vector<task_ex_ptr> all_unnamed_tasks;

    size_t task_remaining = 0;
};

class runner {
private:
    runner() {}

    std::vector<std::thread> threads;
    static bool              is_all_done();

public:
    bool started = false;
    bool running = false;

    std::mutex                  runtime_mtx;
    std::optional<task_runtime> runtime = std::nullopt;
    std::queue<task_runtime>    to_run;

    size_t thread_num();

    // These structures can only be accessed by the runner thread, so no need to lock
    std::unordered_map<task_ptr, task_ex_ptr> task_map;
    std::unordered_set<task_ex_ptr>           all_tasks;

    std::condition_variable has_task;
    std::condition_variable all_done;

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
    ///
    /// You may call this function while running
    static void commit();

    /// Wait for all the tasks to finish
    static void wait();

    static void quit();

    static runner &get() {
        static runner instance;
        return instance;
    }

    runner(runner const &)         = delete;
    void operator=(runner const &) = delete;
};
