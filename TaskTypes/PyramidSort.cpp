#include "PyramidSort.h"

using namespace std::chrono_literals;

Pyramid::Pyramid():
	n(0)
{
	data_v.push_back("null");
}
Pyramid::Pyramid(std::vector<std::string> d)
{
	data_v.push_back("null");
	data_v.insert(data_v.end(), d.begin(), d.end());
	n = data_v.size() - 1;
	for(uint64_t i = n; i>= 1; i--)
	{
		bubble_down(i);
	}
}

int Pyramid::insert(const std::string s)
{
	n += 1;
	// TODO: should I?
	data_v.push_back(std::move(s));
	bubble_up();

	return 0;
}

void Pyramid::bubble_up()
{
	uint64_t p = parent(n);
	uint64_t c = n;
	if (p == 0)
		return;
	while ((p != 0) && (compare_str(p, c) > 0))
	{
		swap(p, c);
		c = p;
		p = parent(c);
	}
}

void Pyramid::bubble_down(uint64_t p)
{
	uint64_t min_index = p;
	while (true) {
		uint64_t c = young_ch(p);
		for (int i = 0; (i <= 1) && ((c + i) <= n); i++)
			if (compare_str(min_index, c + i) > 0)
				min_index = c + i;
		if (min_index != p) {
			swap(p, min_index);
			p = min_index;
		}
		else
			break;
	}
}

PyramidSortTask::PyramidSortTask(uint id, int delay,
		const std::string& f_read, const std::string& f_write):
		TaskAsyncProgress(id, delay),
		file_to_read(f_read),
		file_to_write(f_write)
{
	str_type = "Pyramid sort task";
	pyramid = std::make_shared<Pyramid>();
}


void PyramidSortTask::thread_operations()
{
	std::future<int> progress_val = std::async(&PyramidSortTask::progress_value_async, this, 10);
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
		{
			break;
		}
		// TODO: do sorting things
	}
	progress_val.wait();
	set_results("");
}