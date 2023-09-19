// g++cmd:g++ -o LED_Tokyu_5050 LED_Tokyu_5050.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -I /home/metoro/stb -lwiringPi -g -O0

/*
ディレクトリ構成メモ

main    led----------------
        |                 |
車両    Tokyu------       Mettetsu
        |         |
連番    1-----   ,2----- ,   
　　    |    |    |    |
画像    png  png  png  png


*/
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <dirent.h>       //ディレクトリ関係
#include <string.h>       //文字列関係
#include <errno.h>        //エラー関係
#include <led-matrix-c.h> //パネル制御
#include <stb_image.h>    //画像を読み込む
#include <unistd.h>       //sleep()用
#include <wiringPi.h>     //delay()用
#include <time.h>

struct LedCanvas *offscreen_canvas;  // キャンバス(ライブラリの仕様のためグローバル変数にしざるを得ない)
struct RGBLedMatrix *matrix_options; // 関数がパネルの設定を入れる構造体(同上)
struct RGBLedMatrixOptions options;  // 設定を入れる構造体(同上)

char **get_file_list(char *path, char *ext, int *file_num);
void filelist_free(char **point, int file_num);

char maindir[] = "/home/metoro/led/";

int main(void)
{
    // 初期設定変数
    char argv_add_tmp[][256] = {"--led-slowdown-gpio=2", "--led-no-drop-privs", "--led-cols=64", "--led-rows=32", "--led-chain=3", "--led-pwm-bits=4", "--led-show-refresh", "--led-limit-refresh=120"}; // 補完するオプション

    char ext_dir[] = "dir";

    // 初期設定
    memset(buf, 0, sizeof(buf));
    srand((unsigned int)time(void));
}

char **get_file_list(char *path, char *ext, int *file_num) // ディレクトリ指定:extに"dir"を渡す
{
    // readdir()がらみの定数
    const int DIR_NO = 4, FILE_NO = 8;

    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char **filename_list = NULL, **buf = NULL;

    *file_num = 0;

    dir = opendir(path);

    if (dir == NULL) // エラーメッセージ集
    {
        if (errno == ENOENT)
        {
            printf("ディレクトリが存在しません。\n");
        }
        else if (errno == EACCES)
        {
            printf("ディレクトリにアクセスできません。\n");
        }
        else
        {
            printf("ディレクトリのオープンに失敗しました。エラーコード: %d\n", errno);
        }
        fflush(stdout);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) //.を無視
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

        if ((ext != NULL) && (strcmp(ext, "dir") == 0)) // extがディレクトリ指定ではなく、NULLでもない
        {
            if (strstr(entry->d_name, ext) == NULL) // 指定した拡張子を含まなかったらcontinue
            {
                continue;
            }
        }

        // 条件に合うものが見つかったらカウンタを+1して、パスを配列に格納
        *file_num = *file_num + 1;
        buf = (char **)realloc(filename_list, *file_num * sizeof(char *));
        if (buf == NULL)
        {
            exit(1);
        }
        filename_list = buf;
        buf = NULL;

        filename_list[(*file_num) - 1] = (char *)calloc(256, sizeof(char)); // readdirはNAME_MAX(255)+1文字を返してくる
        if ((filename_list[(*file_num) - 1]) == NULL)
        {
            exit(2);
        }

        strcpy(filename_list[(*file_num) - 1], entry->d_name);
    }
}

void filelist_free(char **point, int file_num)
{
    for (int i = 0; i < file_num; i++)
    {
        free(point[i]);
    }
    free(point);
}