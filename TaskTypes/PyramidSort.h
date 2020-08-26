/*
 *  Here is no any new realization of pyramid sort algorithm, created just for practice
 */

#pragma once

#include <iostream>
#include <vector>
#include "TaskT.h"

// TODO: make template
class Pyramid{
public:
	uint64_t n;
	std::vector<std::string> data_v;

public:
	Pyramid();
	Pyramid(const std::vector<std::string>& d);
	~Pyramid() {};
	int insert(const std::string s);
	std::vector<std::string> get_results();
protected:
	void bubble_up();
	void bubble_down(uint64_t i=1);
	std::string extract_min();

	inline void swap(uint64_t i, uint64_t j) noexcept
	{
		// TODO: use move??
		std::string tmp = data_v[i];
		data_v[i] = data_v[j];
		data_v[j] = tmp;
	}

	inline uint64_t parent(uint64_t n)
	{
		return ((n == 1) ? 0 : (n / 2));
	}

	inline uint64_t young_ch(uint64_t n)
	{
		return (n * 2);
	}

	inline int compare_str(uint64_t i, uint64_t j)
	{
		return data_v[i].compare(data_v[j]);
	}

};

/* @brief Task that sort string from one file and write result in other file. Each string must be
 * placed at new line
 */
class PyramidSortTask: public TaskAsyncProgress {
protected:
	std::string file_to_read;  // path where data for sorting placed
	std::string file_to_write; // path where result data will be placed
public:
	PyramidSortTask(uint id, int delay, const std::string& f_read, const std::string& f_write);
protected:
	void thread_operations() override;
};

