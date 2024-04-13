# Task Runner

A C++ library to run dependent tasks in parallel. Used in my game engine.

## Features

- Tasks may have dependencies
- Add-Commit-Wait workflow
- Add tasks during runtime
- Pin a task to a specific thread

## Examples

First, let's define a `fun_task` derived from `task`:

```cpp
class fun_task : public task {
private:
    void (*func)() = nullptr;

public:
    fun_task(void (*func)()) : func(func) {}
    void run() override { func(); }
};
```

This allows us to create task running a function.

An example (assume `fun1` and `fun2` are two predefined functions):

```cpp
runner::boot();
task_ptr t1 = std::make_shared<fun_task>(fun1);
task_ptr t2 = std::make_shared<fun_task>(fun2);
runner::add_order(t1, t2);
runner::commit();
runner::wait();
runner::quit();
```

This will create two tasks `t1` and `t2`, and set `t2` depends on `t1`.

After calling `commit()`, the runner will executes `t1` then `t2`.

`wait()` will wait for them to finish.

`quit()` will remove all the runner threads.
