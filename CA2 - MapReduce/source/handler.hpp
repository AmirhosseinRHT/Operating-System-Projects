#ifndef HANDLER_HPP
#define HANDLER_HPP
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace std;

#define RESET "\033[0m"
#define YELLOW "\033[33m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define CYAN "\033[36m"

static const string LOG_PATH = "logs/";
static const string DATA_PATH = "buildings/";
const char BILL_PATH[] = "./program/bill.out";
const char BUILDING_PATH[] = "./program/building.out";

class Handler
{
public:
    Handler();
    bool find_all_building_names();
    vector <string> get_folders() {return folders;};
    vector <int> split_string(string input , char splitter);
    string get_time();
    bool logger(string msg);
    bool get_data(string build , string res );
    void print_bill();
    void run();
    void interface();
private:
    vector <string> folders;
    string log_file;
    vector <int> datas;
    vector <string> wanted_buildings;
    vector <string> wanted_resources;
};
vector <string> split_input(string input , char splitter);
#endif