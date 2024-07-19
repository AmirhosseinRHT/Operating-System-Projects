#include "connection.h"

struct List handle_new_restaurant_activity(char * log_file , struct List restaurants_list,char * bc_buffer)
{
    struct List temp = restaurants_list;
    int count;
    char temp_array[MAX_BUFFER_SIZE*2];
    char** result = split_string(bc_buffer, '-', &count);
    if(strcmp(result[1] , "opened") == 0)
    {
        add_new_data(&temp , result[2] , result[3]);
        sprintf(temp_array , "restuarant %s opened!\n", result[2]);
        write(STDOUT_FILENO , temp_array , strlen(temp_array));
    }
    else if(strcmp(result[1] , "closed") == 0)
    {
        remove_data(&temp , result[2] , result[3]);
        sprintf(temp_array , "restuarant %s closed!\n", result[2]);
        write(STDOUT_FILENO , temp_array , strlen(temp_array));
    }
    sprintf(temp_array , "restaurants list updated\n");
    write_log(log_file , temp_array);
    return temp;
}

struct List add_new_supplier_to_list(char * log_file , struct List suppliers_list,char * bc_buffer)
{
    struct List temp = suppliers_list;
    int count;
    char temp_array[MAX_BUFFER_SIZE*2];
    char** result = split_string(bc_buffer, '-', &count);
    if(strcmp(result[1] , "opened") == 0)
    {
        add_new_data(&temp , result[2] , result[3]);
        sprintf(temp_array , "supplier %s opened!\n", result[2]);
        write(STDOUT_FILENO , temp_array , strlen(temp_array));
    }
    else if(strcmp(result[1] , "closed") == 0)
    {
        remove_data(&temp , result[2] , result[3]);
        sprintf(temp_array , "restuarant %s closed!\n", result[2]);
        write(STDOUT_FILENO , temp_array , strlen(temp_array));
    }
    sprintf(temp_array , "suppliers list updated\n");
    write_log(log_file , temp_array);
    return temp;
}


struct User handle_name_entered(const char * rule , struct User user_data , int server_fd_udp , int server_fd_tcp , struct sockaddr_in broadcast_addr , char *terminal_buffer)
{
    char temp[512];
    struct User user = user_data;
    sprintf(temp , "username-%s-%d" , terminal_buffer , user.port);
    int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    user.step = SIGNING_UP;
    strcpy(user.temp_name , terminal_buffer);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (listen_to_check_username(server_fd_tcp , timeout , user))
    {   
        if(user.step == SIGNING_UP)
        {
            char temp[MAX_BUFFER_SIZE*2];
            strcpy(user.name , user.temp_name);
            user.step = SIGNED_UP;
            sprintf(temp , "welcome %s as %s!!\n" , user.name , rule);
            write(STDOUT_FILENO , temp , strlen(temp));
            if (strcmp(rule , SUPPLIER) == 0)
            {
                sprintf(temp , "%s-opened-%s-%d" ,SUPPLIER ,user.name , user.port);
                int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            }
        }
    }
    else
    {
        write(STDOUT_FILENO , USERNAME_NOT_UNIQUE , strlen(USERNAME_NOT_UNIQUE));
        user.step = INIT;
    }
    sprintf(temp , "username validation handeled\n");
    write_log(user.log_file , temp);
    return user;

}

struct User handle_break_restaurant(struct User user_data ,int server_fd_udp , struct sockaddr_in broadcast_addr)
{
    char temp[MAX_BUFFER_SIZE*2];
    struct User user = user_data;
    user.step = SIGNED_UP;
    sprintf(temp , "restaurant-closed-%s-%d" , user.name , user.port);
    int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    sprintf(temp , "restaurant %s closed\n" , user.name);
    write_log(user.log_file , temp);
    return user;
}

struct User handle_start_working_restaurant(struct User user_data ,int server_fd_udp , struct sockaddr_in broadcast_addr)
{
    char temp[MAX_BUFFER_SIZE*2];
    struct User user = user_data;
    user.step = OPEN;
    sprintf(temp , "restaurant-opened-%s-%d" , user.name , user.port);
    int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
    sprintf(temp , "restaurant %s opened\n" , user.name);
    write_log(user.log_file , temp);
    return user;
}


