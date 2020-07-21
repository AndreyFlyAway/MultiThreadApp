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
class Task_t
{
public:
	uint task_id;                      // is set by user / назначется вручную
private:
	// TODO: make some attributes atomic?
	time_t time_started;                 // additng time of task время добавления задачи чтобы, отсчитывать и выводить время, через которое очнеться задача
    int delay_sec;                       // delay for starting of task / задержка запуска задачи
    std::atomic<int> progress;           // progress of task / прогресс задачи
    State status;                        // code status: 0 - task is ending, 1 - in waiting, 2 - started, 3 - task in pause_flag / код статуса: 0 - в процессе завершения, 1 - в ожидании, 2 - запущена, 3 - задача приостановлена
    std::atomic<bool> pause_flag;        // that status od task shows that one ot the tread is working with object / статус задачи, который говорит о том, что сейчас идет работа с текущем экзмемпляро задачи
    std::condition_variable resume_cond; // used to resume task if ir was set in pause_flag / используеться для снятия с паузы
	mutable std::shared_mutex obj_mutex; // mutex for protecting data / мьютек для защиты данных
	// TODO: not sure about this mutex
	std::mutex pause_mutex;

public:
    // TODO: necessarily make destructor 'cause I use smart pointer and thread is has to stopped correctly
    Task_t(uint id, int delay_sec);
	Task_t(const Task_t &t) = delete;
	Task_t & operator=(const Task_t&) = delete;

    // thread function / поточная функция
    void thread_operations() ;
    // running thread / запуск задачи
	void run();

	/*
	 * @brief generating info about task
	 * @return string info
	 */
	std::string task_info() const;


	/*
	 * @brief set status of task /  установить статус задачи
	 * @param st status value / значение статауса
	 * @return 0 if OK, -1 if something bad happened
	 */
	int set_status(State st);

	/*
	 * @brief set status of task /  установить статус задачи
	 * @param st status value / значение статауса
	 * @return 0 if OK, -1 if something bad happened
	 */
	State get_status();

	/*
	 * @brief pause_flag task / поставить на паузу
	 * @return 0 if OK, -1 if something bad happened
	 */
	int pause();


	/*
	 * @brief resume task /  возобновить задачу
	 * @return 0 if OK, -1 if something bad happened
	 */
	int resume();
};
