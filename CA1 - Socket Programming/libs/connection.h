#include "io.h"

fd_set config_fd_set(int fd)
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    return read_fds;
}

struct sockaddr_in config_address_UDP()
{
    struct sockaddr_in bc_addr;
    bc_addr.sin_family = AF_INET;
    bc_addr.sin_port = htons(BROADCAST_PORT);
    bc_addr.sin_addr.s_addr = inet_addr("192.168.122.255");
    return bc_addr;
}

int connect_server_UDP(struct sockaddr_in broadcast_addr)
{
    int fd, broadcast = 1, opt = 1;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    bind(fd, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    return fd;
}

int setup_server_TCP(int port) 
{
    struct sockaddr_in address;
    int server_fd , opt = 1;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 4);
    return server_fd;
}

int accept_client_TCP(int server_fd) 
{
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);
    return client_fd;
}

int connect_server_TCP(char *log_file, int port) 
{
    int fd;
    struct sockaddr_in server_address;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    char temp_array[MAX_BUFFER_SIZE*2];
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    int temp ;
    if ( temp = connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        sprintf(temp_array , "Error in connecting to server %d \n" , temp);

    else
        sprintf(temp_array , "connected to server tcp with port : %d\n" , port);
    write_log(log_file , temp_array);
    return fd;
}

int listen_to_check_username(int server_fd_tcp, struct timeval timeout , struct User user) 
{
    fd_set working_tcp , master_tcp;
    int max_fd_tcp = server_fd_tcp ;
    FD_ZERO(&master_tcp);
    FD_SET(server_fd_tcp, &master_tcp);
    for(int i = 0 ; i < 2; i++)
    {
        working_tcp = master_tcp;
        int activity = select(max_fd_tcp + 1, &working_tcp, NULL, NULL, &timeout);
        if (activity == 0)
        {
            if(user.step == SIGNING_UP)
                return TRUE;
        }
        else
        {
            for (int i = 0; i <= max_fd_tcp; i++) 
            {
                if (FD_ISSET(i, &working_tcp)) 
                {
                    if (i == server_fd_tcp) 
                    {  // new client
                        int new_socket = accept_client_TCP(server_fd_tcp);
                        FD_SET(new_socket, &master_tcp);
                        if (new_socket > max_fd_tcp)
                            max_fd_tcp = new_socket;
                    } 
                    else 
                    {
                        char buffer[MAX_BUFFER_SIZE];
                        int bytes_received = recv(i, buffer, MAX_BUFFER_SIZE, 0);
                        if (user.step == SIGNING_UP && strcmp(buffer , user.temp_name) == 0)
                            return FALSE;
                        memset(buffer, 0, MAX_BUFFER_SIZE);
                    }
                }
            }
        }
    }
    return FALSE;
}

int waiting_for_order_response(int server_fd_tcp, struct timeval timeout , struct User user) 
{
    fd_set working_tcp , master_tcp;
    int max_fd_tcp = server_fd_tcp ;
    FD_ZERO(&master_tcp);
    FD_SET(server_fd_tcp, &master_tcp);
    while(TRUE)
    {
        working_tcp = master_tcp;
        int activity = select(max_fd_tcp + 1, &working_tcp, NULL, NULL, &timeout);
        if (activity == 0)
            return FALSE;
        else
        {
            for (int i = 0; i <= max_fd_tcp; i++) 
            {
                if (FD_ISSET(i, &working_tcp)) 
                {
                    if (i == server_fd_tcp) 
                    {
                        int new_socket = accept_client_TCP(server_fd_tcp);
                        FD_SET(new_socket, &master_tcp);
                        if (new_socket > max_fd_tcp)
                            max_fd_tcp = new_socket;
                    } 
                    else 
                    { 
                        char buffer[MAX_BUFFER_SIZE];
                        int bytes_received = recv(i, buffer, MAX_BUFFER_SIZE, 0);
                        if (strcmp(buffer , "yes") == 0)
                            return TRUE;
                        else if(strcmp(buffer , "no") == 0)
                            return FALSE;
                        memset(buffer, 0, MAX_BUFFER_SIZE);
                    }
                }
            }
        }
    }
}


int send_user_available_message(char * log_file , char * bc_buffer)
{
    int count;
    char** result = split_string(bc_buffer, '-', &count);
    int temp_fd;
    char buff[MAX_BUFFER_SIZE] = {0};
    temp_fd = connect_server_TCP(log_file ,atoi(result[2]));
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "user_available message sent\n");
    write_log(log_file , temp);
    return send(temp_fd, USER_AVAILABLE, strlen(USER_AVAILABLE), 0);
}


int send_order_answer(char * log_file , char * bc_buffer)
{
    int count;
    char** result = split_string(bc_buffer, '-', &count);
    int temp_fd;
    char buff[MAX_BUFFER_SIZE] = {0};
    temp_fd = connect_server_TCP(log_file ,atoi(result[2]));
    return send(temp_fd, USER_AVAILABLE, strlen(USER_AVAILABLE), 0);
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "user_available message broadcasted\n");
    write_log(log_file , temp);
}
