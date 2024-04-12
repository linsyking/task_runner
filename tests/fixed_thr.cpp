
#include <cmath>
#include <iostream>
#include "runner.hpp"
#include "task.hpp"

class fun_task : public task {
private:
    void (*func)() = nullptr;

public:
    fun_task(void (*func)()) : func(func) {}
    fun_task(void (*func)(), size_t c) : func(func) { run_on = c; }
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
    for (int i = 0; i < 100000000; i++) {
        c[(i % 999) + 1] = std::sqrt(i * 0.123) * c[(i % 333)] / 3444;
        a                = std::sqrt(a + c[(i % 999) + 1]);
    }
    std::cout << a << "\n";
}

void fun2() {
    double a = 0;
    std::cout << a << "\n";
}

void test_manyfunc_in_th0() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t2 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t3 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t4 = std::make_shared<fun_task>(fun1, 0);
    runner::add_task(t1);
    runner::add_task(t2);
    runner::add_task(t3);
    runner::add_task(t4);
    runner::commit();
    runner::wait();
    runner::quit();
}

void test_manyfunc_in_th0_dep() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t2 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t3 = std::make_shared<fun_task>(fun2, 0);
    task_ptr t4 = std::make_shared<fun_task>(fun2, 0);
    runner::add_order(t1, t3);
    runner::add_order(t3, t2);
    runner::add_order(t2, t4);
    runner::commit();
    runner::wait();
    runner::quit();
}

void test_manyfunc_in_th01_dep() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t2 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t3 = std::make_shared<fun_task>(fun2, 1);
    task_ptr t4 = std::make_shared<fun_task>(fun2, 1);
    runner::add_order(t1, t3);
    runner::add_order(t3, t4);
    runner::add_order(t2, t3);
    runner::commit();
    runner::wait();
    runner::quit();
}

void test_manyfunc_in_th0a_dep() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t2 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t3 = std::make_shared<fun_task>(fun2);
    task_ptr t4 = std::make_shared<fun_task>(fun2);
    runner::add_order(t1, t3);
    runner::add_order(t3, t4);
    runner::add_order(t2, t3);
    runner::commit();
    runner::wait();
    runner::quit();
}

void test_manyfunc_in_block() {
    runner::boot();
    task_ptr t1 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t2 = std::make_shared<fun_task>(fun1, 0);
    task_ptr t3 = std::make_shared<fun_task>(fun2, 1);
    task_ptr t4 = std::make_shared<fun_task>(fun2, 1);
    task_ptr t5 = std::make_shared<fun_task>(fun1, 1);
    runner::add_order(t1, t3);
    runner::add_order(t3, t4);
    runner::add_order(t2, t3);
    runner::add_task(t5);
    runner::commit();
    runner::wait();
    runner::quit();
}

int main() {
    test_manyfunc_in_th0();
    std::cout << "====================\n";
    test_manyfunc_in_th0_dep();
    std::cout << "====================\n";
    test_manyfunc_in_th01_dep();
    std::cout << "====================\n";
    test_manyfunc_in_th0a_dep();
    std::cout << "====================\n";
    test_manyfunc_in_block();
    return 0;
}
