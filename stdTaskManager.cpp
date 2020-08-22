#include <cstdarg>
#include "stdTaskManager.h"

using Task_shr_p = std::shared_ptr<TaskT>;

/* command constants*/
static const std::string EXIT_CMD = "quit";
static const std::string START_TASK_CMD = "start";
static const std::string INFO_CMD = "info";
static const std::string PAUSE_TASK_CMD = "pause";
static const std::string CONTINUE_TASK_CMD = "resume";
static const std::string STOP_TASK_CMD = "stop";
static const std::string TEST_THREADS_CMD = "start_1000";
/* names of task types */
static const std::string ASYN_PORGRESS_T_CMD = "asyn_prog";
static const std::string INF_T_CMD = "inf";


/* look function print_help */
static const int WRONG_FMT = 1;
static const int UNREC_CMD = 2;


TaskTypes str_to_task(std::string s)
{
	static const std::map<std::string , TaskTypes> mapping = {
			{ASYN_PORGRESS_T_CMD, TaskTypes::ASYNC_PROGRS},
			{INF_T_CMD, TaskTypes::INFINITY}
	};
	return mapping.at(s);
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

	const char common_format[] = "\t%-25s\t%s\n";
	printf( "Commands:\n");
	printf(common_format, START_TASK_CMD.c_str(),  "Start without delay. By default simple task will be created." );
	printf(common_format, (START_TASK_CMD + " [time]").c_str(),  "Start task with delay in seconds." );
	printf(common_format, (START_TASK_CMD + " " + ASYN_PORGRESS_T_CMD + " [time]").c_str(),  "Start task with asynchronous increasing of progress. Can be started with delay or not" );
	printf(common_format, (START_TASK_CMD + " " + INF_T_CMD + " [time]").c_str(),  "Start infinity task." );
	printf(common_format, (STOP_TASK_CMD + " [task ID]" ).c_str(),  "Stop task by ID." );
	printf(common_format, (PAUSE_TASK_CMD + " [task ID]" ).c_str(),  "Set task on pause." );
	printf(common_format, (CONTINUE_TASK_CMD + " [task ID]" ).c_str(),  "Resume task." );
	printf(common_format, (INFO_CMD + " [task ID]" ).c_str(),  "Information about one task by ID" );
#ifdef TEST_MODE
	printf(common_format, TEST_THREADS_CMD.c_str(),  "Start 1000 threads." );
#endif
	printf(common_format, INFO_CMD.c_str(),  "Information about all tasks." );
	printf(common_format, EXIT_CMD.c_str(),  "Stop all tasks and exit" );
	printf("\n" );
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

	Task_shr_p task;
	switch (type_of_prog) {
		case TaskTypes::SIMPLE:
			task = std::make_shared<TaskT>(g_task_count, _delay);
			break;
		case TaskTypes::ASYNC_PROGRS:
			task = std::make_shared<TaskAsyncProgress>(g_task_count, _delay);
			break;
		case TaskTypes::INFINITY:
			task = std::make_shared<InfinityTask>(g_task_count, _delay);
			break;
		default:
			task = std::make_shared<TaskT>(g_task_count, _delay);
			break;
	}

	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list[g_task_count] = task;
	std::cout << "Task #" << g_task_count << " started." << std::endl;
	g_task_count++;
	return ret;
}

int TaskPool::stop_all()
{
	std::unique_lock lock(g_task_list_mutex);
	// not in one cycle 'cause stopping task some time
	for(const auto& it: g_task_list)
	{
		Task_shr_p t = it.second;
		t->stop();
	}
	for(const auto& it: g_task_list)
	{
		Task_shr_p t = it.second;
		t->cur_thread.join();
	}
	return 0;
}

int TaskPool::get_all_task_info() const
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
	int ret = 0;
	clean_tasks_pool();
	std::stringstream ss(cmd);
	std::istream_iterator<std::string> begin_s(ss);
	std::istream_iterator<std::string> end_s;
	std::vector<std::string> commands(begin_s, end_s);
	size_t cmdl_len = commands.size();
	if (cmdl_len == 0) {
		print_help(WRONG_FMT);
		return -1;
	}
	std::string cmd_type = commands[0];

	try {
		if (cmd_type == START_TASK_CMD) {
			int delay = 0;
			TaskTypes type_of_task = TaskTypes::SIMPLE;
			if (cmdl_len == 2)
			{
				if (is_number(commands[1]))
					delay = std::stoi(commands[1]);
				else
					type_of_task = str_to_task(commands[1]);
			}
			if (cmdl_len >= 3)
			{
				type_of_task = str_to_task(commands[1]);
				delay = std::stoi(commands[2]);
			}
			start_task(delay, type_of_task);
		}
		else if (cmd_type == INFO_CMD)
		{
			if (cmdl_len == 1)
				get_all_task_info();
			else
				operation_manager(std::stoi(commands[1]), OperationCode::INFO);
		}
		else if (cmd_type == PAUSE_TASK_CMD)
		{
			operation_manager(std::stoi(commands[1]), OperationCode::PAUSE);
		}
		else if (cmd_type == CONTINUE_TASK_CMD)
		{
			operation_manager(std::stoi(commands[1]), OperationCode::CONTINUE);
		}
		else if (cmd_type == STOP_TASK_CMD)
		{
			operation_manager(std::stoi(commands[1]), OperationCode::STOP);
		}
		else if (cmd_type == EXIT_CMD)
		{
			stop_all();
			return 1;
		}
#ifdef TEST_MODE
		else if (cmd_type == TEST_THREADS_CMD)
		{
			for(int i = 0 ; i < 1000 ; i++)
				start_task(0);
		}
#endif
		else
		{
			print_help(1);
			ret = 0;
		}
	}
	catch (const std::exception & e) {
		print_help(1);
		ret = -1;
	}

	return ret;
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

int TaskPool::main_loop()
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