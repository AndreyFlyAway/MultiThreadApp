//
// Created by user on 12.04.20.
//
// TODO: заменить весб стандартный вывод на log4cplus??
#include "MultiThreadApp.h"

using namespace std;

/* command constants*/
const string EXIT_CMD = "q";
const string START_TASK_CMD = "s";
const string INFO_CMD = "info";

/* other */
const int TASK_WORKS = 1;
const int TASK_ENDED = 0;

pthread_mutex_t g_task_pull_mutex;
map<pthread_t, int> g_task_pull;
map<pthread_t, int>::iterator g_it; // использую для поиска элемента в g_task_pull, создал тут, чтобы не создавать каждый раз в стеке

/* @brief простая функция для потока */
static void *simple_thread(void *){
    pthread_t task_id = pthread_self();
    for (int i = 0; i < 30 ; i++)
    {
        //cout << "simple_thread " << i << endl;
        usleep(500000);
    }
    cout << "Taske id " << task_id << " ends work" << endl;
}

/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(string cmd)
{
    pthread_t t1;

    /* разделяю по словам */
    stringstream ss(cmd);
    istream_iterator<std::string> begin_s(ss);
    istream_iterator<std::string> end_s;
    vector<string> commands(begin_s, end_s);

    if (commands.size() < 1){
        //TODO: print_help()
        return -1;
    }
    if (commands[0] == EXIT_CMD){
        cout << "End work bye! " << endl;
        // TODO: тут должен быть вызов остановки всех потоков
        return 1;
    }
    else if (commands[0] == START_TASK_CMD) // старт задачи
    {
        pthread_create(&t1,  NULL, simple_thread, (void*)NULL);
        pthread_mutex_lock(&g_task_pull_mutex);
        g_task_pull[t1] = TASK_WORKS;
        pthread_mutex_unlock(&g_task_pull_mutex);
        cout << "task id " << t1 << " started" << endl;
    }
    else if (commands[0] == INFO_CMD) // вызов информации о задаче
    {
        if (commands.size() != 2)
        {
            cout << "Wrong command wormat" << endl;
            //TODO: print_help()
        }
        pthread_mutex_lock(&g_task_pull_mutex);
        if (g_it != g_task_pull.end())
        {
//            usinged int = (commands[1])
//            cout << "task id " << g_task_pull[(] << " in process" << endl;
        }
        else
        {

        }
        pthread_mutex_unlock(&g_task_pull_mutex);
    }
    else
    {
        cout << "Unregonized command " << cmd << endl;
        //TODO: print_help()
    }
    return 0;
}

/* @brief  точка входа в стек */
int multi_hread_main()
{
    int res;
    string cmd;
    bool exit_f = false;
    pthread_t mange_cmd;
    while (!exit_f)
    {
        getline(cin, cmd);
        if ((res=task_mannger(cmd)) == 1){
            break;
        }
    }
    return res;
}