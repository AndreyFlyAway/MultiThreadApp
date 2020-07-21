//
// Created by user on 22.04.20.
//

#pragma once

#include "Task_t.h"
#include <shared_mutex>

class TaskPool
{
public:
	/* @brief точка входа / entry point*/
	int std_multi_hread_main();
protected:
	// global list with tasks
	std::shared_mutex g_task_list_mutex;
	std::map<uint, std::shared_ptr<Task_t>> g_task_list;

	/* @brief printing help /вывод справки об использовании
	 * @param wrong_fmt add string "wrong format" if wrong_fmt == 1 /
	 *                  добавить сообщение "неверный формат" если установлено значение 1
	 * @param wrong_fmt: used if with simple help need print "Wrong command format!"
	 *                   используеться, если нужно вывести Wrong command format!"*/
	void print_help(int wrong_fmt=0) const;

	/* @brief starting task / запуск задачи
	 * @param delay delay start / задержка запуска
	 * @return 0 if everything is OK, -1 cant start task
	 *        0 если все ок, -1 если не удалось запустить задачу
	* */
	int start_task(int delay);

	/* @brief stop task / остановка задачи
	 * @param task_id task id / id задачи
	 * @return 0 if everything is OK, -1 cant stop task for some reason, -2 wrong task id
	 *        0 если все ок, -1 если е удалось звершить задачу по какой-либо причине, -2 если неверный task id
	* */
	int stop_task(uint task_id);

	/* @brief printing info about single task or all tasks / вывод информации об одной задаче
	 * @param task_id task id or if task_id equals 0 than return info about all task
	 * 		  id задачи или елси этот праметр равен 0, то выводится информация по всем параметрам
	 * @return 0 if OK, -1 cant take info about task (there is no task with this task id in task pull)
	 *         0 если все ок, -1 если не удалось получит данные по задаче (задача не находится в пулле задач),
	 * */
	int get_task_info(uint task_id);

	/* @brief set task on pause
	 * @param task_id task / id задачи
	 * @return 0 if OK, -1 if task has pause status, -2 there are no task with this task id
	 *         0 если все ок, -1 есди задача на уже паузе, , -2 если нет задачи с таким id
	 * */
	int pause_task(uint task_id);

	/* @brief resume task from pause
	 * @param task_id task / id задачи
	 * @return 0 if OK, -1 if task doesn't have pause status, -2 there are no task with this task id
	 *         0 если все ок, -1 есди задача на уже паузе, -2 если нет задачи с таким id
	 * */
	int resume_task(uint task_id);

	/* @brief  command manager / обработчик консольных сообщений
	 * @param task_id task id / id задачи
	 * @return 0 if all is OK, 1 if command to exit with was executed, -1 if something bad happened
	 *         0 если все ок, 1 если пришла команда завершения, -1 если все плохо
	 * */
	int task_manager(const std::string cmd);

	/* @brief  function for wrapping task, this function delete task from task list when it ends
	 *         his work
	 *         функция для оберкти задачи, эта функция удаляет задачу из пула задача, когда та
	 *         завершиь работу
	 * @param task reference to task / ссылка на задачу
	 * @param task_id just task id, this parameter is used to avoid getting this values from task - no need
	 *        to use mutex
	 *        просто id задачи, используеться для избежания получения этого значния из здачаи - не нужно
	 *        использовать мьютекс
	 * @return
	 * */
	void thread_wrapper(const std::shared_ptr<Task_t> task, uint task_id);
};

