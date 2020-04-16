//
// Created by user on 12.04.20.
//

#ifndef MULTITHREADAPP_H
#define MULTITHREADAPP_H

#include <iostream>
#include <map>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

/* структура, для хранения данных о задаче*/
// TODO: релизовать мьютексы для конрктерных полей данных
// TODO: релизовать флаг, который говорит о том, что со структурой ведеться работа
// TODO: разобраться с конструктором по умолчанию
// TODO: возможно есть смысл перенести in_proccess в состав атрибута status
struct task_t {
    pthread_mutex_t obj_mutex; // используться для доступа к текущему объекту
    pthread_t pt_id;           // для использоованя с API pthread
    time_t time_started;       // время добавления задачи чтобы, отсчитывать и выводить время, через которое очнеться задача
    uint task_id;              // назначется вручную через глобальный счетсчик g_task_coun
    int delay_sec;             // задержка запуска задачи
    int progress ;             // прогресс задачи
    int status;                // код статуса: 0 - в процессе завершения, 1 - запущена, 2 - в ожидании
    bool in_proccess;          // статус задачи, который говорит о том, что сейчас идет работа с текущем экзмемпдяолм задачи
    task_t()
            : pt_id(0)
            , task_id(0)
            , time_started(0)
            , delay_sec(0)
            , progress(0)
            , status(0)
            , in_proccess(false)
    {}
};

/* common functions */
int get_task_by_id(uint task_id, task_t &trgt);
task_t get_task_by_ref(task_t &trgt);
void set_task_status(task_t *tsk, int status);
int add_percentage(task_t *tsk, int percent);
bool is_number(const std::string& s);
void print_task_info(task_t *tsk);
int get_task_info(std::vector<std::string> data);
void print_help(int wrong_fmt=0);
/*  */
int get_task_info(std::vector<std::string> data);
void get_task_info(task_t *tsk);
int task_mannger(std::string cmd);
int multi_hread_main();

#endif //MULTITHREADAPP_MULTITHREADAPP_H
