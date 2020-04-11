#include <iostream>
#include <string>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;

/* command constants*/
const string EXIT_CMD = "q";
const string START_TASK_CMD = "s";
const string INFO_CMD = "info";

/* other */

/* @brief обработчик консольных сообщений
 * @return 0 если все ок, 1 если пришла команда завершения, -1 если все плохо
 * */
int task_mannger(string cmd)
{
    pthread_t t1;

    /* splitting line by spaces */
    stringstream ss(cmd);
    istream_iterator<std::string> begin_s(ss);
    istream_iterator<std::string> end_s;
    vector<string> commands(begin_s, end_s);

    if (commands.size() < 1){
        //TODO: print_help()
        return -1;
    }

    if (commands[0] == EXIT_CMD){
        cout << "End work bye! " << endl;
        return 1;
    }
    else if (commands[0] == START_TASK_CMD)
    {
        //
    }
    else if (commands[0] == INFO_CMD)
    {
        //
    }
    else
    {
        cout << "Unregonized command " << cmd << endl;
        //TODO: print_help()
    }
    return 0;
}

int main()
{
    int res;
    string cmd;
    bool exit_f = false;
    while (!exit_f)
    {
        getline(cin, cmd);
        if ((res=task_mannger(cmd)) == 1){
            break;
        }
    }
    return 0;
}