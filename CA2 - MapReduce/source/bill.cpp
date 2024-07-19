#include "bill.hpp"

Bill::Bill( )
{
    log_file = LOGGING_PATH + "bill.txt";
    logger("bill process created.\n");
}

vector<vector<int>> Bill::read_from_csv()
{
    vector<vector<int>> data;
    ifstream file("buildings/bills.csv");
    if(!file.is_open())
    {
        logger ("bill csv file not found!\n");
        exit(EXIT_FAILURE);
    }
    string line;
    getline(file, line);
    while (getline(file, line)) 
        data.push_back(split_string(line , ','));
        
    file.close();
    logger("data was read from the bills.csv file.\n");
    return data;
}

string Bill::get_time()
{
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    int hour = localTime->tm_hour;              
    int minute = localTime->tm_min;             
    int second = localTime->tm_sec;
    return to_string(hour) + ":" + to_string(minute) + ":" + to_string(second); 
}

bool Bill::logger(string msg)
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

vector <int> Bill::split_string(string input , char splitter)
{
	string word = "";
	vector <int> splitted;
	for (auto x : input)
    {
        if (x == splitter)
        {
            if(word != " " && word != "")
            {
                splitted.push_back(stoi(word));
                word = "";
            }
        }
        else
            word = word + x;
    }
    if(word != " " || word != "")
        splitted.push_back(stoi(word));
    logger("recieved data splitted.\n");
	return splitted;
}


string Bill::calc_bill(vector <int> vals)
{
    string answer = "";
    vector < vector<int>> standards = read_from_csv();
    if(vals[24] == 0)
        for(int k = 0 ; k < 12 ; k++)
        {
            int temp = ((vals[k] * standards[k][2]) + (vals[2*k+1] * standards[k][2] * 0.25)) ;
            answer += to_string(temp) + " ";
        }

    else if(vals[24] == 1)
        for(int k = 0 ; k < 12 ; k++)
        {
            if (vals[k] < EXPECTED_ELECTRICITY_USAGE)
            {
                int temp = ((vals[k] * standards[k][4]) + (vals[2*k+1] * standards[k][4] * 0.25)) * 0.75;
                answer += to_string(temp) + " ";
            }
            else
            {
                int temp = ((vals[k] * standards[k][3]) + (vals[2*k+1] * standards[k][3] * 0.25));
                answer += to_string(temp) + " ";
            }
        }
    else
        for(int k = 0 ; k < 12 ; k++)
        {
            int temp = ((vals[k] * standards[k][3]));
            answer += to_string(temp) + " ";
        }
    logger("bill calculated.\n");
    return answer;
}

void Bill::run()
{
    logger("bill process is now starting calculating.\n");
    int fd;
    char  myfifo[] = "/tmp/myfifo";
    fd = open(myfifo, O_RDONLY);
    char buffer[1024];
    read(fd, buffer, 1024);
    close(fd);
    vector <int> vals = split_string(buffer , ' ');
    string answer = calc_bill(vals);
    fd = open(myfifo,O_WRONLY);
    write(fd, answer.c_str(), answer.size()+1);
    close(fd);
}


int main()
{
    Bill bill = Bill();
    bill.run();
    exit(EXIT_SUCCESS);
}