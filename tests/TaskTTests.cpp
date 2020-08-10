#include <gtest/gtest.h>
#include "TaskT.h"
#include <chrono>

class TaskTTests : public ::testing::Test
{
protected:
	std::shared_ptr<TaskT> task;
	TaskTTests()
	{
		task = std::make_shared<TaskT>(1, 1);
	}

	~TaskTTests()
	{
		task.reset();
	}
};

TEST_F(TaskTTests, someTests)
{
	using namespace std::chrono_literals;
	task->run();
	ASSERT_TRUE(task->get_status() == State::TASK_WAITING);
	std::this_thread::sleep_for(2s);
	ASSERT_TRUE(task->get_status() == State::TASK_WORKS);
	task->pause();
	std::this_thread::sleep_for(1s);
	ASSERT_TRUE(task->get_status() == State::TASK_PAUSE);
	task->resume();
	std::this_thread::sleep_for(1s);
	ASSERT_TRUE(task->get_status() == State::TASK_WORKS);
	task->stop();
	std::this_thread::sleep_for(1s);
	ASSERT_TRUE(task->get_status() == State::TASK_END);
}