int order_food(struct User user, struct List restaurants_list,struct sockaddr_in broadcast_addr , char * restaurant_name ,char * food_name , int server_fd_udp , int server_fd_tcp)
{
    int splitted_count = 0;
    struct timeval timeout;
    timeout.tv_sec = 120;
    timeout.tv_usec = 0;
    for(int i = 0 ; i < restaurants_list.count ; i++)
    {
        char ** splitted = split_string(restaurants_list.datas[i] , '-' ,&splitted_count);
        if (strcmp(splitted[0] , restaurant_name) == 0)
        {
            char temp[MAX_BUFFER_SIZE*2];
            sprintf(temp , "orderfood-%s-%s-%d" , restaurant_name , food_name , user.port);
            int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            sprintf(temp , "waiting for restaurant %s response...\n" , restaurant_name);
            write_log(user.log_file , temp);
            write(STDOUT_FILENO , temp , strlen(temp));
            if (waiting_for_order_response(server_fd_tcp , timeout ,user))
            {
                sprintf(temp , "order accepted an your food is ready!\n");
                write_log(user.log_file , temp);
                write(STDOUT_FILENO , temp , strlen(temp));
            }
            else
            {
                sprintf(temp , "order denied!\n");
                write_log(user.log_file , temp);
                write(STDOUT_FILENO , temp , strlen(temp));
            }
            return TRUE;
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "wrong restaurant name!\n");
    write_log(user.log_file , temp);
    write(STDOUT_FILENO , temp , strlen(temp));
    return FALSE;
}


int request_ingredient(struct User user, struct List suppliers_list,struct sockaddr_in broadcast_addr , char * ingredient ,char * amount ,char * port, int server_fd_udp , int server_fd_tcp)
{
    int splitted_count = 0;
    struct timeval timeout;
    timeout.tv_sec = 90;
    timeout.tv_usec = 0;
    for(int i = 0 ; i < suppliers_list.count ; i++)
    {
        char ** splitted = split_string(suppliers_list.datas[i] , '-' ,&splitted_count);
        if (strcmp(splitted[1] ,port ) == 0)
        {
            char temp[MAX_BUFFER_SIZE*2];
            sprintf(temp , "requestingredient-%s-%s-%s-%d" , ingredient , amount , port ,user.port);
            int bytes_sent = sendto(server_fd_udp, temp, strlen(temp), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            sprintf(temp , "waiting for supplier %s response...\n" , splitted[0]);
            write_log(user.log_file , temp);
            write(STDOUT_FILENO , temp , strlen(temp));
            if (waiting_for_order_response(server_fd_tcp , timeout ,user))
            {
                sprintf(temp , "%s accepted your request\n" , splitted[0]);
                write_log(user.log_file , temp);
                write(STDOUT_FILENO , temp , strlen(temp));
            }
            else
            {
                sprintf(temp , "your request denied\n");
                write_log(user.log_file , temp);
                write(STDOUT_FILENO , temp , strlen(temp));
            }
            return TRUE;
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "wrong supplier port!\n");
    write_log(user.log_file , temp);
    write(STDOUT_FILENO , temp , strlen(temp));
    return FALSE;
}


struct List init_ingredients_list(Food foods[] , int num_of_foods)
{
    struct List ingredients = init_names_list();
    for (int i=0 ; i < num_of_foods ; i++)
    {
        char temp[MAX_BUFFER_SIZE*2];
        char zero[2];
        sprintf(zero ,"0");
        for (int j = 0; j < foods[i].numIngredients; j++) 
        {   
            int flag = 0;
            for (int k = 0 ; k < ingredients.count ; k++)
            {
                sprintf(temp ,"%s-0", foods[i].ingredients[j].ingredient);
                if (strcmp(temp , ingredients.datas[k]) == 0)
                {
                    flag = 1;
                    break;
                }
                else
                    flag = 0;
            }
            if (flag == 0)
                add_new_data(&ingredients,foods[i].ingredients[j].ingredient ,zero);
        }
    }
    return ingredients;
}

struct List notify_order_to_restaurant(Food foods[] , struct List pending_orders ,char * food_name,char *port,char * log_file, int num_of_foods)
{
    struct List temp = pending_orders;
    for (int i=0 ; i < num_of_foods ; i++)
    {
        if (strcmp(food_name , foods[i].name) == 0)
        {
            add_new_data(&temp,food_name , port);
            char temp_array[MAX_BUFFER_SIZE*2];
            sprintf(temp_array , "new order food : %s from port: %s\n" , food_name , port);
            write_log(log_file , temp_array);
            write(STDOUT_FILENO , temp_array , strlen(temp_array));
            return temp;
        }
    }
    return pending_orders;
}


struct List notify_request_to_supplier( struct List pending_requests ,char * ingredinet_name,char *amount , char *port,char * log_file)
{
    struct List temp = pending_requests;
    add_new_data(&temp,ingredinet_name , port);
    char temp_array[MAX_BUFFER_SIZE*2];
    sprintf(temp_array , "new ingredinet request : %s from port: %s\n" , ingredinet_name , port);
    write_log(log_file , temp_array);
    write(STDOUT_FILENO , temp_array , strlen(temp_array));
    return temp;
}

struct List answer_food_order(struct List *sales_history , struct List *order_history , char * port, char * answer ,struct User user , Food foods[] , struct List ingredients_list , int num_of_foods)
{
    int splitted_count = 0;
    char temp[MAX_BUFFER_SIZE*2];
    int order_index = find_name_index(*order_history , port , 1);
    char ** splitted = split_string(order_history->datas[order_index] , '-' , &splitted_count);
    char * food_name = splitted[0];
    struct List ingredients_temp = ingredients_list;
    if(strcmp(answer , "yes") == 0)
    {
        for (int i=0 ; i < num_of_foods ; i++)
        {
            for (int j = 0; j < foods[i].numIngredients; j++) 
            {
                for (int k = 0 ; k<ingredients_list.count ; k++)
                {
                    char** splitted = split_string(ingredients_list.datas[i] , '-' , &splitted_count);
                    if(strcmp(food_name , foods[i].name) == 0)
                        if(strcmp(foods[i].ingredients[j].ingredient , ingredients_list.datas[k])==0)
                        {
                            if (foods[i].ingredients[j].quantity >= atoi(splitted[1]))
                            {
                                sprintf(temp , "insufficient ingredients for food %s\n" , food_name);
                                write_log(user.log_file , temp);
                                write(STDOUT_FILENO , temp , strlen(temp));
                                return ingredients_list;
                            }
                            else
                            {
                                int remained  = atoi(splitted[1])-(foods[i].ingredients[j].quantity);
                                sprintf(ingredients_temp.datas[i]  , "%s-%d" , splitted[0] , remained);
                            }
                        }
                }
            }
        }
        int fd = connect_server_TCP(user.log_file , atoi(port));
        send(fd, answer, strlen(answer), 0);
        sprintf(temp , "food %s accepted\n" , food_name);
        write_log(user.log_file , temp);
        write(STDOUT_FILENO , "done\n" , strlen("done\n"));
        sprintf(temp , "%s-%s" ,user.name , food_name );
        remove_data(order_history ,food_name , port);
        add_new_data(sales_history,temp , answer);
        return ingredients_temp;
    }
    else if(strcmp(answer , "no") == 0)
    {
        int fd = connect_server_TCP(user.log_file , atoi(port));
        send(fd, answer, strlen(answer), 0);
        sprintf(temp , "food %s denied\n" , food_name);
        write(STDOUT_FILENO , "done\n" , strlen("done\n"));
        write_log(user.log_file , temp);
        sprintf(temp , "%s-%s" ,user.name , food_name );
        remove_data(order_history ,food_name , port);
        add_new_data(sales_history,temp , answer);
        return ingredients_temp;
    }
    else
        return ingredients_list;
}



int answer_ingredient_request(struct List *pending_requests , char * port, char * answer ,struct User user)
{
    int splitted_count = 0;
    char temp[MAX_BUFFER_SIZE*2];
    int order_index = find_name_index(*pending_requests , port , 1);
    char ** splitted = split_string(pending_requests->datas[order_index] , '-' , &splitted_count);
    char * ingredient_name = splitted[0];
    if(strcmp(answer , "yes") == 0)
    {
        int fd = connect_server_TCP(user.log_file , atoi(port));
        send(fd, answer, strlen(answer), 0);
        sprintf(temp , "request %s accepted\n" , ingredient_name);
        write_log(user.log_file , temp);
        write(STDOUT_FILENO , "done\n" , strlen("done\n"));
        remove_data(pending_requests ,ingredient_name , port);
        return TRUE;
    }
    else if(strcmp(answer , "no") == 0)
    {
        int fd = connect_server_TCP(user.log_file , atoi(port));
        send(fd, answer, strlen(answer), 0);
        sprintf(temp , "food %s denied\n" , ingredient_name);
        write(STDOUT_FILENO , "done\n" , strlen("done\n"));
        write_log(user.log_file , temp);
        remove_data(pending_requests ,ingredient_name , port);
        return TRUE;
    }
    else
        return FALSE;
}