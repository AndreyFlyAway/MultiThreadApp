#pragma once

#include <iostream>
#include "TaskT.h"

/*
 * task that works infinitely
 */
class InfinityTask : public TaskT {
public:
	InfinityTask(uint id, int delay);
	virtual ~InfinityTask() override {};
private:
	void thread_operations() override;
};


