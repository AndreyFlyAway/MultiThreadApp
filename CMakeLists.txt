cmake_minimum_required(VERSION 3.10)
project(MultiThreadApp)

set(CMAKE_CXX_STANDARD 11)

add_executable(MultiThreadApp main.cpp stdTaskManager.cpp Task_t.cpp)
target_link_libraries(MultiThreadApp pthread)
