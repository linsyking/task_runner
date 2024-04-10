#include "runner.hpp"
#include <iostream>
#include <queue>
#include "task.hpp"

void task_single_runner() {
    runner                      &r = runner::get();
    std::unique_lock<std::mutex> lock(r.mtx);
    while (1) {
        while (r.task_chains.empty()) {
            r.has_task.wait(lock);
            if (r.terminated) {
                std::cout << "Runner exit\n";
                return;
            }
        }
        std::cout << "Runner get task\n";
        // Get a task chain
        task_chain chain = r.task_chains.front();
        r.task_chains.pop();
        lock.unlock();
        while (!chain.empty()) {
            task_ptr task = chain.front();
            chain.pop();
            task->run();

            lock.lock();
            r.done_tasks.insert(task);
            if (r.succ.find(task) != r.succ.end()) {
                for (auto &t : r.succ[task]) {
                    if (r.visited.find(t) != r.visited.end()) {
                        continue;
                    }
                    std::unique_lock<std::mutex> _(t->mtx);
                    t->pred_num--;
                    if (t->pred_num == 0) {
                        // Start point t
                        // lock.lock();
                        std::queue<task_ptr> q;
                        task_ptr             tt = t;
                        while (tt) {
                            q.push(tt);
                            tt = r.find_next(tt);
                        }
                        r.task_chains.push(q);
                        r.has_task.notify_one();
                        // lock.unlock();
                    }
                }
            }
            lock.unlock();
        }
        lock.lock();
        if (r.task_chains.empty()) {
            r.done.notify_all();
        }
    }
}

void runner::boot(size_t num_threads) {
    runner &r = runner::get();
    if (r.started) {
        std::cerr << "runner::boot() can only be called once\n";
        return;
    }
    r.started = true;
    r.threads.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        r.threads.emplace_back(task_single_runner);
    }
}
void runner::add_task(task_ptr a) {
    runner &r = runner::get();
    if (!r.started) {
        std::cerr << "runner::add_order() can only be called after boot()\n";
        return;
    }
    if (r.running) {
        std::cerr << "runner::add_order() can only be called while not running\n";
        return;
    }
    r.all_tasks.insert(a);
}

void runner::add_order(task_ptr a, task_ptr b) {
    runner &r = runner::get();
    r.add_task(a);
    r.add_task(b);
    b->pred_num++;
    r.succ[a].push_back(b);
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
    runner &r = runner::get();
    if (!r.started) {
        std::cerr << "runner::commit() can only be called after boot()\n";
        return;
    }
    if (r.running) {
        std::cerr << "runner::commit() can only be called while not running\n";
        return;
    }
    if (r.all_tasks.empty()) {
        return;
    }
    // std::cout << "Task number: " << r.all_tasks.size() << "\n";
    for (auto &task : r.all_tasks) {
        if (task->pred_num == 0) {
            // Start point
            // std::cout << "Gen queue: \n";
            std::queue<task_ptr> q;
            task_ptr             t = task;
            while (t) {
                // std::cout << t << " ";
                q.push(t);
                t = r.find_next(t);
            }
            // std::cout << "\n";
            r.task_chains.push(q);
        }
    }
    r.running = true;
    r.has_task.notify_all();
}

void runner::wait() {
    runner &r = runner::get();
    if (!r.started || !r.running) {
        std::cerr << "runner::wait() can only be called after commit()\n";
        return;
    }
    std::unique_lock<std::mutex> lock(r.mtx);
    while (r.done_tasks.size() < r.all_tasks.size()) {
        r.done.wait(lock);
    }
    // No more tasks, cleanup
    r.succ.clear();
    r.all_tasks.clear();
    r.running = false;
}

void runner::join() {
    runner &r    = runner::get();
    r.terminated = true;
    r.has_task.notify_all();
    for (auto &t : r.threads) {
        t.join();
    }
}
