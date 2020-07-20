//
// Created by user on 22.04.20.
//

#include <string>
#include "Task_t.h"

Task_t::Task_t(uint id, int delay):
		pause(false),
		task_id(id),
		delay_sec(delay),
		progress(0),
		status(State::TASK_WAITING),
		time_started(0)
{
}

void Task_t::thread_operations()
{
	// TODO: replcase for std::system_clock??
	time(&(time_started));
	if (delay_sec > 0)
	{
		std::chrono::seconds s(delay_sec);
		std::this_thread::sleep_for(s);
	}
	obj_mutex.lock();
	status = State::TASK_WORKS;
	obj_mutex.unlock();
	for (int i = 0; i < 30 ; i++)
	{
		if (pause)
		{
			std::unique_lock<std::mutex> lk(pause_mutex);
			// TODO: figure out how to use pause variable in this condition
			resume_cond.wait(lk);
		}
		progress += 3;
		usleep(500000);
	}

	obj_mutex.lock();
	status = State::TASK_END;
	obj_mutex.unlock();
	progress = 100;
}

std::string Task_t::task_info() const
{
	// copy values then free lock
	std::shared_lock lock(obj_mutex);
	auto _task_id = task_id;
	auto _delay_sec = delay_sec;
	auto _status = status;
	auto _time_started = time_started;
	lock.unlock();
	// TODO: just use assign or explicit load?
	int _progress = progress.load();

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
			str_status = "in waiting (Time until start: " + std::to_string(rest_time) + " )" ;
			break;
		case State::TASK_PAUSE:
			str_status = "in pause";
			break;
	}
	std::string res = "Task #" + std::to_string(_task_id) + " ; staus: " + str_status + " ; progress " + std::to_string(_progress) + "\n";
	return res;
}

int Task_t::pause_task()
{
	pause = true;
}


int Task_t::resume_task()
{
	pause = false;
	resume_cond.notify_one();
}

void Task_t::run()
{
	thread_operations();
}