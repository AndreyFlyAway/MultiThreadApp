/*
 *  Here is no any new realization of quick sort algorithm, created just for practice
 */
#pragma once

#include <iostream>
#include "TaskT.h"

int quick_sort(std::string& s, int l, int h);
int partition(std::string& s, int l, int h);
int swap_ch(std::string &s, int i, int j);

/* @brief task that sort letter in word using quick sort / сортировка букв в слое используя метод быстрой сортировки*/
class QuickSortTask: public TaskAsyncProgress
{
public:
	QuickSortTask(uint id, int delay, const std::string to_sort);
	~QuickSortTask() {};
protected:
	std::string str_to_sort; // word to sort / слово для сортироваки
	void thread_operations() override;
};