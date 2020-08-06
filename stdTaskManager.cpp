//
// Created by user on 22.04.20.
//

#include "stdTaskManager.h"

using Task_shr_p = std::shared_ptr<TaskT>;

/* command constants*/
static const std::string EXIT_CMD = "q";
static const std::string START_TASK_CMD = "s";
static const std::string INFO_CMD = "info";
static const std::string PAUSE_TASK = "p";
static const std::string CONTINUE_TASK = "c";
static const std::string STOP_TASK_CMD = "stop";
/* names of task types */
static const std::string ASYN_PORGRESS_T_CMD = "asyn_prog";


/* look function print_help */
static const int WRONG_FMT = 1;
static const int UNREC_CMD = 2;

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

int TaskPool::start_task(int delay, TaskTypes type_of_prog)
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

	std::shared_ptr<TaskT> task;
	switch (type_of_prog) {
		case TaskTypes::SIMPLE:
			task = std::make_shared<TaskT>(g_task_count, _delay);
			break;
		case TaskTypes::ASYNC_PROGRS:
			task = std::make_shared<TaskAsyncProgress>(g_task_count, _delay);
			break;
		default:
			task = std::make_shared<TaskT>(g_task_count, _delay);
			break;
	}

	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list[g_task_count] = task;
	std::cout << "Task #" << g_task_count << " started" << std::endl;
	g_task_count++;
	return ret;
}

int TaskPool::stop_all()
{
	std::unique_lock lock(g_task_list_mutex);
	for(const auto& it: g_task_list)
	{
		Task_shr_p t = it.second;
		t->stop();
	}
	return 0;
}

int TaskPool::get_all_task_info()
{
	int res = 0;
	std::shared_lock lock(g_task_list_mutex);
	std::string asnwer;
	if (!g_task_list.empty())
	{
		for( auto const& [key, val] : g_task_list)
		{
			Task_shr_p task = val;
			asnwer += task->task_info();
		}
	}
	else
	{
		asnwer = "No active tasks in pull";
	}
	lock.unlock();

	std::cout << asnwer << std::endl;

	return res;
}

int TaskPool::task_manager(const std::string& cmd)
{
	clean_tasks_pool();
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
		{
			stop_all();
			clean_tasks_pool();
			return 1;
		}
		else
			print_help(UNREC_CMD);
	}
	else if (commands.size() == 2)
	{
		uint num_val = 0;
		if (is_number(commands[1]))
		{
			num_val = uint(std::stoi(commands[1]));
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
			if (cmd_type == START_TASK_CMD)
			{
				if (commands[1] == ASYN_PORGRESS_T_CMD)
					start_task(num_val, TaskTypes::ASYNC_PROGRS);
				else
					print_help(UNREC_CMD);
			}
			else
				print_help(UNREC_CMD);
			return 0;
		}
	}
	else if (commands.size() == 3)
	{
		if (commands[0] == START_TASK_CMD)
		{
			if (commands[1] == ASYN_PORGRESS_T_CMD)
			{
				uint num_val = 0;
				if (is_number(commands[2]))
					num_val = uint(std::stoi(commands[2]));
				start_task(num_val, TaskTypes::ASYNC_PROGRS);
			}
		}
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

	Task_shr_p task = it->second;
	std::string debug_info;

	switch (op) {
		case OperationCode::STOP:
		{
			ret = task->stop();
			if (ret == 0)
				debug_info = "Task " + std::to_string(task_id) + " stopped ";
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
			if (ret == 0)
				debug_info = "Task " + std::to_string(task_id) + " paused ";
			else
				debug_info = "Task " + std::to_string(task_id) + " on pause already";
			break;
		}
		case OperationCode::CONTINUE:
		{
			ret = task->resume();
			if (ret == 0)
				debug_info = "Task " + std::to_string(task_id) + " resumed ";
			else
				debug_info = "Task " + std::to_string(task_id) + " is not on pause ";
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

void TaskPool::clean_tasks_pool()
{
	// TODO: optimize?
	std::unique_lock lock(g_task_list_mutex);
	std::vector<int> id_to_delete;
	for(const auto& it : g_task_list)
	{
		Task_shr_p t = it.second;
		if (t->get_status() == State::TASK_END)
		{
			id_to_delete.push_back(it.first);
			t->cur_thread.join();
		}
	}
	for(int i: id_to_delete)
		g_task_list.erase(i);
}

TaskPool::TaskPool():
	exit_flag(false)
{
}

int TaskPool::std_multi_thread_main()
{
	std::string cmd;
	print_help();
	while (!exit_flag)
	{
		getline(std::cin, cmd);
		if (task_manager(cmd) == 1){
			exit_flag = true;
		}
	}
	return 0;
}