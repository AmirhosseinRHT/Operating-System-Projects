#ifndef BUILDING_HPP
#define BUILDING_HPP
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

static const string LOG_PATH = "logs/";
static const char* PROGRAM_PATH = "./program/resource.out";
char myfifo[] = "/tmp/myfifo";

class Building
{
public:
    Building(string _building , string res);
    bool logger(string msg);
    string get_time();
    void send_data_to_biller();
    bool run_resource_calculator(int *pipefd);
    vector <int> split_string(string input , char splitter);
    bool get_resources_datas();
    void send_data_to_parent(string buffer);
    string get_bill();
    void run();
private:
    vector <vector<int> > datas;
    string building;
    string resource;
    string log_file;
    int *bill_to_building_pipe;
    int *building_to_bill_pipe;
};
#endif
