/*
 *  Here is no any new realization of pyramid sort algorithm, created just for practice
 */

#include <iostream>
#include <vector>

#pragma once

// TODO: make template
class Pyramid{
public:
	uint64_t n;
	std::vector<std::string> data_v;

public:
	Pyramid();
	~Pyramid() {};
	int insert(const std::string s);

private:
	void bubble_up();

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

};