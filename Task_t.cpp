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
	obj_mutex.lock();
	status = State::TASK_WORKS;
	obj_mutex.unlock();
    for (int i = 0; i < 30 ; i++)
    {
        // main thread work
//        std::cout << "Task #" << task_id << " works" << std::endl;
        usleep(500000);
    }
//	std::cout << "Task #" << task_id << " ends works" << std::endl;

	obj_mutex.lock();
	status = State::TASK_END;
	obj_mutex.unlock();
	progress = 100;
}

std::string Task_t::get_task_info() const
{
	// copy values then free lock
	std::unique_lock<std::mutex> lock_m(obj_mutex, std::defer_lock);
	lock_m.lock();
	auto _task_id = task_id;
	auto _delay_sec = delay_sec;
	auto _progress = progress;
	auto _status = status;
	auto _time_started = time_started;
	lock_m.unlock();
	std::string str_status;

	int rest_time;
	time_t now;
	switch (_status)
	{
		case State::TASK_WORKS:
			str_status = "in progress" ;
			break;
		case State::TASK_WAITING:
			time(&now);
			rest_time = _delay_sec - (int)difftime(now, _time_started);
			str_status = "is waiting (Time until start: " + std::to_string(rest_time) + " )" ;
			break;
		case State::TASK_PAUSE:
			str_status = "in pause";
			break;
	}
	std::string res = "Task #" + std::to_string(_task_id) + " ; staus: " + str_status + " ;progress " + std::to_string(_progress) + "\n";
	return res;
}

void Task_t::operator()() {
//    this->thread_operations();
    thread_operations();
    printf("a");
}

void Task_t::run() {
//    this->thread_operations();
//	std::thread t(&Task_t::thread_operations, this);
//	cur_thread = std::move(t);
	thread_operations();
}