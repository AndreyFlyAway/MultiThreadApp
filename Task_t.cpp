//
// Created by user on 22.04.20.
//

#include <string>
#include "Task_t.h"

Task_t::Task_t(uint id, int delay){
    in_process = true;
    task_id = id;
    delay_sec = delay;
    progress = 0;
    status = TASK_WAITING;
    std::asctime(std::localtime(&time_started));
//    time_started = 0;
    in_process = false;
}

void Task_t::set_status(int st){
    const std::lock_guard<std::mutex> lock(obj_mutex);
    status = st;
}


std::string Task_t::str_task_info(){
    std::string res = "";
    in_process = true;
    switch (status)
    {
        case TASK_WORKS:
            res = "Task id " + std::to_string(task_id) + " in progress: " + std::to_string(progress);
            break;
        case TASK_WAITING:
            std::time_t diff;
            diff = delay_sec - std::difftime(std::time(nullptr), time_started);
            res = "Task id " + std::to_string(task_id) + " is waiting. Time until start: " + std::to_string(diff);
            break;
        case TASK_PAUSE:
            res = "Task id " + std::to_string(task_id) + " in pause. Current progress: " + std::to_string(progress);
            break;
        default:
            res = "Unrecognised state for task id "  + std::to_string(task_id);
            break;
    }
    in_process = false;
    return res;
}

void Task_t::thread_operations() {
    // std::this_thread::sleep_for(std::chrono::seconds(2)); это для сна
    for (int i = 0; i < 30 ; i++)
    {
        // main thread work
//        std::cout << "Thread work " << i << std::endl;
        usleep(500000);
    }
}

void Task_t::operator()() {
//    this->thread_operations();
    set_status(TASK_WORKS);
    thread_operations();
    printf("a");
}