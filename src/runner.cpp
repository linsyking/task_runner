#include "runner.hpp"
#include <queue>
#include "task.hpp"

void task_single_runner() {
    runner                      &r = runner::get();
    std::unique_lock<std::mutex> lock(r.mtx);
    while (1) {
        while (r.task_chains.empty()) {
            r.has_task.wait(lock);
            if (r.terminated) {
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
            r.done.notify_all();
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
    if (r.terminated || !r.started || r.running) {
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
    if (r.terminated) {
        return;
    }
    r.add_task(a);
    r.add_task(b);
    if (r.running || !r.started) {
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

task_ex_ptr runner::find_next(task_ex_ptr task) {
    task->visited = true;
    for (auto &t : task->succ) {
        if (!t->visited) {
            // Found t
            return t;
        }
    }
    return nullptr;
}

void runner::commit() {
    runner &r = runner::get();
    if (r.terminated || !r.started || r.running || r.all_tasks.empty()) {
        return;
    }
    // std::cout << "Task number: " << r.all_tasks.size() << "\n";
    for (auto &task : r.all_tasks) {
        if (task->pred_num == 0) {
            // Start point
            // std::cout << "Gen queue: \n";
            std::queue<task_ex_ptr> q;
            task_ex_ptr             t = task;
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
    if (r.terminated || !r.started || !r.running) {
        return;
    }
    std::unique_lock<std::mutex> lock(r.mtx);
    while (r.done_tasks < r.all_tasks.size()) {
        r.done.wait(lock);
    }
    // No more tasks, cleanup
    r.task_map.clear();
    r.all_tasks.clear();
    r.done_tasks = 0;
    r.running    = false;
}

void runner::quit() {
    runner &r = runner::get();
    if (r.terminated || !r.started) {
        return;
    }
    if (r.running) {
        wait();
    }
    r.terminated = true;
    r.has_task.notify_all();
    for (auto &t : r.threads) {
        t.join();
    }
}
