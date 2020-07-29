cmake_minimum_required(VERSION 3.10)
project(MultiThreadApp)

set(CMAKE_CXX_STANDARD 17)

add_executable(MultiThreadApp main.cpp stdTaskManager.cpp TaskT.cpp)
target_link_libraries(MultiThreadApp pthread)
