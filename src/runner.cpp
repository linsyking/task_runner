#include "runner.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <mutex>
#include "task.hpp"

size_t runner::thread_num() {
    std::thread::id id = std::this_thread::get_id();
    for (size_t i = 0; i < threads.size(); ++i) {
        if (threads[i].get_id() == id) {
            return i;
        }
    }
    std::cerr << "Thread not found\n";
    assert(false);
}

void task_single_runner() {
    runner                      &r         = runner::get();
    size_t                       thread_id = r.thread_num();
    std::unique_lock<std::mutex> lock(r.runtime_mtx);
    while (r.started) {
        while (r.task_chains.empty()) {
            r.has_task.wait(lock);
            if (!r.started) {
                return;
            }
        }
        // Get a task chain
        task_chain chain = r.task_chains.front();
        r.task_chains.pop();
        lock.unlock();
        while (!chain.empty()) {
            task_ex_ptr task = chain.front();
            chain.pop();
            task->self->run();

            lock.lock();
            r.done_tasks++;
            for (auto &t : task->succ) {
                if (t->visited) {
                    continue;
                }
                std::unique_lock<std::mutex> _(t->mtx);
                t->pred_num--;
                if (t->pred_num == 0) {
                    // Start point t
                    // lock.lock();
                    std::queue<task_ex_ptr> q;
                    task_ex_ptr             tt = t;
                    while (tt) {
                        q.push(tt);
                        tt = r.find_next(tt);
                    }
                    r.task_chains.push(q);
                    r.has_task.notify_one();
                    // lock.unlock();
                }
            }

            lock.unlock();
        }
        lock.lock();
        if (r.task_chains.empty()) {
            r.all_done.notify_all();
        }
    }
}

void runner::boot(size_t num_threads) {
    runner &r = runner::get();
    if (r.started) {
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
        return;
    }
    if (r.task_map.find(a) == r.task_map.end()) {
        r.task_map[a]       = std::make_shared<task_ex>();
        r.task_map[a]->self = a;
        r.all_tasks.insert(r.task_map[a]);
    }
}

void runner::add_order(task_ptr a, task_ptr b) {
    runner &r = runner::get();
    r.add_task(a);
    r.add_task(b);
    if (!r.started) {
        return;
    }
    for (auto &t : r.task_map[a]->succ) {
        if (t == r.task_map[b]) {
            return;
        }
    }
    r.task_map[b]->pred_num++;
    r.task_map[a]->succ.push_back(r.task_map[b]);
}

void runner::commit() {
    runner &r = runner::get();
    if (!r.started || r.all_tasks.empty()) {
        return;
    }
    task_runtime rt;
    // std::cout << "Task number: " << r.all_tasks.size() << "\n";
    for (auto &task : r.all_tasks) {
        if (task->self->run_on.has_value()) {
            rt.all_named_tasks[task->self->run_on.value()].push_back(task);
        } else {
            rt.all_unnamed_tasks.push_back(task);
        }
    }
    std::sort(rt.all_unnamed_tasks.begin(), rt.all_unnamed_tasks.end(),
              [](task_ex_ptr &a, task_ex_ptr &b) { return a->pred_num < b->pred_num; });
    for (size_t i = 0; i < rt.all_named_tasks.size(); ++i) {
        std::sort(rt.all_named_tasks[i].begin(), rt.all_named_tasks[i].end(),
                  [](task_ex_ptr &a, task_ex_ptr &b) { return a->pred_num < b->pred_num; });
    }
    // std::unique_lock<std::mutex> lock(r.runtime_mtx);
    r.to_run.enqueue(rt);
    r.running = true;
    r.has_task.notify_all();
}

bool runner::is_all_done(){
    runner &r = runner::get();
    return r.runtime.task_remaining == 0 && r.to_run.empty();
}

void runner::wait() {
    runner &r = runner::get();
    if (!r.started || !r.running) {
        return;
    }
    std::unique_lock<std::mutex> lock(r.runtime_mtx);
    while (!r.is_all_done()) {
        r.all_done.wait(lock);
    }
    // No more tasks, cleanup
    r.task_map.clear();
    r.all_tasks.clear();
    r.running    = false;
}

void runner::quit() {
    runner &r = runner::get();
    if (!r.started) {
        return;
    }
    if (r.running) {
        wait();
    }
    r.started = false;
    r.has_task.notify_all();
    for (auto &t : r.threads) {
        t.join();
    }
    r.threads.clear();
}
