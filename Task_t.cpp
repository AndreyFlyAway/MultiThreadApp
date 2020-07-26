//
// Created by user on 22.04.20.
//

#include <string>
#include "Task_t.h"

Task_t::Task_t(uint id, int delay):
		pause_flag(false),
		task_id(id),
		delay_sec(delay),
		progress(0),
		status(State::TASK_WAITING),
		time_started(0),
		stop_flag(false)
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
	set_status(State::TASK_WORKS);
	for (int i = 0; i < 1000 ; i++)
	{
		if (pause_flag)
		{
			set_status(State::TASK_PAUSE);
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
			set_status(State::TASK_WORKS);
		}
		if (stop_flag)
			return;
		progress += 3;
		usleep(500000);
	}

	set_status(State::TASK_END);
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

int Task_t::set_status(State st)
{
	std::unique_lock lk(obj_mutex);
	status = st;
	return 0;
}

int Task_t::pause()
{
	int ret = 0;
	if (pause_flag == false)
	{
		pause_flag = true;
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	return ret;
}

int Task_t::resume()
{

	int ret = 0;
	if (pause_flag == true)
	{
		pause_flag = false;
		resume_cond.notify_one();
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	return ret;
}
int Task_t::stop()
{
	stop_flag = true;
	set_status(State::TASK_END);
	// TODO: figure out do I need to use statement  (pause_flag == true) or I can use  (pause_flag) for atomic variable
	if (pause_flag == true)
		resume();
	return 0;
}

void Task_t::run()
{
	thread_operations();
}