//
// Created by user on 22.04.20.
//

#include "stdTaskManager.h"

using Task_shr_p = std::shared_ptr<Task_t>;

/* command constants*/
static const std::string EXIT_CMD = "q";
static const std::string START_TASK_CMD = "s";
static const std::string INFO_CMD = "info";
static const std::string PAUSE_TASK = "p";
static const std::string CONTINUE_TASK = "c";
static const std::string STOP_TASK_CMD = "stop";

/* look function print_help */
static const int WRONG_FMT = 1;//
static const int UNREC_CMD = 2;//

/* @brief checking if word is a number / проверка являеться ли число строкой
* @return boolean true/false / булево значение */
bool is_number(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

void TaskPool::print_help(int wrong_fmt) const
{
	switch (wrong_fmt)
	{
		case WRONG_FMT:
			std::cout << "Wrong command format!" << std::endl;
			break;
		case UNREC_CMD:
			std::cout << "Unrecognized command." << std::endl;
			break;
	}

	std::cout << "Start task command format without delay:" << std::endl;
	std::cout << START_TASK_CMD << " now" << std::endl;
	std::cout << "Start task command format with delay" << std::endl;
	std::cout << START_TASK_CMD << " [time]" << std::endl;
	std::cout << "Stop task command" << std::endl;
	std::cout << STOP_TASK_CMD << " [task ID]" << std::endl;
	std::cout << "Information about all tasks:" << std::endl;
	std::cout << INFO_CMD << std::endl;
	std::cout << "Information about all one task by ID:" << std::endl;
	std::cout << INFO_CMD << " [task ID]" << std::endl;
	std::cout << "Enter " << EXIT_CMD << " to quit." << std::endl;
	std::cout << std::endl;
}

int TaskPool::start_task(int delay)
{
	// TODO: make warning if amount of tasks is over hardware_concurrency - 1
	static uint g_task_count = 1;
	int _delay = delay;
	if (_delay < 0)
	{
		std::cout << "ERROR: delay cant be negative! Started without delay. " << std::endl;
		_delay = 0;
	}
	int ret = 0;

	auto task = std::make_shared<Task_t>(g_task_count, _delay);
	std::thread t(&TaskPool::thread_wrapper, this, task, g_task_count);
	t.detach();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list[g_task_count] = task;
	lock.unlock();
	std::cout << "Task #" << g_task_count << " started" << std::endl;
	g_task_count++;
	return ret;
}

void TaskPool::thread_wrapper(const Task_shr_p task, uint task_id)
{
	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list.erase(task_id);
}

int TaskPool::get_all_task_info()
{
	int res = 0;
	std::shared_lock lock(g_task_list_mutex);
	if (g_task_list.size())
	{
		for( auto const& [key, val] : g_task_list)
		{
			Task_shr_p task = val;
			std::cout << task->task_info() << std::endl;
		}
	}
	else
	{
		std::cout << "No active tasks in pull"<< std::endl;
	}

	return res;
}

int TaskPool::task_manager(const std::string cmd)
{
	/* разделяю строку по словам */
	std::stringstream ss(cmd);
	std::istream_iterator<std::string> begin_s(ss);
	std::istream_iterator<std::string> end_s;
	std::vector<std::string> commands(begin_s, end_s);
	if (commands.size() == 0)
	{
		print_help(WRONG_FMT);
		return -1;
	}

	std::string cmd_type = commands[0];

	if (commands.size() == 1)
	{
		if (cmd_type == START_TASK_CMD)
			start_task(0);
		else if (cmd_type == INFO_CMD)
			get_all_task_info();
		else if (cmd_type == EXIT_CMD)
			return 1;
		else
			print_help(UNREC_CMD);
	}
	else if (commands.size() == 2)
	{
		uint num_val = 0;
		if (is_number(commands[1]))
			num_val = uint(std::stoi(commands[1]));
		else
		{
			print_help(WRONG_FMT);
			return 0;
		}

		if (cmd_type == START_TASK_CMD)
			start_task(num_val);
		else if (cmd_type == INFO_CMD)
			operation_manager(num_val, OperationCode::INFO);
		else if (cmd_type == PAUSE_TASK)
			operation_manager(num_val, OperationCode::PAUSE);
		else if (cmd_type == CONTINUE_TASK)
			operation_manager(num_val, OperationCode::CONTINUE);
		else if (cmd_type == STOP_TASK_CMD)
			operation_manager(stoi(commands[1]), OperationCode::STOP);
		else
			print_help(UNREC_CMD);
	}
	else
	{
		print_help(UNREC_CMD);
	}
	return 0;
}

int TaskPool::operation_manager(uint task_id, OperationCode op)
{
	int ret = 0;
	std::shared_lock lock(g_task_list_mutex);
	auto it = g_task_list.find(task_id);
	if (it == g_task_list.end())
	{
		std::cout << "There are no task with this task id" << std::endl;
		return -1;
	}

	auto task = it->second;
	std::string debug_info;

	switch (op) {
		case OperationCode::STOP:
		{
			ret = task->stop();
			break;
		}
		case OperationCode::INFO:
		{
			debug_info = task->task_info();
			break;
		}
		case OperationCode::PAUSE:
		{
			ret = task->pause();
			break;
		}
		case OperationCode::CONTINUE:
		{
			ret = task->resume();
			break;
		}
	}
	// To free mutex as fast as it possible
	task.reset();
	lock.unlock();
	if (debug_info.length())
		std::cout << debug_info << std::endl;

	return ret;
}

int TaskPool::std_multi_thread_main()
{
	int res;
	std::string cmd;
	bool exit_f = false;
	pthread_t mange_cmd;
	print_help();
	while (!exit_f) // TODO: ???
	{
		getline(std::cin, cmd);
		if ((res = task_manager(cmd)) == 1){
			exit_f = true;
			break;
		}
	}
	return res;
}