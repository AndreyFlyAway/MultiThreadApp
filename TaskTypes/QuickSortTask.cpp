#include "QuickSortTask.h"

int quick_sort(std::string& s, int l, int h)
{
	int p;
	if((h-l)>0)
	{
		p = partition(s, l, h);
		quick_sort(s, 0, p-1);
		quick_sort(s, p+1, h);
	}
	return 0;
}

int partition(std::string& s, int l, int h)
{
	int f = l;
	int p = h;
	for(int i = l; i < h ; i++)
		if (s[i] < s[p])
		{
			swap_ch(s, i, f);
			f++;
		}
	swap_ch(s, p, f);
	return f;
}


int swap_ch(std::string &s, int i, int j)
{
	auto tmp = s[i];
	s[i] = s[j];
	s[j] = tmp;
	return 0;
}

QuickSortTask::QuickSortTask(uint id, int delay, const std::string to_sort):
		TaskAsyncProgress(id, delay),
		str_to_sort(to_sort)
{
	str_type = "QuickSortTask";
}

void QuickSortTask::thread_operations()
{
	std::future<int> progress_val = std::async(&QuickSortTask::progress_value_async, this, 2);
	if (pause_flag)
	{
		std::unique_lock<std::mutex> lk(pause_mutex);
		resume_cond.wait(lk, [&]{return !(pause_flag.load());});
	}
	if (stop_flag)
		return;
	quick_sort(str_to_sort, 0, str_to_sort.size()-1);
	progress_val.wait();
	set_results("Quick sort results " + str_to_sort);
	return;
}