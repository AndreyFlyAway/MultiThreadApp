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
const int TASK_ENDED = 0;
const int TASK_WORKS = 1;
const int TASK_WAITING = 2;

uint g_task_coun = 1;
/* структура, для хранения данных о задаче*/
struct task_t {
    pthread_t pt_id; // для использоованя с API pthread
    uint task_id ; // назначется вручную через глобальный счетсчик g_task_coun
    uint delay_sec; // задержка запуска задачи
    int progress; // прогресс задачи
    int status; // код статуса 1 - запущена, 2 - в ожидании
    // void task_func();
};

pthread_mutex_t g_task_pull_mutex;
map<uint, task_t*> g_task_pull;
// TODO: заменить на hash-таблиу?
map<uint, task_t*>::iterator g_it; // использую для поиска элемента в g_task_pull, создал тут, чтобы не создавать каждый раз в стеке

/* вспомогательные функции */
/* из-за частых вызовов мьютекса код стал перегруженным, поэтому отдельные действия вынес в отдельные функции */
/*
 * @brief установдение статуса работы задачи в зависимости от наличия задержки и возвращает величину задержки
 * @return величина задержки
 */
static task_t get_task_struct(task_t *tsk){
    task_t res;
    pthread_mutex_lock(&g_task_pull_mutex);
    res = *tsk;
    pthread_mutex_unlock(&g_task_pull_mutex);
    return res;
}
/*
 * @brief установдение статуса работы задачи в зависимости от наличия задержки и возвращает величину задержки
 * @return величина задержки
 */
static int set_task_status(task_t *tsk){
    int delay;
    pthread_mutex_lock(&g_task_pull_mutex);
    delay = tsk->delay_sec;
    if (delay != 0)
        tsk->status = TASK_WAITING;
    else
        tsk->status = TASK_WORKS;
    pthread_mutex_unlock(&g_task_pull_mutex);
    return delay;
}

/*
 * @brief установка процента выполнения
 * @return количесетво установленных процентов
 */
static int add_percentage(task_t *tsk, int percent){
    int ret;
    pthread_mutex_lock(&g_task_pull_mutex);
    tsk->progress += percent;
    if (tsk->progress > 99)
    {
        tsk->progress = 99;
    }
    ret = tsk->progress;
    pthread_mutex_unlock(&g_task_pull_mutex);
    return ret;
}

/* @brief проверка являеться ли строка числом
 * @return булевое значение */
static bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

static task_t get_struct(const string& s){

}

/* @brief простая функция для потока */
static void *simple_thread(void *args){
    int progress = 0;
    int delay = 0;
    int task_id = 0;
    task_t *task_info = (task_t*)args;

    // TODO: слишком много захватов мьютекса
    delay = set_task_status(task_info);

    // TODO: сделать задержку с использованием таймеров и слотов
//    if (delay != 0){
//        usleep(delay * 1000000);
//    }
    printf("task id %u started\n", task_id);

    /* основная работа */
    for (int i = 0; i < 30 ; i++)
    {
        //cout << "simple_thread " << i << endl;
        usleep(500000);
        add_percentage(task_info, 5);
    }
    /* завершение */
    printf("Taske id %u ends work\n", task_info->task_id);
    // удаляю элемент из пула задач
    // TODO: вынести это дело за пределы поточной функции... или нет?
    pthread_mutex_lock(&g_task_pull_mutex);
    g_it=g_task_pull.find(task_info->task_id);
    if (g_it != g_task_pull.end())
    {
        g_task_pull.erase (g_it);
    }
    else
    {
        printf("Some error caused. Cant erase task with id %u\n", task_info->task_id);
    }
    pthread_mutex_unlock(&g_task_pull_mutex);
    delete task_info;
}

/* @brief вызов информации о задаче
 * @return 0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач),
 * s-2 если неверный формат команды
 * */
int print_task_info(std::vector<std::string> data)
{
    task_t *task_info;
    uint task_id;
    if (data.size() == 1) // вывод инофрмвции по всем задачам
    {
        pthread_mutex_lock(&g_task_pull_mutex);
        for (g_it=g_task_pull.begin(); g_it!=g_task_pull.end(); ++g_it)
        {
            task_info =  g_it->second;
            if (task_info->status == TASK_WORKS)
                printf("Task id %u in progress: %d%\n", task_info->task_id, task_info->progress);
            else if(task_info->status == TASK_WAITING)
                printf("Task id %u is waiting. \n", task_info->task_id);
        }
        pthread_mutex_unlock(&g_task_pull_mutex);
    }
    else if (data.size() == 2) // вывод инофрмвции по конкрентно одной задаче
    {
        if (is_number(data[1]))
        {
            task_id = stoi(data[1]);
            pthread_mutex_lock(&g_task_pull_mutex);
            g_it=g_task_pull.find(task_id);
            if (g_it != g_task_pull.end())
            {
                task_info = (g_task_pull[task_id]);
                if (task_info->status == TASK_WORKS)
                    printf("Task id %u in progress: %d%\n", task_info->task_id, task_info->progress);
                else if(task_info->status == TASK_WAITING)
                    printf("Task id %u is waiting. Time until start: %d second\n", task_info->task_id, task_info->delay_sec);
            }
            else{
                cout << "There is no task with task id " << task_id << endl;
            }
            pthread_mutex_unlock(&g_task_pull_mutex);
        }
        else{
            return -2;
        }
    }else{
        return -2;
    }

}

/* @brief запуск задачи
 * @return 0 если все ок, -1 если не удалось запустить задачу, -2 если неверный формат команды
 * */
int start_task(std::vector<std::string> data){
    // TODO: сделать вывод предупреждения, если запущенных задач стало больше 1000, например
    pthread_t t1;
    int arg_len = data.size();
    int delay;
    task_t *task_info = new task_t;

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

    pthread_mutex_lock(&g_task_pull_mutex);
    task_info->delay_sec = delay;
    task_info->task_id = g_task_coun;
    task_info->pt_id = t1;
    // кладу данные в контейнер
    g_task_pull[g_task_coun] = task_info;
    // создаю задачу
    pthread_create(&t1, NULL, simple_thread,  (void*)task_info);
    g_task_coun++;
    pthread_mutex_unlock(&g_task_pull_mutex);

}


/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(string cmd)
{
    /* разделяю по словам */
    int res;
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
        res = start_task(commands) ;
        if (res == -2){
            printf("Worong command format!\n");
            //TODO: print_help()
        }

//        // TODO: сделать вывод предупреждения, если запущенных задач стало больше 1000, например
//        task_info = new task_t;
//        task_info->task_id = g_task_coun;
//        task_info->pt_id = t1;
//        // кладу данные в контейнер
//        pthread_mutex_lock(&g_task_pull_mutex);
//        g_task_pull[g_task_coun] = task_info;
//        // создаю задачу
//        pthread_mutex_unlock(&g_task_pull_mutex);
//        cout << "task id " << g_task_coun << " started" << endl;
//        pthread_create(&t1, NULL, simple_thread,  (void*)task_info);
//        g_task_coun++;

    }
    else if (commands[0] == INFO_CMD) // вызов информации о задаче
    {
        res = print_task_info(commands);
        if (res == -2){
            printf("Worong command format!\n");
            //TODO: print_help()
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
    //TODO: print_help()
    while (!exit_f) // TODO: ???
    {
        getline(cin, cmd);
        if ((res=task_mannger(cmd)) == 1){
            exit_f = true;
            break;
        }
    }
    return res;
}