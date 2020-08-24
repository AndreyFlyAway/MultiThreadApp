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
	Pyramid(std::vector<std::string> d);
	~Pyramid() {};
	int insert(const std::string s);
	void print()
	{
		for(int i = 1; i <= n; i++)
			std::cout << data_v[i] << " ";
		std::cout << std::endl;
	}

private:
	void bubble_up();
	void bubble_down(uint64_t i=1);

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