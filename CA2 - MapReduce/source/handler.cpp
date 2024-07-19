#include "handler.hpp"

Handler::Handler()
{
    log_file = LOG_PATH+"handler.txt";
    logger("handler started working.\n");
    find_all_building_names();
}

string Handler::get_time()
{
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    int hour = localTime->tm_hour;              
    int minute = localTime->tm_min;             
    int second = localTime->tm_sec;
    return to_string(hour) + ":" + to_string(minute) + ":" + to_string(second); 
}

bool Handler::logger(string msg)
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


vector <string> split_input(string input , char splitter)
{
	string word = "";
	vector <string> splitted;
	for (auto x : input)
    {
        if(splitted.size() == 72)
            return splitted;
        if (x == splitter)
        {
            if(word != " " && word != "")
            {
                splitted.push_back((word));
                word = "";
            }
        }
        else
            word = word + x;
    }
    splitted.push_back(word);
	return splitted;
}


vector <int> Handler::split_string(string input , char splitter)
{
	string word = "";
	vector <int> splitted;
	for (auto x : input)
    {
        if(splitted.size() == 72)
            return splitted;
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
    logger("recieved data splitted.\n");
	return splitted;
}


bool Handler::find_all_building_names()
{
    vector<string> building_names;
    for (const auto& entry : filesystem::directory_iterator(DATA_PATH))
        if (filesystem::is_directory(entry.path()))
        {
            string building_name = entry.path().filename().string();
            building_names.push_back(building_name);
        }
    folders = building_names;
    logger("some buidings found in building folder.\n");
    return true;
}

bool Handler::get_data(string build , string res)
{
    pid_t childpid2, childpid3;
    char  myfifo[] = "/tmp/myfifo";
    int fd = mkfifo(myfifo, 0666);
    int par_child_pipe[2];
    if (pipe(par_child_pipe) == -1)
    {
        logger("Failed to create named pipe.\n" );
        return 1;
    }
    childpid2 = fork();
    if (childpid2 == -1)
    {
        logger("Failed to fork for bill.\n");
        return false;
    }
    else if (childpid2 == 0)
    {
        const char* arguments[] = {BILL_PATH,NULL};
        execvp(BILL_PATH , const_cast<char**>(arguments));
    }
    childpid3 = fork();
    if (childpid3 == -1)
    {
        logger("Failed to fork for building.\n" );
        return false;
    }
    else if (childpid3 == 0)
    {
        close(par_child_pipe[0]);
        dup2(par_child_pipe[1], STDOUT_FILENO);
        const char* arguments[] = {BUILDING_PATH, build.c_str(),res.c_str(),NULL};
        execvp(BUILDING_PATH, const_cast<char**>(arguments));
        logger("Failed to execute program.\n");
    }
    close(par_child_pipe[1]); 
    char buffer[512];
    read(par_child_pipe[0], buffer , sizeof(buffer));
    datas = split_string(buffer , ' ');
    wait(NULL);
    wait(NULL);
    unlink(myfifo);
    return true;
}

void Handler::run()
{
    interface();
    for(int i = 0 ; i< wanted_buildings.size() ; i++)
        for(int j = 0 ; j < wanted_resources.size(); j++)
        {
            get_data(wanted_buildings[i] , wanted_resources[j]);
            cout << "    " <<  wanted_buildings[i] << "   " << wanted_resources[j] << endl;
            print_bill();
        }
}

void Handler::interface()
{
    vector<string> build , res;
     char input[1024];
    for(int i = 0 ; i < folders.size(); i++)
        std::cout << "Building " << i+1 << " : " << folders[i] << endl;
    std::cout << "please write building names :\n";
    cin.getline(input , 1024);
    wanted_buildings = split_input(input , ' ');
    std::cout << "please write resources names :\n";
    cin.getline(input , 1024);
    wanted_resources = split_input(input , ' ');
    logger("interface got some inputs.\n");
}

void Handler::print_bill() 
{
    const int num_cols = 6 , num_rows = 12;
    vector<string> col_names = {"Max Usage", "Avg Usage", "Peak Hours", "Peak Usage", "Difference", "Bill Price"};
    vector<string> row_names = {"Jan", "Feb", "Mar", "Apr", "May", "Jun","Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    std::cout << YELLOW << "╔═════════════╦";
    for (int i = 0; i < num_cols; ++i)
        std::cout << "═════════════" << (i == num_cols - 1 ? "╗" : "╦");
    std::cout << RESET << endl;

    std::cout << YELLOW << "║" << GREEN << setw(12) << "Month" << YELLOW << " ║ ";
    for (int i = 0 ; i < 6 ; i++)
        std::cout << RED << setw(11)  <<col_names[i] << YELLOW << " ║ ";
    std::cout << RESET << endl;
    std::cout << YELLOW << "╠═════════════╬";
    for (int i = 0; i < num_cols; ++i) 
        std::cout << "═════════════" << (i == num_cols - 1 ? "╣" : "╬");
    std::cout << RESET << endl;

    for (int i = 0; i < num_rows; ++i) 
    {
        std::cout << YELLOW << "║" << CYAN << setw(12) << row_names[i] << YELLOW << " ║ ";
        for (int j = 0; j < num_cols; ++j) 
        {
            int data_idx = j * num_rows + i;
            int value = datas[data_idx];
            std::cout << RESET << setw(11) << value << YELLOW << " ║ ";
        }
        std::cout << RESET << endl;
    }
    std::cout << YELLOW << "╚═════════════╩";
    for (int i = 0; i < num_cols; ++i) 
        std::cout << "═════════════" << (i == num_cols - 1 ? "╝" : "╩");
    std::cout << RESET << endl;
    logger("a bill printed to user.\n");
}
