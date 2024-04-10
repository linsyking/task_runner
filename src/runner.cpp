#include "runner.hpp"
#include <iostream>
#include <queue>
#include "task.hpp"

void task_single_runner() {
    runner                      &r = runner::get();
    std::unique_lock<std::mutex> lock(r.mtx);
    while (1) {
        while (r.task_chains.empty() && r.running) {
            r.has_task.wait(lock);
        }
        if (!r.running) {
            break;
        }
        // Get a task chain
        task_chain chain = r.task_chains.front();
        r.task_chains.pop();
        lock.unlock();
        while (!chain.empty()) {
            task_ptr task = chain.front();
            chain.pop();
            task->run();
            for (auto &t : r.succ[task]) {
                std::unique_lock<std::mutex> _(t->mtx);
                t->pred_num--;
                if (t->pred_num == 0) {
                    // Start point t
                    lock.lock();
                    std::queue<task_ptr> q;
                    task_ptr             tt = t;
                    while (tt) {
                        q.push(tt);
                        tt = r.find_next(tt);
                    }
                    r.task_chains.push(q);
                    lock.unlock();
                }
            }
        }
        lock.lock();
        if (r.task_chains.empty()) {
            r.no_task.notify_all();
        }
    }
    std::cout << "Runner exit\n";
}

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
    b->pred_num++;
    succ[a].push_back(b);
    all_tasks.insert(a);
    all_tasks.insert(b);
}

task_ptr runner::find_next(task_ptr task) {
    visited.insert(task);
    for (auto &t : succ[task]) {
        if (visited.find(t) == visited.end()) {
            // Found t
            return t;
        }
    }
    return nullptr;
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
    if (all_tasks.empty()) {
        return;
    }
    for (auto &task : all_tasks) {
        if (task->pred_num == 0) {
            // Start point
            std::queue<task_ptr> q;
            task_ptr             t = task;
            while (t) {
                q.push(t);
                t = find_next(t);
            }
            task_chains.push(q);
        }
    }
    running = true;
    has_task.notify_all();
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
    // No more tasks, cleanup
    succ.clear();
    all_tasks.clear();
    running = false;
    has_task.notify_all();
}
