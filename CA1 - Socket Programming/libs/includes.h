#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "cJSON.h"

#define FALSE 0
#define TRUE 1
#define MAX_BUFFER_SIZE 1024
#define MAXING_PENDING_ORDER 128
#define BROADCAST_PORT 8080
#define MAX_NAMES 20
#define MAX_INGREDIENTS 10
#define MAX_FOODS 20
#define MAX_NAME_LENGTH 50
#define INIT 0
#define SIGNING_UP 1
#define SIGNED_UP 2
#define OPEN 3

const char *USER_AVAILABLE = "user_available";
const char *RESTAURANT_OPENED = "res_opened";
const char *USERNAME_NOT_UNIQUE = "name is not unique!try again:\n";
const char *GET_USERNAME = "please enter your name:\n";
const char *JSON_FILE_ADDRESS = "json/recipes.json";
const char *COSTUMER = "costumer";
const char *RESTAURANT = "restaurant";
const char *SUPPLIER = "supplier";

struct User
{
    char name[MAX_BUFFER_SIZE];
    char temp_name[MAX_BUFFER_SIZE];
    char log_file[MAX_BUFFER_SIZE*2];
    int port;
    int step;
};

struct List 
{
    char datas[MAX_NAMES][MAX_NAME_LENGTH];
    int count;
};

typedef struct 
{
    char name[50];
    struct {
        char ingredient[50];
        int quantity;
    } ingredients[MAX_INGREDIENTS];
    int numIngredients;
} Food;


struct User init_costumer(int port)
{
    struct User user;
    memset(user.name, 0, MAX_BUFFER_SIZE);
    memset(user.temp_name, 0, MAX_BUFFER_SIZE);
    memset(user.log_file, 0, MAX_BUFFER_SIZE *2);
    sprintf(user.log_file , "logs/%d_log.txt" , port);
    user.port = port;
    user.step = INIT;
    return user;
}

struct List init_names_list()
{
    struct List temp;
    temp.count = 0;
    return temp;
}


void add_new_data(struct List *list, const char *name , const char* port) 
{
    char temp[MAX_BUFFER_SIZE*2];
    if (list->count < MAX_NAMES) 
    {
        sprintf(temp , "%s-%s" , name , port);
        strcpy(list->datas[list->count], temp);
        list->count++;
    } 
}

void remove_data(struct List *list, const char *name , const char* port) 
{
    int index = -1;
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "%s-%s" , name , port);
    for (int i = 0; i < list->count; i++) 
        if (strcmp(list->datas[i], temp) == 0) 
        {
            index = i;
            break;
        }

    if (index != -1) 
    {
        for (int i = index; i < list->count - 1; i++)
            strcpy(list->datas[i], list->datas[i + 1]);
        list->count--;
    } 
}