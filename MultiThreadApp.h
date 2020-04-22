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
// TODO: релизовать флаг, который говорит о том, что со структурой ведеться работа, атомарная
// TODO: возможно есть смысл перенести in_proccess в состав атрибута status
struct task_t {
    pthread_mutex_t obj_mutex;         // используться для доступа к текущему объекту
    pthread_mutex_t pause_flag_mutex;  // используться для работы с флагом паузы
    pthread_cond_t thread_cond;        // атрбут состояния (в стд рассылке состояний не гарантирует доставки)
    pthread_t pt_id;                   // для использоованя с API pthread
    time_t time_started;               // время добавления задачи чтобы, отсчитывать и выводить время, через которое очнеться задача
    uint task_id;                      // назначется вручную через глобальный счетсчик g_task_coun
    int delay_sec;                     // задержка запуска задачи
    int progress ;                     // прогресс задачи
    int status;                        // код статуса: 0 - в процессе завершения, 1 - запущена, 2 - в ожидании
    bool in_proccess;                  // статус задачи, который говорит о том, что сейчас идет работа с текущем экзмемпдяолм задачи
    int pause_flag;                    // флаг паузы потока, atomic флаг.
    task_t()
            : pt_id(0)
            , task_id(0)
            , time_started(0)
            , delay_sec(0)
            , progress(0)
            , status(0)
            , in_proccess(false)
            , pause_flag(true)
    {
        pthread_cond_init(&thread_cond, NULL);
        pthread_mutex_init(&obj_mutex, NULL);
        pthread_mutex_init(&pause_flag_mutex, NULL);
    }
};

// TODO: прописать интрефейсы и комментарии
/* вспомогательные функции */
/* из-за частых вызовов мьютекса код стал перегруженным, поэтому отдельные действия вынес в отдельные функции */
/*
 * @brief копирование данных структуры из пула задач по id задачи
 * @return 0 если задачи есть в пуле задач, 1 если нету
 */
int get_task_by_id(uint task_id, task_t &trgt);

/*
 * @brief копирование данных конктретного элемента по ссылке
 * @return возвращает значение структру
 */
task_t get_task_by_ref(task_t &trgt);

/*
 * @brief установдение статуса работы задачи в зависимости от наличия задержки и возвращает величину задержки
 * @return величина задержки
 */
void set_task_status(task_t *tsk, int status);

/*
 * @brief установка процента выполнения
 * @return количесетво установленных процентов
 */
int add_percentage(task_t *tsk, int percent);

/* @brief проверка являеться ли строка числом
 * @return булевое значение */
bool is_number(const std::string& s);

/* @brief вывод информации о задаче, не предполагается, что эта функция должна работать напрямую с объектами,
 * к которым может быть одновременный доступ, поэтому мьютексы не используются */
void print_task_info(task_t *tsk);

/* @brief вызов информации о задаче
 * @return 0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач),
 * -2 если неверный формат команды
 * */
int get_task_info(std::vector<std::string> data);

/* @brief вывод справки об использовании
 * @param wrong_fmt: используеться, если нужно вывести Wrong command format!"*/
void print_help(int wrong_fmt=0);
/* main work */

/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(std::string cmd);

/* @brief используеться для приостановки (на паузу) или запуска задачи
 * @return код результата:
 * 0 - все ок
 * 1 - нет задачи с данным id
 * 2 - статус задачи совпадает с тем, что уже имеется, т.е. например если задача ставиться на паузу, а она уже
 *     на паузе
 * 3 - задача в процессе завершения
 * */
int set_pause_state(uint task_id, bool state);

/* @brief  точка входа */
int multi_hread_main();

#endif //MULTITHREADAPP_MULTITHREADAPP_H
