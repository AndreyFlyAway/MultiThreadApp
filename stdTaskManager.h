#pragma once

#include <shared_mutex>
#include "TaskTypes/TaskT.h"
#include "TaskTypes/InfinityTask.h"

enum class OperationCode {
	STOP,
	INFO,
	PAUSE,
	CONTINUE
};

enum class TaskTypes {
	SIMPLE,
	ASYNC_PROGRS,
	INFINITY
};

class TaskPool
{
public:
	/* @brief точка входа / entry point*/
	TaskPool();
	int main_loop();
protected:
	// global list with tasks
	mutable std::shared_mutex g_task_list_mutex;
	std::map<uint, std::shared_ptr<TaskT>> g_task_list;
	bool exit_flag;

	/* @brief printing help /вывод справки об использовании
	 * @param wrong_fmt add string "wrong format" if wrong_fmt == 1 /
	 *                  добавить сообщение "неверный формат" если установлено значение 1
	 * @param wrong_fmt: used if with simple help need print "Wrong command format!"
	 *                   используеться, если нужно вывести Wrong command format!"
	 */
	void print_help(int wrong_fmt=0) const;

	/* @brief starting task / запуск задачи
	 * @param delay delay start / задержка запуска
	 * @param type_of_prog type of task / тип задачи
	 * @return 0 if everything is OK, -1 cant start task
	 *        0 если все ок, -1 если не удалось запустить задачу
	 */
	int start_task(int delay, TaskTypes type_of_prog=TaskTypes::SIMPLE);

	/* @brief stop all tasks / остановить все задачи
	 * @param task_id task id / id задачи
	 * @return 0 if everything is OK, -1 cant stop tasks for some reason
	 */
	int stop_all();

	/* @brief printing info about  all tasks / вывод информации обо всех задача
	 * 		  id задачи или елси этот праметр равен 0, то выводится информация по всем параметрам
	 * @return 0 if OK, -1 if something bad happen
	 *         0 если все ок, -1 если что-то плохое произошло
	 * */
	int get_all_task_info() const;

	/* @brief  command manager / обработчик консольных сообщений
	 * @param task_id task id / id задачи
	 * @return 0 if all is OK, 1 if command to exit with was executed, -1 if something bad happened
	 *         0 если все ок, 1 если пришла команда завершения, -1 если все плохо
	 * */
	int task_manager(const std::string& cmd);

	/* @brief  method where united operations for tasks / метод который объеденяет все операции нал задачми
	 * @param task_id task id / id задачи
	 * @param OperationCode operation code for task / код операции для задачи
	 * 		  OperationCode::STOP - stop task / остановить задачу
	 * 		  OperationCode::INFO - print info about task / вывести сообщение о задаче
	 * 		  OperationCode::PAUSE - set task on pause / поставить задачу на паузу
	 * 		  OperationCode::CONTINUE - continue task / продолжить задачу
	 * @return 0 if all is OK, -1 f there is no task with this tasks ID, -2 if something bad happened
	 *         0 если все ок, -1 нет задачи с таким task id, -2 если все плохо
	 */
	int operation_manager(uint task_id, OperationCode op);

	/* @brief remove tasks rom task list if there are end there work
	 * @return
	 */
	void clean_tasks_pool();

private:

	/* @brief checking if word is a number / проверка являеться ли число строкой
 	 * @return boolean true/false / булево значение
 	 */
	inline bool is_number(const std::string& s) const
	{
		std::string::const_iterator it = s.begin();
		while (it != s.end() && isdigit(*it)) ++it;
		return !s.empty() && it == s.end();
	}
};
