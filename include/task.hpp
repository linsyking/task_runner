#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <queue>

class task;
using task_ptr = std::shared_ptr<task>;

class task {
public:
    /// If not null, run this task on a specific thread
    std::optional<size_t> run_on = std::nullopt;
    virtual void          run()  = 0;
    virtual ~task(){};
};

struct task_ex;
using task_ex_ptr = std::shared_ptr<task_ex>;
using task_chain  = std::queue<task_ex_ptr>;

struct task_ex {
    /// Successors
    std::vector<task_ex_ptr> succ;

    /// Runtime predessor number
    size_t pred_num = 0;

    /// Mutex for pred_num
    std::mutex mtx;

    task_ptr self;
};
