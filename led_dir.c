#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <dirent.h>       //ディレクトリ関係
#include <string.h>       //文字列関係
#include <errno.h>        //エラー関係

// ディレクトリの中身を返す
char **get_dir_list(char *path, char *ext, size_t *dir_num) // ディレクトリ指定:extに"dir"を渡す
{
    // readdir()がらみの定数
    const int DIR_NO = 4, FILE_NO = 8;

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char **filename_list = NULL, **buf = NULL;

    *dir_num = 0;

    dir = opendir(path);

    if (dir == NULL)
    {
        return NULL;
    }

    while (1)
    {
        entry = readdir(dir);
        if (!entry)
        {
            break;
        }

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) //.を無視
        {
            continue;
        }

        if (strcmp(entry->d_name, ".git") == 0) //.gitを無視
        {
            continue;
        }

        if ((strcmp(ext, "dir") == 0) && (entry->d_type == FILE_NO)) // ディレクトリ指定ならファイルを無視
        {
            continue;
        }

        if ((strcmp(ext, "dir") != 0) && (entry->d_type == DIR_NO)) // ファイル指定ならディレクトリを無視
        {
            continue;
        }

        if ((ext != NULL) && (strcmp(ext, "dir") != 0)) // extがディレクトリ指定ではなく、NULLでもない
        {
            if (strstr(entry->d_name, ext) == NULL) // 指定した拡張子を含まなかったらcontinue
            {
                continue;
            }
        }

        // 条件に合うものが見つかったらカウンタを+1して、パスを配列に格納
        *dir_num = *dir_num + 1;
        buf = (char **)realloc(filename_list, *dir_num * sizeof(char *));
        if (buf == NULL)
        {
            exit(2);
        }
        filename_list = buf;
        buf = NULL;

        filename_list[(*dir_num) - 1] = (char *)calloc(256, sizeof(char)); // readdirはNAME_MAX(255)+1文字を返してくる
        if ((filename_list[(*dir_num) - 1]) == NULL)
        {
            exit(2);
        }

        strcpy(filename_list[(*dir_num) - 1], entry->d_name);
    }
    closedir(dir);
    return filename_list;
}

// リストをfree()
void filelist_free(char ***point, int dir_num)
{
    for (int i = 0; i < dir_num; i++)
    {
        free((*point)[i]);
    }
    free(*point);
    point = NULL;
}

// エラーメッセージを表示
void error_print(const char message[], int return_num)
{
    printf("%s,%s\n", strerror(errno), message);
    fflush(stdout);
    exit(return_num);
}

// コマンドライン引数を生成
void add_option(char *config_path, int *argc_copy, char ***argv_copy)
{
    FILE *fp = NULL;
    char file_path[256];

    sprintf(file_path, "%s/%s", config_path, "panel_config.ini");

    fp = fopen(file_path, "r");
    if (!fp)
    {
        error_print("panel_config.iniがありません。", 1);
    }

    char **pp_buf = NULL;

    for (int i = 1;; i++)
    {
        pp_buf = (char **)realloc(*argv_copy, sizeof(char *) * (i + 1));
        if (pp_buf)
        {
            *argv_copy = pp_buf;
            pp_buf = NULL;
        }
        else
        {
            error_print("mem_error", 2);
        }

        (*argv_copy)[i] = (char *)calloc(sizeof(char), 100);

        if (fscanf(fp, "%s", (*argv_copy)[i]) == EOF)
        {
            free((*argv_copy)[i]);
            *argc_copy = i;
            break;
        }
    }
}
