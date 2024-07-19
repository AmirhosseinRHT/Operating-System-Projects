#include "includes.h"

void write_log(const char* fileName, const char* str) 
{
  int fileDescriptor = open(fileName, O_CREAT | O_WRONLY | O_APPEND, 0644);
  if (fileDescriptor == -1) {
    printf("Could not open or create file.\n");
    return;
  }
  if (write(fileDescriptor, str, strlen(str)) == -1)
    printf("Error writing to file.\n");
  if (close(fileDescriptor) == -1)
    printf("Error closing file.\n");
}

char** split_string(const char* str, char splitter, int* count) 
{
    int len = strlen(str) , num_splits = 0;
    char** result = NULL;
    for (int i = 0; i < len; i++)
        if (str[i] == splitter)
            num_splits++;
    result = (char**)malloc((num_splits + 1) * sizeof(char*));
    if (result == NULL) 
    {
        *count = 0;
        return NULL;
    }
    char* copy = strdup(str);
    char* token = strtok(copy, &splitter);
    int i = 0;
    while (token != NULL) 
    {
        result[i] = strdup(token);
        token = strtok(NULL, &splitter);
        i++;
    }
    *count = i;
    free(copy);
    return result;
}


void print_wrong_command(char * log_file , char * terminal_buffer)
{
    write(STDOUT_FILENO , "wrong command!\n" , strlen("wrong command!\n"));
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "wrong command entered : %s\n" , terminal_buffer);
    write_log(log_file , temp);
}

int find_name_index(struct List list , char * name , int location)
{
    int splitted_count = 0 ;
    for(int i = 0 ; i < list.count ; i++ )
    {
        char ** splitted  = split_string(list.datas[i] , '-' , &splitted_count);
        if (strcmp(splitted[location] , name) == 0 )
            return i;
    }
    return -1;
}

void read_from_terminal(char *buffer) 
{
    ssize_t bytesRead;
    memset(buffer, 0, MAX_BUFFER_SIZE);
    bytesRead = read(STDIN_FILENO, buffer, MAX_BUFFER_SIZE);
    if (bytesRead < 0)
        perror("Error reading input");
    buffer[bytesRead-1] = '\0';
}


struct User get_port(int argc , char * argv[])
{
    struct User user;
    if (argc == 2)
        user = init_costumer(atoi(argv[1]));
    else
    {
        write(1, "wrong port!\n", 12);
        exit(EXIT_FAILURE);
    }
    return user;
}

