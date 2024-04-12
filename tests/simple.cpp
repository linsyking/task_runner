#include <iostream>
#include "runner.hpp"
#include "task.hpp"

class fun_task : public task {
private:
    void (*func)() = nullptr;

public:
    fun_task(void (*func)()) : func(func) {}
    void run() override { func(); }
};

void f() {
    double a = 0;
    std::cout << a << "\n";
}

int main() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(f);
    runner::add_task(t1);
    runner::commit();
    std::cout << "Commit done, waiting...\n";
    runner::wait();
    std::cout << "Wait done, quiting\n";
    runner::quit();
    std::cout << "Quited successfully\n";
    return 0;
}
