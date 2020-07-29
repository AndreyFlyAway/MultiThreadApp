//
// Created by user on 22.04.20.
//

#include <string>
#include <future>
#include "TaskT.h"

TaskT::TaskT(uint id, int delay):
		pause_flag(false),
		task_id(id),
		delay_sec(delay),
		progress(0),
		status(State::TASK_WAITING),
		time_started(0),
		stop_flag(false)
{
}

void TaskT::thread_operations()
{
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

	progress = 100;
}

std::string TaskT::task_info() const
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
			str_status = "in waiting (Seconds until start: " + std::to_string(rest_time) + " )" ;
			break;
		case State::TASK_PAUSE:
			str_status = "in pause";
			break;
	}
	std::string res = "Task #" + std::to_string(_task_id) + " ; staus: " + str_status + " ; progress " + std::to_string(_progress) + "\n";
	return res;
}

int TaskT::set_status(State st)
{
	std::unique_lock lk(obj_mutex);
	status = st;
	return 0;
}

int TaskT::pause()
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

int TaskT::resume()
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
int TaskT::stop()
{
	stop_flag = true;
	set_status(State::TASK_END);
	// TODO: figure out do I need to use statement  (pause_flag == true) or I can use  (pause_flag) for atomic variable
	if (pause_flag == true)
		resume();
	return 0;
}

void TaskT::run()
{
	// TODO: replace for std::system_clock??
	time(&(time_started));
	if (delay_sec > 0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(delay_sec));
	}
	set_status(State::TASK_WORKS);
	thread_operations();
	set_status(State::TASK_END);
}

TaskAsyncProgress::TaskAsyncProgress(uint id, int delay):
		TaskT(id, delay)
{
}

void TaskAsyncProgress::thread_operations()
{
	std::future<int> progress_val = std::async(&TaskAsyncProgress::progress_value_async, this, 10);
	for (int i = 0; i < 20 ; i++)
	{
		if (pause_flag)
		{
			set_status(State::TASK_PAUSE);
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
			set_status(State::TASK_WORKS);
		}
		if (stop_flag)
			break;
		usleep(500000);
	}
}

int TaskAsyncProgress::progress_value_async(int sec_to_work)
{
	int ms_to_sleep = sec_to_work * 1000 / 99;
	for (int i = 0 ; i < 99 ; i++)
	{
		if (pause_flag)
		{
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
		}
		if (stop_flag)
			break;
		progress += 1;
		std::this_thread::sleep_for(std::chrono::milliseconds(ms_to_sleep));
	}
	return progress.load();
}