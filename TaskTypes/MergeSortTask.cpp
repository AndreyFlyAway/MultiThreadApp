#include "MergeSortTask.h"
#include <iostream>
#include <deque>

int mergesort(std::string& _str, int low, int high)
{
	int i;
	int middle;
	if (low < high)
	{
		middle = (low + high) / 2;
		mergesort(_str, low, middle);
		mergesort(_str, middle + 1, high);
		merge(_str, low, middle, high);
	}
	return 0;
}

int merge(std::string& _str, int low, int middle, int high)
{
	int i;
	std::deque<char> buffer1;
	std::deque<char> buffer2;
	for(i = low ; i <= middle ; i++)
		buffer1.push_back(_str[i]);
	for(i = middle+1 ; i <= high; i++)
		buffer2.push_back(_str[i]);
	i = low;
	while(!(buffer1.empty() || buffer2.empty()))
	{
		if (buffer1.front() <= buffer2.front())
		{
			_str[i++] = buffer1.front();
			buffer1.pop_front();
		}
		else
		{
			_str[i++] = buffer2.front();
			buffer2.pop_front();
		}
	}
	while (!buffer1.empty())
	{
		_str[i++] = buffer1.front();
		buffer1.pop_front();
	}
	while (!buffer2.empty())
	{
		_str[i++] = buffer2.front();
		buffer2.pop_front();
	}
	return 0;
}

MergeSortTask::MergeSortTask(uint id, int delay, const std::string to_sort):
		TaskAsyncProgress(id, delay),
		str_to_sort(to_sort)
{
	str_type = "MergeSortTask";
}

void MergeSortTask::thread_operations()
{
	std::future<int> progress_val = std::async(&MergeSortTask::progress_value_async, this, 2);
	if (pause_flag)
	{
		std::unique_lock<std::mutex> lk(pause_mutex);
		resume_cond.wait(lk, [&]{return !(pause_flag.load());});
	}
	if (stop_flag)
		return;
	mergesort(str_to_sort, 0, str_to_sort.size());
	progress_val.wait();
	set_results("Merge sort results " + str_to_sort);
	return;
}