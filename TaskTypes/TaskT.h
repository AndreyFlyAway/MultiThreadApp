#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <future>

enum class State {
	TASK_WORKS,
	TASK_WAITING,
	TASK_PAUSE,
	TASK_END
};

/*
 * Class represent work of thread that can be stopped, set on pause, requested information about current progress,.
 * Also it's base class for other tasks.
 * Класс предоставляет работу потока, который модет быть остановлен, поставлена на паузу, у него может быть запрошена
 * информация от текущем результате. Так же класс являеться базовым для всех остальных типов задач
 */
class TaskT
{
protected:
	std::thread cur_thread;                    // thread object
	// TODO: make some attributes atomic?
	uint task_id;                             // is set by user / назначется вручную
	time_t time_started;                      // time when task was started / время добавления задачи чтобы, отсчитывать и выводить время, через которое очнеться задача
    int delay_sec;                            // delay for starting of task / задержка запуска задачи
    std::atomic<int> progress;                // progress of task / прогресс задачи
    State status;                             // code status: 0 - task is ending, 1 - in waiting, 2 - started, 3 - task in pause_flag / код статуса: 0 - в процессе завершения, 1 - в ожидании, 2 - запущена, 3 - задача приостановлена
    std::atomic<bool> pause_flag;             // that status od task shows that one ot the tread is working with object / статус задачи, который говорит о том, что сейчас идет работа с текущем экзмемпляро задачи
	mutable std::shared_mutex obj_mutex;      // mutex for protecting operations on data / мьютек для защиты операций над данными
	std::condition_variable_any resume_cond;  // used to resume task if it was set in pause_flag / используеться для снятия с паузы
	std::mutex pause_mutex;                   // mutex that used in condition_variable
	std::atomic<bool>  stop_flag;             // stop flag
	std::string str_type;                     // string representation of type, some kind of small description
	std::string result_info;                  // here can be placed string representation of result

protected:
	/* @brief thread function / поточная функция
	 * @return
	 */
	virtual void thread_operations();

	/* @brief set status of task /  установить статус задачи
	 * @param st status value / значение статауса
	 * @return 0 if OK, -1 if something bad happened
	 */
	int set_status(State st);

	/* @brief thread function / потоковая функция
	 * @param time to sleep until start thread work / время, которое будет выжидаться, пока начнеться основная работа потока
	 * @return
	 */
	void thread_function(std::chrono::seconds time_tleep);

	/* @brief set results of task, should be called before task gets status TASK_END in other case
	 * 		  results will not be wrote, it's supposed that calling this function make before stopping
	 * 		  task, since task can be completed by calling stop method
	 * 		  установить результат, эта функци должна быть вызвана до того, как задача будет иметь статус
	 * 		  TASK_END, иначе результат записан не будет, подразумевается, что запись результат осуществляеться
	 * 		  до остановки задача, т.к. задача может быть завершена вызовом метода stop
	 * @param s string with result information / строка с результатом
	 * @return 0 if OK, -1 if task in end process and results cant be set
	 *         0 если ве ОК, -1 если задача в статусе зваершения и результат не может быть записан
	 */
	int set_results(const std::string& s);

public:
    // TODO: necessarily make destructor 'cause I use smart pointer and thread is has to stopped correctly
    TaskT(uint id, int delay_sec);
	TaskT(const TaskT &t) = delete;
	TaskT & operator=(const TaskT&) = delete;
	// TODO: make move constructors
	virtual ~TaskT();
	/* @brief running thread / запуск задачи
	 * @return string info
	 */
	void run();

	/* @brief generating info about task
	 * @return string info
	 */
	std::string task_info() const;


	/* @brief task status /  статус задачи
	 * @return 0 if OK, -1 if something bad happened
	 */
	State get_status() const;

	/* @brief pause_flag task / поставить на паузу
	 * @return 0 if OK, 1 if paused already, -1 if something bad happened
	 */
	int pause();

	/* @brief resume task /  возобновить задачу
	 * @return 0 if OK, -1 if something bad happened
	 */
	int resume();

	/* @brief use to stop task / остановка задачи
	 * @return 0 if OK, -1 if something bad happened
	 */
	int stop();

	/* @brief return results of task / результат выполнения задачи
	 * @return string result
	 */
	std::string get_results();

	friend class TaskPool;
};

/* @brief  class where overloaded method thread_operations, in this method progress increasing made
 * in async thread
 */
class TaskAsyncProgress : public TaskT{
public:
	TaskAsyncProgress(uint id, int delay);
	virtual ~TaskAsyncProgress() override {};
protected:
	void thread_operations() override;

	/* @brief Method that increase progress value from 0 to 100 during sec_to_work seconds. It is assumed
	 *        that method works async in thread_operations method
	 *        Метод увеличивает значение прогресса от 0 до 100 в течении sec_to_work секунд. Подразумеваеться,
	 *        что метод работает асихнронно в thread_operations.
	 * @param sec_to_work time in seconds during which progress will be increasing
	 *        количество секунд, в течении которого будет увеличиваться прогресс
	 * @return value of progress / начение progress
	 */
	int progress_value_async(int sec_to_work=20);
};
