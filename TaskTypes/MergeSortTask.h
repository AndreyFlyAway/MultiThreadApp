/*
 * Here is no any new realization of merge  sort algorithm, created just for practice
 */

#pragma once
#include <list>
#include <iostream>
#include "TaskT.h"

/* @brief recursive merge sort algorithm
 * @param _str string to sort
 * @param low low limit of string
 * @param low high limit of string
 * @return 0
 */
int mergesort(std::string& _str, int low, int high);
int merge(std::string& _str, int low, int middle, int high);

/* @brief task that sort letter in word using merge sort / сортировка букв в слое используя метод сортировки слиянием */
class MergeSortTask: public TaskAsyncProgress
{
public:
	MergeSortTask(uint id, int delay, const std::string to_sort);
	~MergeSortTask() {};
protected:
	std::string str_to_sort; // word to sort / слово для сортироваки
	void thread_operations() override;
};