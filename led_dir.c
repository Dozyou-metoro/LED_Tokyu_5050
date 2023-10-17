#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *option;
    file_list *p_next;
} file_list;

file_list *panel_list_start = NULL;

void divide_option(int argc, char **argv, int *argc_panel, char ***argv_panel)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--") == 0) // LEDパネルのオプション
        {
        }
    }
}

void add_list(file_list *list_start, char *add_str)
{
    while (1)
    {
        if (!list_start) // リストに要素が無い場合
        {
            list_start = (file_list *)malloc(sizeof(file_list));
            list_start->option = add_str;
            list_start->p_next = NULL;
            return;
        }
        else
        {
            list_start = list_start->p_next;
        }
    }
}

void free_list(file_list *list_start)
{
    file_list p_next_buf = NULL;

    while (list_start)
    {
        p_next_buf = list_start->p_next;
        free(list_start);
        list_start = p_next_buf;
    }
}

file_list *access_list(file_list *list_start, size_t list_num)
{
    for (int i = 0; i < list_num; i++)
    {
        list_start=list_start->p_next;
        if(!list_start){
            return NULL;
        }
    }
}

char **change_list_array(file_list *list_start)
{
    size_t count = 0;
    while (list_start)
    {
        list_start = list_start->p_next;
        count++;
    }

    char **list_str = NULL;
    list_str = (char **)malloc(sizeof(char *) * count);

    for (int i = 0; i < count; i++)
    {
        list_str[i] = (char)malloc(strlen())
    }
}