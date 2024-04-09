#pragma once

#include <memory>
#include <mutex>
#include <queue>

class task;
using task_ptr   = std::shared_ptr<task>;
using task_chain = std::queue<task_ptr>;

class task {
public:
    virtual void run() = 0;
    virtual ~task()    = default;

    /// Runtime predessor number
    size_t pred_num = 0;

    /// Mutex for pred_num
    std::mutex mtx;
};
