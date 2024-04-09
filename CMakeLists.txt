cmake_minimum_required(VERSION 3.20)
project(task_runner)
add_library(trunner src/task.cpp)
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-value -Wno-unused-result -Wno-unused-local-typedefs")

add_executable(test0 tests/test0.cpp)
target_link_libraries(test0 trunner)
