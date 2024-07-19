#ifndef RESOURCE_HPP
#define RESOURCE_HPP
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

using namespace std;
static const string DATA_PATH = "buildings/";
static const string LOG_PATH = "logs/";

class Resource
{
public:
    Resource(string _building , string _name);
    void run();
    bool calc_monthly_average();
    bool calc_monthly_usage();
    bool calc_peak_hours();
    bool calc_peak_and_average_difference();
    bool logger(string msg);
    vector<vector<int>> read_from_csv();
    vector <int> split_csv(string input , char splitter);
    vector <int> get_monthly_usage() {return monthly_usage;};
    vector <int> get_monthly_average() {return monthly_average;};
    vector <int> get_peak_hours() {return peak_hours;};
    vector <int> get_peak_hours_usage() {return peak_hours_usage;};
    vector <int> get_peak_and_average_difference() {return peak_and_average_difference;};
    string get_time();
    bool write_to_pipe();
private:
    string building;
    string name;
    string log_file;
    int  parent_fd;
    vector<vector<int>> data;
    vector <int> monthly_usage;
    vector <int> monthly_average;
    vector <int> peak_hours;
    vector <int> peak_hours_usage;
    vector <int> peak_and_average_difference;
};
#endif