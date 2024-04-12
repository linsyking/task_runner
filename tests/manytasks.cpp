
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
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
    for (int i = 0; i < 100000000; i++) {
        c[(i % 999) + 1] = std::sqrt(i * 0.123) * c[(i % 333)] / 3444;
        a                = std::sqrt(a + c[(i % 999) + 1]);
    }
    std::cout << a << "\n";
}

void fun3() {
    double a = 0;
    double b = 1.1234;
    double c[1000];
    for (int i = 0; i < 1000; i++) {
        b += 0.333;
        c[i] = b;
    }
    for (int i = 0; i < 1000000; i++) {
        c[(i % 999) + 1] = std::sqrt(i * 0.123) * c[(i % 333)] / 3444;
        a                = std::sqrt(a + c[(i % 999) + 1]);
    }
    std::cout << a << "\n";
}

void fun2() {
    double a = 0;
    std::cout << a << "\n";
}

void fun4() {
    // Sleep for a while
    std::cout << "Sleeping..\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Infinite task done\n";
}

int main() {
    runner::boot();
    auto inf = std::make_shared<fun_task>(fun4);

    std::vector<std::shared_ptr<fun_task>> tasks;
    for (int i = 0; i < 6; ++i) {
        auto t = std::make_shared<fun_task>(fun1);
        tasks.emplace_back(t);
        runner::add_task(t);
    }
    for (int i = 0; i < 10; ++i) {
        auto t = std::make_shared<fun_task>(fun3);
        tasks.emplace_back(t);
        runner::add_task(t);
        runner::add_order(tasks[0], t);
    }
    runner::add_order(inf, tasks[14]);
    runner::commit();
    std::cout << "Commit done, waiting...\n";
    runner::wait();
    std::cout << "Wait done, quiting...\n";
    runner::quit();
    std::cout << "Quited successfully\n";
    return 0;
}
