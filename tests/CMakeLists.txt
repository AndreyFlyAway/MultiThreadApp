cmake_minimum_required(VERSION 3.0)
project(unit-tests)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/TaskTypes)

add_executable(${PROJECT_NAME} TaskTTests.cpp RunTests.cpp ${CMAKE_SOURCE_DIR}/TaskTypes/TaskT.cpp)

target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} pthread)