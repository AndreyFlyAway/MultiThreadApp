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

int print_task_info(std::vector<std::string> data);
int task_mannger(std::string cmd);
int multi_hread_main();

#endif //MULTITHREADAPP_MULTITHREADAPP_H
