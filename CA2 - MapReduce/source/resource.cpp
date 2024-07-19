#include "resource.hpp"

Resource::Resource(string _building , string _name )
{
    building  = _building;
    name = _name;
    log_file = LOG_PATH + building + "-" + name + ".txt";
    parent_fd = 1;
    logger("calculator process created.\n");
    data = read_from_csv();
}

void Resource::run()
{
    calc_monthly_usage();
    calc_monthly_average();
    calc_peak_hours();
    calc_peak_and_average_difference();
    write_to_pipe();
}

vector <int> Resource::split_csv(string input , char splitter)
{
	string word = "";
	vector <int> splitted;
	for (auto x : input)
		if (x == splitter)
		{
			splitted.push_back(stoi(word));
			word = "";
		}
		else
			word = word + x;
	splitted.push_back(stoi(word));
	return splitted;
}

vector<vector<int>> Resource::read_from_csv()
{
    vector<vector<int>> data;
    ifstream file(DATA_PATH + building + '/' + name + ".csv");
    string line;
    getline(file, line);
    while (getline(file, line)) 
        data.push_back(split_csv(line , ','));
    file.close();
    logger("data was read from the csv file.\n");
    return data;
}

string Resource::get_time()
{
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    int hour = localTime->tm_hour;              
    int minute = localTime->tm_min;             
    int second = localTime->tm_sec;
    return to_string(hour) + ":" + to_string(minute) + ":" + to_string(second); 
}

bool Resource::logger(string msg)
{
    ofstream file(log_file, std::ios::app); 
    if (file.is_open()) 
    {
        file << get_time() + " " + msg << endl; 
        file.close(); 
        return true;
    }
    else 
       return false;
}

bool Resource::calc_monthly_usage()
{
    vector <int> usage; 
    int j = 0  , limit;
    for (int i = 1 ; i <= 12 ; i++)
    { 
        limit = i *30;
        int temp = 0;
        for( j ; j< limit ; j++)
            for(int k = 3 ; k <= 8 ; k++)
                temp += data[j][k];
        usage.push_back(temp);
    }
    monthly_usage = usage;
    return logger("resource usage in month calculated.\n");
}

bool Resource::calc_monthly_average()
{
    vector <int> average;
    if (monthly_usage.size() != 12)
        calc_monthly_usage();
    for(int i = 0 ; i < 12 ; i++)
        average.push_back(monthly_usage[i] / 30);
    monthly_average = average;
    return logger("average resource usage in month calculated.\n");
}

bool Resource::calc_peak_hours()
{
    vector <int> temp_peak_hours; 
    vector <int> temp_peak_hours_usage; 
    int j = 0  , limit;
    for (int i = 1 ; i <= 12 ; i++)
    { 
        limit = i *30;
        int temp[6] = { 0 , 0 , 0 , 0 , 0 , 0};
        for( j ; j< limit ; j++)
            for(int k = 3 ; k <= 8 ; k++)
                temp[k-3] += data[j][k];

        int max_idx = 0;
        for(int l = 1 ; l < 6 ; l++)
            if(temp[max_idx] < temp[l])
                max_idx = l;
        temp_peak_hours.push_back(max_idx);
        temp_peak_hours_usage.push_back((temp[max_idx]) / 30);
    }
    peak_hours = temp_peak_hours;
    peak_hours_usage = temp_peak_hours_usage;
    return logger("peak hours in month calculated.\n");
}

bool Resource::calc_peak_and_average_difference()
{
    vector <int> difference;
    for(int i =0; i<12 ; i++)
        difference.push_back((peak_hours_usage[i] - (monthly_average[i]) / 6) );
    peak_and_average_difference = difference;
    return logger("difference between average usage and peak usage calculated.\n");
}

bool Resource::write_to_pipe()
{
    std::vector<int> all_vectors;
    all_vectors.insert(all_vectors.end(), monthly_usage.begin(), monthly_usage.end());
    all_vectors.insert(all_vectors.end(), monthly_average.begin(), monthly_average.end());
    all_vectors.insert(all_vectors.end(), peak_hours.begin(), peak_hours.end());
    all_vectors.insert(all_vectors.end(), peak_hours_usage.begin(), peak_hours_usage.end());
    all_vectors.insert(all_vectors.end(), peak_and_average_difference.begin(), peak_and_average_difference.end());
    write(STDOUT_FILENO, all_vectors.data(), sizeof(int) * all_vectors.size());
    return true;
}
 
int main(int argc , char * argv[])
{
    Resource res = Resource(argv[1] , argv[2]);
    res.run();
    exit(EXIT_SUCCESS);
}