cmake_minimum_required(VERSION 3.20)
project(task_runner)
add_library(trunner src/runner.cpp)
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -Wall -Wextra -pedantic -O3")

add_executable(basic tests/basic.cpp)
target_link_libraries(basic trunner)

add_executable(multipass tests/multipass.cpp)
target_link_libraries(multipass trunner)

add_executable(manytasks tests/manytasks.cpp)
target_link_libraries(manytasks trunner)

add_executable(noop tests/noop.cpp)
target_link_libraries(noop trunner)

add_executable(simple tests/simple.cpp)
target_link_libraries(simple trunner)

add_executable(mpass2 tests/mpass2.cpp)
target_link_libraries(mpass2 trunner)
