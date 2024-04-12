
#include "runner.hpp"
int main() {
    runner::boot();
    runner::commit();
    runner::wait();
    runner::quit();

    runner::boot();
    runner::wait();
    runner::quit();

    runner::boot();
    runner::quit();
    return 0;
}
