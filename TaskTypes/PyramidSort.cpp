#include <filesystem>
#include <fstream>
#include <algorithm>
#include "PyramidSort.h"

using namespace std::chrono_literals;
namespace fs = std::filesystem;

Pyramid::Pyramid():
	n(0)
{
	data_v.push_back("null");
}


Pyramid::Pyramid(const std::vector<std::string>& d)
{
	data_v.push_back("null");
	data_v.insert(data_v.end(), d.begin(), d.end());
	n = data_v.size() - 1;
	for(uint64_t i = n; i>= 1; i--)
	{
		bubble_down(i);
	}
}

std::vector<std::string> Pyramid::get_results()
{
	std::vector<std::string> results;
	while (n != 0)
	{
		results.push_back(extract_min());
	}
	return results;
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

std::string Pyramid::extract_min()
{
	std::string min;
	if (n != 0)
	{
		min = data_v[1];
		data_v[1] = data_v[n];
		data_v.pop_back();
		n-=1;
		bubble_down(1);
	}
	return min;
}

PyramidSortTask::PyramidSortTask(uint id, int delay,
		const std::string& f_read, const std::string& f_write):
		TaskAsyncProgress(id, delay),
		file_to_read(f_read),
		file_to_write(f_write)
{
	str_type = "Pyramid sort task";
}

void PyramidSortTask::thread_operations()
{
	fs::path p(file_to_read);
	if(!fs::exists(p))
	{
		set_results("File " + file_to_read + " doesnt exists.");
		return;
	}

	std::future<int> progress_val = std::async(&PyramidSortTask::progress_value_async, this, 5);

	// reading and sorting
	std::vector<std::string> data_from_file;
	std::string line;
	std::ifstream infile(file_to_read);
	while (std::getline(infile, line))
	{
		data_from_file.push_back(line);
		if (pause_flag)
		{
			std::unique_lock<std::mutex> lk(pause_mutex);
			resume_cond.wait(lk, [&]{return !(pause_flag.load());});
		}
		if (stop_flag)
			break;
	}
	// sorting
	auto start_t = std::chrono::system_clock::now();
	Pyramid pyramid_data(data_from_file);
	std::vector<std::string> sorted_data = pyramid_data.get_results();
	auto sorting_pyromid_t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_t);

	// TODO: probably should refactor duplication
	if (pause_flag)
	{
		std::unique_lock<std::mutex> lk(pause_mutex);
		resume_cond.wait(lk, [&]{return !(pause_flag.load());});
	}
	if (stop_flag)
		return;

	// writing results
	std::ofstream output_file(file_to_write);
	std::ostream_iterator<std::string> output_iterator(output_file, "\n");
	std::copy(sorted_data.begin(), sorted_data.end(), output_iterator);

	progress_val.wait();
	set_results("Pyramid sort total time: " + std::to_string(sorting_pyromid_t.count()));
	infile.close();
}