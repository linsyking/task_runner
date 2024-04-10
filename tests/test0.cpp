
#include <cmath>
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

void fun1() {
    // Time cost calculation
    double a = 0;
    double b = 1.1234;
    double c[1000];
    for (int i = 0; i < 1000; i++) {
        b += 0.333;
        c[i] = b;
    }
    for (int i = 0; i < 300000000; i++) {
        c[(i % 999) + 1] = std::sqrt(i * 0.123) * c[(i % 333)] / 10230234;
        a                = std::sqrt(a + c[(i % 999) + 1]);
    }
    std::cout << a << "\n";
}

void fun2() {
    double a = 0;
    std::cout << a << "\n";
}

int main() {
    runner::boot();
    task_ptr a = std::make_shared<fun_task>(fun1);
    task_ptr b = std::make_shared<fun_task>(fun1);
    runner::add_task(a);
    runner::add_task(b);
    runner::add_order(a, b);
    runner::commit();
    std::cout << "Commit done, waiting...\n";
    runner::wait();
    runner::join();
    return 0;
}
