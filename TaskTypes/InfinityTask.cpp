#include <cmath>
#include "InfinityTask.h"
#include <chrono>

using namespace std::chrono_literals;

InfinityTask::InfinityTask(uint id, int delay):
		TaskT(id, delay)
{
	str_type = "infinity work";
}

void InfinityTask::thread_operations()
{
	static double v = 0.0;
	static double pi = 3.14159265;
	auto f = []{v += 0.1; return cos ( v * pi / 180.0 );};
	while (!stop_flag.load())
	{
		if (pause_flag)
		{
			set_status(State::TASK_PAUSE);
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
			set_status(State::TASK_WORKS);
		}
		f();
		progress += 1;
		// add it to not allow thread to take too much of process time
		std::this_thread::sleep_for(500ms);
	}
}