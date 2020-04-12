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

uint g_task_coun = 1;
/* структура, для хранения данных о задаче*/
struct task_t {
    uint task_id ; // назначется вручную через глобальный счетсчик g_task_coun
    pthread_t pt_id; // для использоованя с API pthread
    // void task_func();
};

pthread_mutex_t g_task_pull_mutex;
map<uint, task_t*> g_task_pull;
// TODO: заменить на hash-таблиу?
map<uint, task_t*>::iterator g_it; // использую для поиска элемента в g_task_pull, создал тут, чтобы не создавать каждый раз в стеке

/* вспомогательные функции */
/* @brief проверка являеться ли строка числом
 * @return булевое значение */
static bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

/* @brief простая функция для потока */
static void *simple_thread(void *args){
    task_t *task_info = (task_t*)args;
    /* основная работа */
    for (int i = 0; i < 30 ; i++)
    {
        //cout << "simple_thread " << i << endl;
        usleep(500000);
    }
    /* завершение */
    cout << "Taske id " << task_info->task_id << " ends work" << endl;
    // удаляю элемент из пула задач
    // TODO: вынести это дело за пределы поточной функции
    pthread_mutex_lock(&g_task_pull_mutex);
    g_it=g_task_pull.find(task_info->task_id);
    if (g_it != g_task_pull.end())
    {
        g_task_pull.erase (g_it);
    }
    else
    {
        cout << "Some error caused. Cant erase task id " << task_info->task_id << endl;
    }
    pthread_mutex_unlock(&g_task_pull_mutex);
    delete task_info;
}

/* @brief вызов информации о задаче
 * @return 0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач)
 * */
int print_task_info(uint task_id)
{
    task_t task_info;
    pthread_mutex_lock(&g_task_pull_mutex);
    g_it=g_task_pull.find(task_id);
    if (g_it != g_task_pull.end())
    {
        task_info = *(g_task_pull[task_id]);
        cout << "Task id " << task_info.task_id << " in progress." << endl;
    }
    else{
        cout << "There is no task with task id " << task_id << endl;
    }
    pthread_mutex_unlock(&g_task_pull_mutex);
}


/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(string cmd)
{
    pthread_t t1;
    task_t *task_info;
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
        cout << "End work bye!" << endl;
        // TODO: тут должен быть вызов остановки всех потоков
        return 1;
    }
    else if (commands[0] == START_TASK_CMD) // старт задачи
    {
        task_info = new task_t;
        task_info->task_id = g_task_coun;
        task_info->pt_id = t1;
        pthread_mutex_lock(&g_task_pull_mutex);
        g_task_pull[g_task_coun] = task_info;
        pthread_mutex_unlock(&g_task_pull_mutex);
        cout << "task id " << g_task_coun << " started" << endl;
        // создаю задачу
        pthread_create(&t1, NULL, simple_thread,  (void*)task_info);
        g_task_coun++;
        // кладу данные в контейнер

    }
    else if (commands[0] == INFO_CMD) // вызов информации о задаче
    {
        if (commands.size() == 2)
        {
            if (is_number(commands[1]))
            {
                print_task_info(stoi(commands[1]));
            }
            else{
                cout << "Wrong command format" << endl;
                //TODO: print_help() вместо этой секции
            }
        }else{
            cout << "Wrong command format" << endl;
            //TODO: print_help() вместо этой секции
        }
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