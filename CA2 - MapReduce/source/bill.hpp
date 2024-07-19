#ifndef BILL_HPP
#define BILL_HPP
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;
static const string LOGGING_PATH = "logs/";
static const int EXPECTED_ELECTRICITY_USAGE = 9000;

class Bill
{
public:
    Bill();
    vector <int> split_string(string input , char splitter);
    string calc_bill(vector <int> vals);
    vector<vector<int>> read_from_csv();
    string get_time();
    bool logger(string msg);
    void run();
private:
    string log_file;
};
#endif