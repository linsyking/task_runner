cmake_minimum_required(VERSION 3.20)
project(task_runner)
add_library(trunner src/task.cpp src/runner.cpp)
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -Wall -Wextra -pedantic -O3")

add_executable(test0 tests/test0.cpp)
target_link_libraries(test0 trunner)
