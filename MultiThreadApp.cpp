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
const string PAUSE_TASK = "p";
const string CONTINUE_TASK = "c";

/* other */
/* статусы для задач  */
const int TASK_ENDING   = 0;  // предполагаеться что этот статус будет использоваться, если будет паралельно вместе с завершением задачи будет запущен процесс остановки задачи
const int TASK_WORKS   = 1;
const int TASK_WAITING = 2;
const int TASK_PAUSE = 3;

uint g_task_coun = 1;

/* см. функцию print_help */
const int WRONG_FMT = 1;//
const int UNREC_CMD = 2;//


/* для функции set_pause_state */
const bool PAUSE_STATE = false;
const bool CONTINUE_STATE = true;

pthread_mutex_t g_task_pull_mutex;
map<uint, task_t*> g_task_pull;
// TODO: заменить на hash-таблиу?
/* вспомогательные функции */
/* из-за частых вызовов мьютекса код стал перегруженным, поэтому отдельные действия вынес в отдельные функции */
/*
 * @brief копирование данных структуры из пула задач по id задачи
 * @return 0 если задачи есть в пуле задач, 1 если нету
 */
int get_task_by_id(uint task_id, task_t &trgt){
    int res = 0;
    pthread_mutex_lock(&g_task_pull_mutex);
    map<uint, task_t*>::iterator  it=g_task_pull.find(task_id);
    if (it != g_task_pull.end())
    {
        trgt = *(g_task_pull[task_id]);
        res = 0;
    }
    else{
        res = 1;
    }
    pthread_mutex_unlock(&g_task_pull_mutex);
    return res;
}

/*
 * @brief копирование данных конктретного элемента по ссылке
 * @return возвращает значение структру
 */
task_t get_task_by_ref(task_t *trgt){
    pthread_mutex_lock(&(trgt->obj_mutex));
    task_t ret = *trgt;
    pthread_mutex_unlock(&(trgt->obj_mutex));
    return ret;
}

/*
 * @brief установдение статуса работы задачи в зависимости от наличия задержки и возвращает величину задержки
 * @return величина задержки
 */
void set_task_status(task_t *tsk, int status){
    pthread_mutex_lock(&(tsk->obj_mutex));
    tsk->status = status;
    tsk->in_proccess = false;
    pthread_mutex_unlock(&(tsk->obj_mutex));
}

/*
 * @brief установка процента выполнения
 * @return количесетво установленных процентов
 */
int add_percentage(task_t *tsk, int percent){
    tsk->in_proccess = true;
    pthread_mutex_lock(&tsk->obj_mutex);
    tsk->progress += percent;
    if (tsk->progress > 99)
    {
        tsk->progress = 99;
    }
    int ret = tsk->progress;
    pthread_mutex_unlock(&tsk->obj_mutex);
    return ret;
}

/* @brief проверка являеться ли строка числом
 * @return булевое значение */
