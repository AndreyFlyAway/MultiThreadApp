//
// Created by user on 22.04.20.
//

#include "stdTaskManager.h"

/* command constants*/
static const std::string EXIT_CMD = "q";
static const std::string START_TASK_CMD = "s";
static const std::string INFO_CMD = "info";
static const std::string PAUSE_TASK = "p";
static const std::string CONTINUE_TASK = "c";

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
	std::cout << "Information about all tasks:" << std::endl;
	std::cout << INFO_CMD << std::endl;
	std::cout << "Information about all one task by ID:" << std::endl;
	std::cout << INFO_CMD << " [task ID]" << std::endl;
	std::cout << "Enter " << EXIT_CMD << " to quit." << std::endl;
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

void TaskPool::thread_wrapper(const std::shared_ptr<Task_t> task, uint task_id)
{
	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list.erase (task_id);
}

int TaskPool::get_task_info(uint task_id)
{
	int res = 0;
	std::shared_ptr<Task_t> task;
	if (task_id > 0)
	{
		std::shared_lock lock(g_task_list_mutex);
		if (g_task_list.find(task_id) != g_task_list.end())
		{
			task = g_task_list[task_id];
			std::cout << task->task_info() << std::endl;
			res = 0;
		}
		else
		{
			std::cout << "There are no task with this task id" << std::endl;
			res = -1;
		}
	}
	else
	{
		std::shared_lock lock(g_task_list_mutex);
		if (g_task_list.size())
		{
			for( auto const& [key, val] : g_task_list)
			{
				task = val;
				std::cout << task->task_info() << std::endl;
			}
		}
		else
		{
			std::cout << "No active tasks in pull"<< std::endl;
		}
	}

	return res;
}

int TaskPool::task_mannger(const std::string cmd)
{
	/* разделяю строку по словам */
	std::stringstream ss(cmd);
	std::istream_iterator<std::string> begin_s(ss);
	std::istream_iterator<std::string> end_s;
	std::vector<std::string> commands(begin_s, end_s);

	if (commands.size() < 1)
	{
		print_help(WRONG_FMT);
		return -1;
	}
	if (commands[0] == EXIT_CMD)
	{
		printf("End work bye!\n");
		return 1;
	}
	else if (commands[0] == START_TASK_CMD) // старт задачи
	{
		// TODO: do something with this big monstrous code
		if (commands.size() != 2)
		{
			print_help(WRONG_FMT);
			return -2;
		}
		int delay = 0;
		if (commands[1] == "now")
		{
			delay = 0;
		}
		else
		{
			if (is_number(commands[1]))
				delay = stoi(commands[1]);
			else
			{
				print_help(WRONG_FMT);
				return -2;
			}
		}

		int res = start_task(delay) ;
	}
	else if (commands[0] == INFO_CMD) // printing info about task / вызов информации о задаче
	{
		if (commands.size() < 2)
		{
			get_task_info(0);
		}
		else
		if (is_number(commands[1]))
		{
			uint id = uint(std::stoi(commands[1]));
			get_task_info(id);
		}
	}
	else if (commands[0] == PAUSE_TASK) // pause task / поставить задачу на паузу
	{
	}
	else if (commands[0] == CONTINUE_TASK) // continue task / снять задачу с паузы
	{
	}
	else
	{
		print_help(UNREC_CMD);
	}
	return 0;
}

int TaskPool::std_multi_hread_main()
{
	int res;
	std::string cmd;
	bool exit_f = false;
	pthread_t mange_cmd;
	print_help();
	while (!exit_f) // TODO: ???
	{
		getline(std::cin, cmd);
		if ((res=task_mannger(cmd)) == 1){
			exit_f = true;
			break;
		}
	}
	return res;
}