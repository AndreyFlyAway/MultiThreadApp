//
// Created by user on 22.04.20.
//

#pragma once

#include "TaskT.h"
#include <shared_mutex>

enum class OperationCode {
	START,
	STOP,
	INFO,
	PAUSE,
	CONTINUE
};

enum class TaskTypes {
	SIMPLE,
	ASYNC_PROGRS,
};

class TaskPool
{
public:
	/* @brief точка входа / entry point*/
	int std_multi_thread_main();
protected:
	// global list with tasks
	std::shared_mutex g_task_list_mutex;
	std::map<uint, std::shared_ptr<TaskT>> g_task_list;

	/* @brief printing help /вывод справки об использовании
	 * @param wrong_fmt add string "wrong format" if wrong_fmt == 1 /
	 *                  добавить сообщение "неверный формат" если установлено значение 1
	 * @param wrong_fmt: used if with simple help need print "Wrong command format!"
	 *                   используеться, если нужно вывести Wrong command format!"*/
	void print_help(int wrong_fmt=0) const;

	/* @brief starting task / запуск задачи
	 * @param delay delay start / задержка запуска
	 * @param type_of_prog type of task / тип задачи
	 * @return 0 if everything is OK, -1 cant start task
	 *        0 если все ок, -1 если не удалось запустить задачу
	* */
	int start_task(int delay, TaskTypes type_of_prog=TaskTypes::ASYNC_PROGRS);

	/* @brief stop task / остановка задачи
	 * @param task_id task id / id задачи
	 * @return 0 if everything is OK, -1 cant stop task for some reason, -2 wrong task id
	 *        0 если все ок, -1 если е удалось звершить задачу по какой-либо причине, -2 если неверный task id
	* */
	int stop_task(uint task_id);

	/* @brief printing info about  all tasks / вывод информации обо всех задача
	 * 		  id задачи или елси этот праметр равен 0, то выводится информация по всем параметрам
	 * @return 0 if OK, -1 if something bad happen
	 *         0 если все ок, -1 если что-то плохое произошло
	 * */
	int get_all_task_info();

	/* @brief  command manager / обработчик консольных сообщений
	 * @param task_id task id / id задачи
	 * @return 0 if all is OK, 1 if command to exit with was executed, -1 if something bad happened
	 *         0 если все ок, 1 если пришла команда завершения, -1 если все плохо
	 * */
	int task_manager(const std::string cmd);

	/* @brief  method where united operations for tasks / метод который объеденяет все операции нал задачми
	 * @param task_id task id / id задачи
	 * @param OperationCode operation code for task / код операции для задачи
	 * 		  OperationCode::STOP - stop task / остановить задачу
	 * 		  OperationCode::INFO - print info about task / вывести сообщение о задаче
	 * 		  OperationCode::PAUSE - set task on pause / поставить задачу на паузу
	 * 		  OperationCode::CONTINUE - continue task / продолжить задачу
	 * @return 0 if all is OK, -1 f there is no task with this tasks ID, -2 if something bad happened
	 *         0 если все ок, -1 нет задачи с таким task id, -2 если все плохо
	 * */
	int operation_manager(uint task_id, OperationCode op);

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
	void thread_wrapper(const std::shared_ptr<TaskT> task, uint task_id);
};
