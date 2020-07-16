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

void TaskPool::print_help(int wrong_fmt){
    switch (wrong_fmt){
        case WRONG_FMT:
            printf("Wrong command format!\n");
            break;
        case UNREC_CMD:
            printf("Unrecognized command.\n");
            break;
    }
    printf("Start task command format without delay:\n"
           "  %s now\n"
           "Start task command format with delay\n"
           "  %s [time]\n"
           "Information about all tasks:\n"
           "  %s\n"
           "Information about all one task by ID:"
           "  %s [task ID]\n"
           "Enter '%s' to quit\n",
           START_TASK_CMD.c_str(),
           START_TASK_CMD.c_str(),
           INFO_CMD.c_str(),
           INFO_CMD.c_str(),
           EXIT_CMD.c_str());
}

int TaskPool::start_task(std::vector<std::string> data)
{
    // TODO: make warning if amount of tasks is over 1000 (for example) / сделать вывод предупреждения, если запущенных задач стало больше 1000, например
    static uint g_task_count = 1;
    int ret = 0;
    int delay = 0;

    if (data.size() != 2)
        return -2;

    if (data[1] == "now"){
        delay = 0;
    }
    else{
        if (is_number(data[1]))
            delay = stoi(data[1]);
        else
            return -2;
    }

	std::shared_ptr<Task_t> task = std::make_shared<Task_t>(g_task_count, delay);
	std::thread t(&TaskPool::thread_wrapper, this, task, g_task_count);
	t.detach();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list[g_task_count] = task;
	g_task_count++;
	return ret;
}

void TaskPool::thread_wrapper(std::shared_ptr<Task_t> task, uint task_id)
{
	task->run();
	std::unique_lock lock(g_task_list_mutex);
	g_task_list.erase (task_id);
}

int TaskPool::get_task_info(uint task_id)
{
	int res = 0;
	std::shared_lock lock(g_task_list_mutex);
	std::shared_ptr<Task_t> task = g_task_list[task_id];
	auto it = g_task_list.find(task_id);
	if (it != g_task_list.end())
	{

	}

	printf("Tasks in pool %d.\n", g_task_list.size());
    // TODO: unlock mutex after reading data of task and print info
    return res;
}

int TaskPool::task_mannger(std::string cmd)
{
    /* разделяю строку по словам */
    std::stringstream ss(cmd);
    std::istream_iterator<std::string> begin_s(ss);
    std::istream_iterator<std::string> end_s;
    std::vector<std::string> commands(begin_s, end_s);

    if (commands.size() < 1){
        print_help(WRONG_FMT);
        return -1;
    }
    if (commands[0] == EXIT_CMD){
        printf("End work bye!\n");
        return 1;
    }
    else if (commands[0] == START_TASK_CMD) // старт задачи
    {
        int res = start_task(commands) ;
        if (res == -2){
            print_help(WRONG_FMT);
        }
    }
    else if (commands[0] == INFO_CMD) // printing info about task / вызов информации о задаче
    {
    	uint id = 0;
		if (is_number(commands[1]))
		{
			id = uint(std::stoi(commands[1]));
		}
		get_task_info(id);
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