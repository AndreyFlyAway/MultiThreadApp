cmake_minimum_required(VERSION 3.10)
project(MultiThreadApp)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(tests)
add_definitions(-DTEST_MODE)
add_executable(MultiThreadApp
        main.cpp
        stdTaskManager.cpp
        TaskTypes/TaskT.cpp
        TaskTypes/InfinityTask.cpp
        TaskTypes/PyramidSort.cpp
        TaskTypes/MergeSortTask.cpp)

target_link_libraries(MultiThreadApp pthread)
