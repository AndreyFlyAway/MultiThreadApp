//
// Created by user on 22.04.20.
//

#include <string>
#include "Task_t.h"

Task_t::Task_t(uint id, int delay){
    task_id = id;
    delay_sec = delay;
    progress = 0;
    status = TASK_WAITING;
    time_started = 0;
}

void Task_t::thread_operations() const {
    // std::this_thread::sleep_for(std::chrono::seconds(2)); это для сна
    for (int i = 0; i < 30 ; i++)
    {
        // main thread work
        usleep(500000);
    }
}

void Task_t::operator()() const {
//    this->thread_operations();
    thread_operations();
    printf("a");
}