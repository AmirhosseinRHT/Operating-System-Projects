#include "libs/handler.h"


int main(int argc, char *argv[]) 
{
    //////////////init//////////////////////
    char bc_buffer[MAX_BUFFER_SIZE] = {0} , terminal_buffer[MAX_BUFFER_SIZE] = {0};
    struct User user = get_port(argc,  argv);
    Food foods[MAX_FOODS];
    struct List restaurants_list = init_names_list() , pending_requests = init_names_list();
    struct sockaddr_in broadcast_addr = config_address_UDP();
    int server_fd_udp = connect_server_UDP(broadcast_addr),server_fd_tcp = setup_server_TCP(user.port) , num_of_foods , splitted_count;
    fd_set read_fds = config_fd_set(server_fd_udp);
    read_from_json_file(JSON_FILE_ADDRESS, foods, &num_of_foods);
    write(1, GET_USERNAME , strlen(GET_USERNAME));
    while (1) 
    {
        fd_set temp_fds = read_fds;
        int max_fd = (server_fd_udp > STDIN_FILENO) ? server_fd_udp : STDIN_FILENO;
        int activity = select(max_fd + 1, &temp_fds, NULL, NULL, NULL);
        if (FD_ISSET(server_fd_udp, &temp_fds)) 
        {
            memset(bc_buffer, 0, MAX_BUFFER_SIZE);
            recv(server_fd_udp, bc_buffer, MAX_BUFFER_SIZE, 0);
            if (user.step > SIGNING_UP)
            {
                char temp [128];
                sprintf(temp ,"%d", user.port );
                char** splitted = split_string(bc_buffer, '-', &splitted_count);
                if(strcmp(splitted[1] , user.name) == 0 && strcmp(splitted[0] , "username") == 0)
                    send_user_available_message(user.log_file,bc_buffer);
                else if(strcmp(splitted[0] , "restaurant") == 0)
                    restaurants_list = handle_new_restaurant_activity(user.log_file , restaurants_list , bc_buffer);
                else if(strcmp(splitted[0] , "requestingredient")== 0 && strcmp(splitted[3] , temp) == 0)
                   pending_requests = notify_request_to_supplier(pending_requests ,splitted[1],splitted[2] , splitted[4],user.log_file);
            }
        }
        else if (FD_ISSET(STDIN_FILENO, &temp_fds)) 
        {
            read_from_terminal(terminal_buffer);
            char **splitted = split_string(terminal_buffer , ' ' , &splitted_count);
            if(user.step == INIT)
                user = handle_name_entered(SUPPLIER , user , server_fd_udp , server_fd_tcp , broadcast_addr , terminal_buffer);
            else if(strcmp(terminal_buffer , "show restaurants") == 0)
                print_restaurants_list(user.log_file,restaurants_list);
            else if (strcmp(terminal_buffer , "show menu") == 0)
                print_menu(user.log_file , foods ,num_of_foods);
            else if(strcmp(splitted[0] , "answer") == 0 && strcmp(splitted[1] , "request") == 0 && splitted_count == 4)
                answer_ingredient_request(&pending_requests ,splitted[2],splitted[3],user);
            else
                print_wrong_command(user.log_file , terminal_buffer);
        }
    }
    return 0;
}