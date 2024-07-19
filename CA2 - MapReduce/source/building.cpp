#include "building.hpp"

Building::Building(string _building ,string res)
{
    building  = _building;
    log_file = LOG_PATH + building + ".txt";
    resource = res;
    logger("building : " + building + " process created.\n");
}

bool Building::logger(string msg)
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

vector <int> Building::split_string(string input , char splitter)
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
    if(word != " " && word != "")
	    splitted.push_back(stoi(word));
	return splitted;
}

string Building::get_time()
{
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    int hour = localTime->tm_hour;              
    int minute = localTime->tm_min;             
    int second = localTime->tm_sec;
    return to_string(hour) + ":" + to_string(minute) + ":" + to_string(second); 
}

bool Building::run_resource_calculator(int *pipefd )
{
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    const char* arguments[] = {PROGRAM_PATH, building.c_str(),resource.c_str(),NULL};
    execvp(PROGRAM_PATH, const_cast<char**>(arguments));
    logger("Failed to execute program.\n");
    return false;
}

bool Building::get_resources_datas()
{
    int pipefd[2];
    if(pipe(pipefd) == -1)
    {
        logger("error while create pipe.\n");
        return false;
    }
    pid_t pid = fork();
    if (pid == -1) 
    {
        logger("error while fork child process.\n");
        return false;
    }
    if (pid == 0) 
        run_resource_calculator(pipefd);
    else 
    {
        close(pipefd[1]); 
        vector<int> buffer(12);
        while (read(pipefd[0], buffer.data(), buffer.size() * sizeof(int)) > 0) 
            datas.push_back(buffer);
        close(pipefd[0]);
    }
    logger("resource : " + resource + " got.\n");
    return true;
}


void Building::send_data_to_biller()
{
    vector<int> all_vectors;
    all_vectors.insert(all_vectors.end(), datas[0].begin(), datas[0].end());
    all_vectors.insert(all_vectors.end(), datas[3].begin(), datas[3].end());
    string out = "";
    for(int k = 0 ; k < 24 ; k++)
        out += to_string(all_vectors[k]) +" ";
    if (resource == "Water")
        out += "0";
    else if(resource == "Electricity")
        out += "1";
    else
        out +="2";
    int fd = open(myfifo,O_WRONLY);
    write(fd, out.c_str() , sizeof(string) * 24);
    close(fd);
    logger("data of resource: " + resource + " sent to biller.\n");
}

void Building::send_data_to_parent(string buffer)
{
    vector <int> all_vectors;
    all_vectors.insert(all_vectors.end(), datas[0].begin(), datas[0].end());
    all_vectors.insert(all_vectors.end(), datas[1].begin(), datas[1].end());
    all_vectors.insert(all_vectors.end(), datas[2].begin(), datas[2].end());
    all_vectors.insert(all_vectors.end(), datas[3].begin(), datas[3].end());
    all_vectors.insert(all_vectors.end(), datas[4].begin(), datas[4].end());
    string out = "";
    for(int k = 0 ; k < 60 ; k++)
        out += to_string(all_vectors[k]) +" ";
    out += buffer;
    write(STDOUT_FILENO, out.c_str() , out.size());
    logger("data sent to parent.\n");
}

string Building::get_bill()
{
    char buffer[1024];
    int fd = open(myfifo, O_RDONLY);
    read(fd, buffer, 1024);
    close(fd);
    logger("bill of resource "+ resource + " have been got from biller.\n");
    return buffer;
}

void Building::run()
{
    get_resources_datas();
    send_data_to_biller();
    string buffer = get_bill();
    send_data_to_parent(buffer );
    exit(0);
}


int main(int argc , char * argv[])
{
    Building building = Building(argv[1]  ,argv[2]);
    building.run();
    exit(EXIT_SUCCESS);
}