bool is_number(const string& s)
{
    string::const_iterator it = s.begin();
    while (it != s.end() && isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

/* @brief простая функция для потока */
void *simple_thread(void *args){
    // TODO: слишком много захватов мьютекса
    // TODO: с сылкой на объект происходит работа, хотя в этот момент она находиться в пуле задачь, так же в этот
    //       момент ее уже могут удалить - это неверно. Тут нужно использовать мьютексы для конкретных данных и флаг
    //       занятоти объекта.
    task_t *task_info_p = (task_t*)args;
    pthread_mutex_lock(&task_info_p->obj_mutex);
    task_info_p->in_proccess = false;
    pthread_mutex_unlock(&task_info_p->obj_mutex);
    task_t task_info_v = get_task_by_ref((task_t*)args);
    uint delay = task_info_v.delay_sec;

    if (delay != 0){
        pthread_mutex_lock(&task_info_p->obj_mutex);
        time(&(task_info_p->time_started)); // чтобы вычислять сколько осталось до старта задачи
        pthread_mutex_unlock(&task_info_p->obj_mutex);
        set_task_status(task_info_p, TASK_WAITING);
        usleep(delay * 1000000);
    }
    set_task_status(task_info_p, TASK_WORKS);
    printf("task id %u started\n", task_info_v.task_id);

    /* основная работа */
    for (int i = 0; i < 30 ; i++)
    {
        // обспечение паузы
        pthread_mutex_lock(&task_info_p->pause_flag_mutex);
        while(!(task_info_p->pause_flag)){
            pthread_cond_wait(&task_info_p->thread_cond, &task_info_p->pause_flag_mutex);
        }
        pthread_mutex_unlock(&task_info_p->pause_flag_mutex);
        /* работа в цикле */
        //cout << "simple_thread " << i << endl;
        usleep(500000);
        add_percentage(task_info_p, 5);
    }
    /* завершение */
    printf("KEKEKE\n");
    set_task_status(task_info_p, TASK_ENDING);
    // удаляю элемент из пула задач
    pthread_mutex_lock(&g_task_pull_mutex);
    g_task_pull.erase (task_info_v.task_id);
    pthread_mutex_unlock(&g_task_pull_mutex);
    printf("Task id %u ends work\n", task_info_v.task_id);
    // TODO: как корректно удалять объект, ведь его могут в этот момент тоже использовать?
    delete task_info_p;
}

/* @brief вывод информации о задаче, не предполагается, что эта функция должна работать напрямую с объектами,
 * к которым может быть одновременный доступ, поэтому мьютексы не используются */
void print_task_info(task_t *tsk){
    int stat = tsk->status;
    int rest_time;
    time_t now;
    switch (stat)
    {
        case TASK_WORKS:
            printf("Task id %u in progress: %d%\n", tsk->task_id, tsk->progress);
            break;
        case TASK_WAITING:
            time(&now) ;
            rest_time = tsk->delay_sec - (int)difftime(now, tsk->time_started);
            printf("Task id %u is waiting. Time until start: %d seconds\n", tsk->task_id, rest_time);
            break;
        case TASK_PAUSE:
            printf("Task id %u in pause. Current progress %d%\n", tsk->task_id, tsk->progress);
            break;
    }
}

/* @brief вызов информации о задаче
 * @return 0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач),
 * s-2 если неверный формат команды
 * */
int get_task_info(std::vector<std::string> data)
{
    task_t task_info_v;
    task_t *task_info_p;

    if (data.size() == 1) // вывод инофрмвции по всем задачам
    {
        // FIXME: слишком на долго запираеться пул задач
        pthread_mutex_lock(&g_task_pull_mutex);
        for (map<uint, task_t*>::iterator it=g_task_pull.begin(); it!=g_task_pull.end(); ++it)
        {
            task_info_p =  it->second;
            print_task_info(task_info_p);
        }
        pthread_mutex_unlock(&g_task_pull_mutex);
    }
    else if (data.size() == 2) // вывод инофрмвции по конкрентно одной задаче
    {
        if (is_number(data[1]))
        {
            uint task_id = uint(stoi(data[1]));
            int res = get_task_by_id(task_id, task_info_v);
            if (res == 0)
            {
                print_task_info(&task_info_v);
            }
            else{
                printf("There is no task with task id %u\n", task_id);
            }
        }
        else{
            return -2;
        }
    }else{
        return -2;
    }
    return 0;

}

/* @brief вывод справки об использовании
 * @param wrong_fmt: используеться, если нужно вывести Wrong command format!"*/
void print_help(int wrong_fmt){
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

/* @brief используеться для приостановки (на паузу) или запуска задачи
 * @return код результата:
 * 0 - все ок
 * 1 - нет задачи с данным id
 * 2 - статус задачи совпадает с тем, что уже имеется, т.е. например если задача ставиться на паузу, а она уже
 *     на паузе
 * 3 - задача в процессе завершения
 * */
int set_pause_state(uint task_id, bool state){
    int res = 0;
    task_t *task_info_p;
    pthread_mutex_lock(&g_task_pull_mutex);
    map<uint, task_t*>::iterator it = g_task_pull.find(task_id);
    if (it != g_task_pull.end())
    {
        task_info_p = g_task_pull[task_id];
    }
    else{
        pthread_mutex_unlock(&g_task_pull_mutex);
        return 1;
    }
    pthread_mutex_unlock(&g_task_pull_mutex);

    // TODO: использовать trylock??
    pthread_mutex_lock(&task_info_p->obj_mutex);
    int status = task_info_p->status;
    if ((status == TASK_WORKS) || (status == TASK_WAITING))
    {
        task_info_p->pause_flag = state;
        pthread_cond_signal(&task_info_p->thread_cond);
    }
    else if ( ((status == TASK_PAUSE) && (state == PAUSE_STATE)) ||
              ((status == TASK_WAITING) && (state == CONTINUE_STATE))  )
    {
        res = 2;
    }
    else if (status == TASK_ENDING)
    {
        res = 3;
    }
    if (state == PAUSE_STATE)
        task_info_p->status = TASK_PAUSE;
    if (state == CONTINUE_STATE)
        task_info_p->status = TASK_WORKS;
    pthread_mutex_unlock(&task_info_p->obj_mutex);
    return res;
}

/* @brief запуск задачи
 * @return 0 если все ок, -1 если не удалось запустить задачу, -2 если неверный формат команды
 * */
int start_task(std::vector<std::string> data){
    // TODO: сделать вывод предупреждения, если запущенных задач стало больше 1000, например
    pthread_t t1;
    int delay = 0;
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
    task_info->delay_sec = delay;
    task_info->task_id = g_task_coun;
    task_info->pt_id = t1;
    task_info->in_proccess = true;

    // кладу данные в контейнер
    pthread_mutex_lock(&g_task_pull_mutex);
    g_task_pull[g_task_coun] = task_info;
    pthread_mutex_unlock(&g_task_pull_mutex);
    // создаю задачу
    pthread_create(&t1, NULL, simple_thread,  (void*)task_info);
    g_task_coun++;

}

/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(string cmd)
{
    /* разделяю строку по словам */
    int res = 0;
    stringstream ss(cmd);
    istream_iterator<std::string> begin_s(ss);
    istream_iterator<std::string> end_s;
    vector<string> commands(begin_s, end_s);

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
        // TODO: сделать вывод предупреждения, если запущенных задач стало больше 1000, например
        res = start_task(commands) ;
        if (res == -2){
            print_help(WRONG_FMT);
        }
    }
    else if (commands[0] == INFO_CMD) // вызов информации о задаче
    {
        res = get_task_info(commands);
        if (res == -2){
            print_help(WRONG_FMT);
        }
    }
    else if (commands[0] == PAUSE_TASK) // поставить задачу на паузу
    {
        uint task_id = uint(stoi(commands[1]));
        set_pause_state(task_id, PAUSE_STATE);
    }
    else if (commands[0] == CONTINUE_TASK) // поставить задачу на паузу
    {
        uint task_id = uint(stoi(commands[1]));
        set_pause_state(task_id, CONTINUE_STATE);
    }
    else
    {
        print_help(UNREC_CMD);
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
    print_help();
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