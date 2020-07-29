//
// Created by user on 22.04.20.
//

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
#include <time.h>
#include <shared_mutex>
#include <atomic>


/* other */
/* статусы для задач  */
// TODO: сделать через enum
const int TASK_WORKS = 1;
const int TASK_WAITING = 2;
const int TASK_PAUSE = 3;
const int TASK_END = 4;

enum class State {
	TASK_WORKS,
	TASK_WAITING,
	TASK_PAUSE,
	TASK_END
};


/*
 * Class describes object, that works in due std::thread, most of operations that made by functions
 * in MultiThread.cpp will be made as method in class.
 * Класс описывает объект, который будет при помощи std::thread, болшая часть операций, которые реализованы
 * в MultiThread.cpp через функции планируються быть реализованы через методы класса.
 */
class TaskT
{
protected:
	// TODO: make some attributes atomic?
	uint task_id;                             // is set by user / назначется вручную
	time_t time_started;                      // time when task was started / время добавления задачи чтобы, отсчитывать и выводить время, через которое очнеться задача
    int delay_sec;                            // delay for starting of task / задержка запуска задачи
    std::atomic<int> progress;                // progress of task / прогресс задачи
    State status;                             // code status: 0 - task is ending, 1 - in waiting, 2 - started, 3 - task in pause_flag / код статуса: 0 - в процессе завершения, 1 - в ожидании, 2 - запущена, 3 - задача приостановлена
    std::atomic<bool> pause_flag;             // that status od task shows that one ot the tread is working with object / статус задачи, который говорит о том, что сейчас идет работа с текущем экзмемпляро задачи
	mutable std::shared_mutex obj_mutex;      // mutex for protecting data / мьютек для защиты данных
	std::condition_variable_any resume_cond;  // used to resume task if it was set in pause_flag / используеться для снятия с паузы
	std::mutex pause_mutex;                   // mutex that used in condition_variable
	std::atomic<bool>  stop_flag;             // stop flag
	std::string str_type;                     // string representation of type, some kind of small description

protected:
	/* @brief thread function / поточная функция
	 * @return
	 */
	virtual void thread_operations();

	/*
	 * @brief set status of task /  установить статус задачи
	 * @param st status value / значение статауса
	 * @return 0 if OK, -1 if something bad happened
	 */
	int set_status(State st);

public:
    // TODO: necessarily make destructor 'cause I use smart pointer and thread is has to stopped correctly
    TaskT(uint id, int delay_sec);
	TaskT(const TaskT &t) = delete;
	TaskT & operator=(const TaskT&) = delete;


    // running thread / запуск задачи
	void run();

	/*
	 * @brief generating info about task
	 * @return string info
	 */
	std::string task_info() const;

	/*
	 * @brief pause_flag task / поставить на паузу
	 * @return 0 if OK, 1 if paused already, -1 if something bad happened
	 */
	int pause();

	/*
	 * @brief resume task /  возобновить задачу
	 * @return 0 if OK, -1 if something bad happened
	 */
	int resume();


	/*
	 * @brief use to stop task / остановка задачи
	 * @return 0 if OK, -1 if something bad happened
	 */
	int stop();
};

/* @brief  class where overloaded method thread_operations, in this method progress increasing made
 * in async thread
 */
class TaskAsyncProgress : public TaskT{
public:
	TaskAsyncProgress(uint id, int delay);
protected:
	void thread_operations();

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