//
// Created by user on 22.04.20.
//

#include <string>
#include "Task_t.h"

Task_t::Task_t(uint id, int delay){
    task_id = id;
    delay_sec = delay;
    progress = 0;
    status = State::TASK_WAITING;
    time_started = 0;
}

void Task_t::thread_operations() {
    // std::this_thread::sleep_for(std::chrono::seconds(2)); это для сна
    for (int i = 0; i < 30 ; i++)
    {
        // main thread work
        std::cout << "Task #" << task_id << " works" << std::endl;
        usleep(500000);
    }
	std::cout << "Task #" << task_id << " ends works" << std::endl;
	std::unique_lock lock(obj_mutex);
	status = State::TASK_END;
	progress = 100;
}

void Task_t::operator()() {
//    this->thread_operations();
    thread_operations();
    printf("a");
}

void Task_t::run() {
//    this->thread_operations();
	std::thread t(&Task_t::thread_operations, this);
	cur_thread = std::move(t);
}