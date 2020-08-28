#include <string>
#include "TaskT.h"
#include <chrono>

using namespace std::chrono_literals;

static const std::string RES_NOT_READY= "not ready";

TaskT::TaskT(uint id, int delay):
		pause_flag(false),
		task_id(id),
		delay_sec(delay),
		progress(0),
		status(State::TASK_WAITING),
		time_started(0),
		stop_flag(false),
		str_type("simple task"),
		result_info("no result")
{
}

TaskT::~TaskT()
{
	// TODO: think more about it
	stop();
	if (cur_thread.joinable())
		cur_thread.join();
}

void TaskT::thread_operations()
{
	for (int i = 0; i < 30 ; i++)
	{
		if (stop_flag)
			return;
		if (pause_flag)
		{
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
		}

		progress += 3;

		if (stop_flag)
			return;
		std::this_thread::sleep_for(500ms);
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
		case State::TASK_END:
			str_status = "ending";
			break;
	}
	std::string res = "Task #" + std::to_string(_task_id) + " of type " + str_type +
			"; staus: " + str_status + " ; progress " + std::to_string(_progress) + "\n";
	return res;
}

int TaskT::set_status(State st)
{
	std::unique_lock lock(obj_mutex);
	status = st;
	return 0;
}

void TaskT::thread_function(std::chrono::seconds time_tleep)
{
	try {
		// TODO: replace for std::system_clock??
		time(&(time_started));
		if (time_tleep != std::chrono::seconds::zero())
		{
			// no need to use status(State::TASK_WAITING), 'cause it's init value
			std::chrono::seconds s(delay_sec);
			for(int i = 1; i <= delay_sec ; i++)
			{
				if (get_status() == State::TASK_END)
					return;
				std::this_thread::sleep_for(1s);
			}
		}
		set_status(State::TASK_WORKS);
		thread_operations();
	}
	catch (const std::exception & e)
	{
		set_results("error in process caused");
		std::cout << "In task #" << task_id << " cause some exception: " + std::string(e.what()) << std::endl;
	}
	set_status(State::TASK_END);
}

int TaskT::set_results(const std::string& s)
{
	if (get_status() != State::TASK_END)
	{
		std::unique_lock lk(obj_mutex);
		result_info = s;
		return 0;
	}
	else
	{
		return -1;
	}
}

State TaskT::get_status() const
{
	std::shared_lock lock(obj_mutex);
	return status;
}

int TaskT::pause()
{
	int ret = 0;
	if (pause_flag == false)
	{
		set_status(State::TASK_PAUSE);
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
		set_status(State::TASK_WORKS);
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
	set_results("force stopped");
	// TODO: figure out do I need to use statement  (pause_flag == true) or I can use  (pause_flag) for atomic variable
	if (pause_flag == true)
		resume();
	return 0;
}

std::string TaskT::get_results()
{
	if (get_status() == State::TASK_END)
	{
		// TODO: probably no benefit to use mutex
		std::unique_lock lk(obj_mutex);
		return result_info;
	}
	else
		return RES_NOT_READY;
}

void TaskT::run()
{
	std::chrono::seconds s(delay_sec);
	cur_thread = std::thread(&TaskT::thread_function, this, s);
}

TaskAsyncProgress::TaskAsyncProgress(uint id, int delay):
		TaskT(id, delay)
{
	str_type = "with async progress";
}

void TaskAsyncProgress::thread_operations()
{
	std::future<int> progress_val = std::async(&TaskAsyncProgress::progress_value_async, this, 10);
	for (int i = 0; i < 20 ; i++)
	{
		if (pause_flag)
		{
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
		}
		if (stop_flag)
		{
			break;
		}
		std::this_thread::sleep_for(500ms);
	}
	progress_val.wait();
	set_results("AsyncProgress type of task doesnt have results");
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