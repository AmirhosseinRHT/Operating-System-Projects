#include "libs/handler.h"

int main(int argc, char *argv[]) 
{
    //////////////init//////////////////////
    char bc_buffer[MAX_BUFFER_SIZE] = {0} , terminal_buffer[MAX_BUFFER_SIZE] = {0} ;
    struct User user = get_port(argc,  argv);
    struct List suppliers_list = init_names_list() ,sales_history = init_names_list() , pending_orders = init_names_list();
    struct sockaddr_in broadcast_addr = config_address_UDP();
    Food foods[MAX_FOODS];
    struct List recipes_list = init_names_list();
    int server_fd_udp = connect_server_UDP(broadcast_addr),server_fd_tcp = setup_server_TCP(user.port) , num_of_foods , splitted_count;
    fd_set read_fds = config_fd_set(server_fd_udp);
    read_from_json_file(JSON_FILE_ADDRESS, foods, &num_of_foods);
    struct List ingredients_list = init_ingredients_list(foods , num_of_foods);
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
            char** splitted = split_string(bc_buffer, '-', &splitted_count);
            if (user.step > SIGNING_UP && strcmp(splitted[0] , "username") == 0)
            {
                if(strcmp(splitted[1] , user.name) == 0)
                    send_user_available_message(user.log_file ,bc_buffer);
            }
            else if(strcmp(splitted[0] , "supplier") == 0)
                suppliers_list = add_new_supplier_to_list(user.log_file , suppliers_list , bc_buffer);
            else if(strcmp(splitted[0] , "orderfood")== 0 && strcmp(splitted[1] , user.name) == 0)
                pending_orders = notify_order_to_restaurant(foods , pending_orders ,splitted[2],splitted[3],user.log_file,num_of_foods);
        }
        else if (FD_ISSET(STDIN_FILENO, &temp_fds)) 
        {
            read_from_terminal(terminal_buffer);
            char** splitted = split_string(terminal_buffer, ' ', &splitted_count);
            if(user.step == INIT)
                user = handle_name_entered(RESTAURANT , user , server_fd_udp , server_fd_tcp , broadcast_addr , terminal_buffer);
            else if(strcmp(terminal_buffer , "break") == 0 && user.step > SIGNED_UP)
                user = handle_break_restaurant(user , server_fd_udp , broadcast_addr);
            else if(strcmp(terminal_buffer , "start working") == 0 && user.step == SIGNED_UP)
                user = handle_start_working_restaurant(user , server_fd_udp , broadcast_addr);
            else if(strcmp(terminal_buffer , "show recipes") == 0 && user.step >= SIGNED_UP)
                print_recipes_list(user.log_file,foods , num_of_foods);
            else if(strcmp(terminal_buffer , "show ingredients") == 0 && user.step >= SIGNED_UP)
                print_ingredients_list(user.log_file,ingredients_list);
            else if(strcmp(terminal_buffer , "show requests list") == 0 && user.step >= SIGNED_UP)
                print_requests_list(user.log_file,pending_orders);
            else if(strcmp(terminal_buffer , "show suppliers") == 0)
                print_suppliers_list(user.log_file , suppliers_list);
            else if(strcmp(splitted[0] , "answer") == 0 && strcmp(splitted[1] , "request") == 0 && splitted_count == 4)
                ingredients_list = answer_food_order(&sales_history , &pending_orders , splitted[2],splitted[3] ,user ,foods,ingredients_list , num_of_foods);
            else if(strcmp(terminal_buffer , "show sales history") == 0)
                print_sales_history(user.log_file , sales_history);
            else if(strcmp(splitted[0],"request") == 0 && strcmp(splitted[1] , "ingredient") == 0 && splitted_count == 5)
                request_ingredient(user, suppliers_list,broadcast_addr ,splitted[2],splitted[3],splitted[4],server_fd_udp ,server_fd_tcp);
            else
                print_wrong_command(user.log_file , terminal_buffer);
        }
    }
    return 0;
}