void read_from_json_file(const char* filename, Food* foods, int* numFoods) 
{
    int file = open(filename, O_RDONLY);
    if (file == -1) 
    {
        printf("Failed to open file %s\n", filename);
        exit(1);
    }
    struct stat fileStat;
    fstat(file, &fileStat);
    off_t fileSize = fileStat.st_size;

    char* fileContents = (char*)malloc(fileSize + 1);
    ssize_t bytesRead = read(file, fileContents, fileSize);
    if (bytesRead == -1) {
        printf("Failed to read file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fileContents[bytesRead] = '\0';
    close(file);
    cJSON* root = cJSON_Parse(fileContents);
    if (root == NULL) 
    {
        printf("Failed to parse JSON: %s\n", cJSON_GetErrorPtr());
        exit(1);
    }
    cJSON* item = NULL;
    int i = 0;
    cJSON_ArrayForEach(item, root) 
    {
        cJSON* foodObject = cJSON_GetObjectItem(root, item->string);

        strcpy(foods[i].name, item->string);
        foods[i].numIngredients = 0;

        cJSON* ingredient = NULL;
        cJSON_ArrayForEach(ingredient, foodObject) {
            strcpy(foods[i].ingredients[foods[i].numIngredients].ingredient, ingredient->string);
            foods[i].ingredients[foods[i].numIngredients].quantity = cJSON_GetObjectItem(foodObject, ingredient->string)->valueint;
            foods[i].numIngredients++;
        }
        i++;
    }
    *numFoods = i;
    cJSON_Delete(root);
    free(fileContents);
}

void print_restaurants_list(char * log_file , struct List restaurants_list)
{
    if (restaurants_list.count == 0)
        write(STDOUT_FILENO , "no restaurant is open\n" , strlen("no restaurant is open\n"));
    else
    {
        write(STDOUT_FILENO , "Restaurant & port:\n" , strlen("Restaurant & port:\n"));
        for (int i =0 ; i<restaurants_list.count;i++)
        {
            int count = 0;
            char temp[MAX_BUFFER_SIZE*2];
            char ** splitted = split_string(restaurants_list.datas[i] , '-' , &count);
            sprintf(temp , "%s %s\n" , splitted[0] , splitted[1]);
            write(STDOUT_FILENO , temp , strlen(temp));
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "restuarants list printed\n");
    write_log(log_file , temp);
}


void print_sales_history(char * log_file , struct List sales_history)
{
    if (sales_history.count == 0)
        write(STDOUT_FILENO , "no sale is happened\n" , strlen("no sale is happened\n"));
    else
    {
        write(STDOUT_FILENO , "user & food & status:\n" , strlen("user & food & status:\n"));
        for (int i =0 ; i<sales_history.count;i++)
        {
            int count = 0;
            char temp[MAX_BUFFER_SIZE*2];
            char ** splitted = split_string(sales_history.datas[i] , '-' , &count);
            sprintf(temp , "%s %s %s\n" , splitted[0] , splitted[1], splitted[2]);
            write(STDOUT_FILENO , temp , strlen(temp));
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "sales history list printed\n");
    write_log(log_file , temp);
}


void print_suppliers_list(char * log_file, struct List suppliers_list)
{
    if (suppliers_list.count == 0)
        write(STDOUT_FILENO , "no supplier is open!\n" , strlen("no supplier is open!\n"));
    else
    {
        write(STDOUT_FILENO , "suppliers & port:\n" , strlen("suppliers & port:\n"));
        for (int i =0 ; i<suppliers_list.count;i++)
        {
            int count = 0;
            char temp[MAX_BUFFER_SIZE*2];
            char ** splitted = split_string(suppliers_list.datas[i] , '-' , &count);
            sprintf(temp , "%s %s\n" , splitted[0] , splitted[1]);
            write(STDOUT_FILENO , temp , strlen(temp));
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "suppliers printed\n");
    write_log(log_file , temp);
}


void print_requests_list(char * log_file, struct List requests_list)
{
    if (requests_list.count == 0)
        write(STDOUT_FILENO , "no request pending!\n" , strlen("no request pending!\n"));
    else
    {
        write(STDOUT_FILENO , "food & port:\n" , strlen("food & port:\n"));
        for (int i =0 ; i<requests_list.count;i++)
        {
            int count = 0;
            char temp[MAX_BUFFER_SIZE*2];
            char ** splitted = split_string(requests_list.datas[i] , '-' , &count);
            sprintf(temp , "%s %s\n" , splitted[0] , splitted[1]);
            write(STDOUT_FILENO , temp , strlen(temp));
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "requests list printed\n");
    write_log(log_file , temp);
}


void print_ingredients_list(char * log_file, struct List ingredients)
{
    for (int i = 0 ; i < ingredients.count ; i++)
    {
        int count = 0;
        char temp[MAX_BUFFER_SIZE*2];
        char ** splitted = split_string(ingredients.datas[i] , '-' , &count);
        sprintf(temp , "    %s : %s\n" , splitted[0] , splitted[1]);
        write(STDOUT_FILENO , temp , strlen(temp));
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "ingredients printed\n");
    write_log(log_file , temp);
}

void print_recipes_list(char * log_file,Food foods[] , int num_of_foods)
{
    for (int i=0 ; i < num_of_foods ; i++)
    {
        char temp[MAX_BUFFER_SIZE*2];
        sprintf(temp , "%d- %s\n" , i+1 , foods[i].name);
        write(STDOUT_FILENO , temp , strlen(temp));
        for (int j = 0; j < foods[i].numIngredients; j++) 
        {
            sprintf(temp ,"         %s : %d\n", foods[i].ingredients[j].ingredient, foods[i].ingredients[j].quantity);
            write(STDOUT_FILENO , temp , strlen(temp));
        }
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "recipes printed\n");
    write_log(log_file , temp);
}

void print_menu(char * log_file,Food foods[] , int num_of_foods)
{
    for (int i=0 ; i < num_of_foods ; i++)
    {
        char temp[MAX_BUFFER_SIZE*2];
        sprintf(temp , "%d- %s\n" , i+1 , foods[i].name);
        write(STDOUT_FILENO , temp , strlen(temp));
    }
    char temp[MAX_BUFFER_SIZE*2];
    sprintf(temp , "menu printed\n");
    write_log(log_file , temp);
}
