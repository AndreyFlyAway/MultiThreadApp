//
// Created by user on 22.04.20.
//

#ifndef MULTITHREADAPP_STDTASKMANAGER_H
#define MULTITHREADAPP_STDTASKMANAGER_H

#include "Task_t.h"

/* @brief printing help /вывод справки об использовании
 * @param wrong_fmt: used if with simple help need print "Wrong command format!"
 *                   используеться, если нужно вывести Wrong command format!"*/
void print_help(int wrong_fmt=0);

/* @brief checking if word is a number / проверка являеться ли число строкой
 * @return boolean true/false / булево значение */
bool is_number(const std::string& s);

/* @brief starting task / запуск задачи
* @return 0 if everything is OK, -1 cant start task, -2 wrong command format
 *        0 если все ок, -1 если не удалось запустить задачу, -2 если неверный формат команды
* */
int start_task(std::vector<std::string> data);


/* @brief stop task / остановка задачи
* @return 0 if everything is OK, -1 cant stop task for some reason, -2 wrong task id
 *        0 если все ок, -1 если е удалось звершить задачу по какой-либо причине, -2 если неверный task id
* */
int stop_task(uint task_id);


/* @brief printing info about task / вывод информации о задаче
 * @return 0 if OK, -1 cant take info about task (there is no task with this task id in task pull)
 *         0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач),
 * */
int get_task_info(std::vector<std::string> data);

/* @brief  command manager / обработчик консольных сообщений
 * @return 0 if all is OK, 1 if task in exit task status, -1 if something bad happened
 *         0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(std::string cmd);

/* @brief  thread function that delete task pointer from task list if thread finish its work.
 *         This function use global condition variable
 *         потоковая функция, которая удаляет ссылку на задачу из списка задач, если та завершила
 *         свою работу. Функция использует глобальный переменную состояния
 * @return 0 if all is OK, -1 if task doesn't have ending status
 *         0 если все ок, 1 если задача не имеет статус "в завершении"
 * */
int garbage_thread();

/* @brief точка входа / entry point*/
int std_multi_hread_main();

#endif //MULTITHREADAPP_STDTASKMANAGER_H
