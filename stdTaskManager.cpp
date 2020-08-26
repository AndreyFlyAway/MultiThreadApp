#include <cstdarg>
#include "stdTaskManager.h"
#include "TaskTypes/PyramidSort.h"

using Task_shr_p = std::shared_ptr<TaskT>;

/* command constants*/
static const std::string EXIT_CMD = "quit";
static const std::string START_TASK_CMD = "start";
static const std::string INFO_CMD = "info";
static const std::string PAUSE_TASK_CMD = "pause";
static const std::string CONTINUE_TASK_CMD = "resume";
static const std::string STOP_TASK_CMD = "stop";
static const std::string GET_RESULTS_CMD = "results";
static const std::string TEST_THREADS_CMD = "start_1000";
/* names of task types */
static const std::string SIMPLE_T_CMD = "simple";
static const std::string ASYN_PORGRESS_T_CMD = "asyn_prog";
static const std::string INF_T_CMD = "inf";
static const std::string PYRAMID_SORT_T_CMD = "pyramid";


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

	const char common_format[] = "\t%-50s\t%s\n";
	printf( "Commands:\n");
	printf(common_format, START_TASK_CMD.c_str(),  "Start without delay. By default simple task will be created." );
	printf(common_format, (START_TASK_CMD + " [task type] [delay] [args] ").c_str(),  "Start task with specified type and delay. Some of tasks requires arguments. Types of tasks and below." );
	printf(common_format, (STOP_TASK_CMD + " [task ID]" ).c_str(),  "Stop task by ID." );
	printf(common_format, (PAUSE_TASK_CMD + " [task ID]" ).c_str(),  "Set task on pause." );
	printf(common_format, (CONTINUE_TASK_CMD + " [task ID]" ).c_str(),  "Resume task." );
	printf(common_format, (INFO_CMD + " [task ID]" ).c_str(),  "Information about one task by ID" );
	printf(common_format, INFO_CMD.c_str(),  "Information about all tasks." );
	printf(common_format, EXIT_CMD.c_str(),  "Stop all tasks and exit" );
	printf(common_format, (GET_RESULTS_CMD + " [task ID]" ).c_str(), "Get results for task. If [task ID] is not set then all results will be printed.");
#ifdef TEST_MODE
	printf(common_format, TEST_THREADS_CMD.c_str(),  "Start 1000 task of type simple." );
#endif
	printf("\n" );
	printf( "Types of tasks:\n");
	printf(common_format, SIMPLE_T_CMD.c_str(),  "Simple task that increase progress during 10 seconds." );
	printf(common_format, ASYN_PORGRESS_T_CMD.c_str(),  "Task that increase progress during 10 seconds asynchronously. Work two threads");
	printf(common_format, INF_T_CMD.c_str(),  "Tasks that works infinitely.");
	printf(common_format, PYRAMID_SORT_T_CMD.c_str(),  "Task that sort strings from file using pyramid sort. Results are saved in file.");
	printf(common_format, "",  "This task requires two arguments - path to file with data and path to file where results will be saved");
	printf("\n" );
	printf("Examples of starting commands:\n");
	printf(common_format, "start simple 0",  "Start simple task with no delay");
	printf(common_format, "start asyn_prog 12",  "Start task with asynchronous increase progress with no delay equals 12 seconds.");
	printf(common_format, "start pyramid 0 ./test_data.txt ./results.txt",  "Start task pyramid sorting task. Files are placed in current folder.");
	printf("\n" );
}

int TaskPool::start_task(const std::vector<std::string>& args)
{
	// TODO: make warning if amount of tasks is over hardware_concurrency - 1
	int ret = 0;
	static uint g_task_count = 1;
	int arg_len = args.size();
	std::string task_type = args[1];
	int _delay = std::stoi(args[2]);
	if (_delay < 0)
	{
		std::cout << "ERROR: delay cant be negative! Started without delay. " << std::endl;
		_delay = 0;
	}

	Task_shr_p task;
	if (task_type == SIMPLE_T_CMD)
		task = std::make_shared<TaskT>(g_task_count, _delay);
	else if (task_type == ASYN_PORGRESS_T_CMD)
		task = std::make_shared<TaskAsyncProgress>(g_task_count, _delay);
	else if (task_type == INF_T_CMD)
		task = std::make_shared<InfinityTask>(g_task_count, _delay);
	else if (task_type == PYRAMID_SORT_T_CMD)
		task = std::make_shared<PyramidSortTask>(g_task_count, _delay, args[3], args[4]);
	else
		throw WrongTaskArgsException();

	// TODO: make preventing starting tasks if them exist more than some value
	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list[g_task_count] = task;
	std::cout << "Task #" << g_task_count << " started." << std::endl;
	g_task_count++;
	// if once amount of task will be more than uint max
	if (g_task_count == 0)
		g_task_count = 1;
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
			start_task(commands);
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
		else if (cmd_type == GET_RESULTS_CMD)
		{
			if (cmdl_len == 1)
				get_result(0);
			else
				get_result(std::stoi(commands[1]));
		}
		else if (cmd_type == EXIT_CMD)
		{
			stop_all();
			return 1;
		}
#ifdef TEST_MODE
		else if (cmd_type == TEST_THREADS_CMD)
		{
			std::vector<std::string> v{"start"};
			for(int i = 0 ; i < 1000 ; i++)
				start_task(v);
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

int TaskPool::get_result(uint task_id)
{
	if (results.empty())
		std::cout << "No results yet" << std::endl;
	if (task_id == 0)
	{
		for (const auto &it: results)
		{
			std::cout << "Task #" << it.first << ": " << it.second << std::endl;
		}
	}
	else
	{
		auto it = results.find(task_id);
		if (it != results.end())
			std::cout << "Task #" << it->first << ": " << it->second << std::endl;
		else
			std::cout << "No results for task # " << task_id << std::endl;
	}
	return 0;
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
			results[it.first] = t->get_results();
			// TODO: think about it, make size of this buffer customizable
			if (results.size() >= 30)
			{
				auto eraseIter = results.begin();
//				std::advance(eraseIter, 10)
				results.erase(results.begin());
			}
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