// g++cmd:g++ -o LED_Tokyu_5050 LED_Tokyu_5050.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -I /home/metoro/stb -lwiringPi -g -O0

/*
ディレクトリ構成メモ

main    led----------------
        |                 |
車両    Tokyu------       Mettetsu
        |         |
連番    1-----   ,2----- ,
　　    |    |    |    |
分類    種別 行先 種別 行先
        |
画像    png


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

int led_x = 0, led_y = 0;

char **get_dir_list(char *path, char *ext, int *dir_num);
void filelist_free(char **point, int dir_num);
void error_print(char[] message, int return_num);

// 初期設定変数

char maindir[] = "/home/metoro/led/";

// 補完するオプション
char argv_add_tmp[][256] = {"--led-slowdown-gpio=2", "--led-no-drop-privs", "--led-cols=64", "--led-rows=32", "--led-chain=3", "--led-pwm-bits=4", "--led-show-refresh", "--led-limit-refresh=120"};

char ext_dir[] = "dir";
char ext_png[] = ".png";

int main(int argc, char **argv)
{

    // プログラム用変数
    int dir_num = 0;
    int file_num = 0;
    int rand_num = 0;

    char dir_path_buf[256];
    char dir_path_1[256]; // 選択された車両のパスを入れる
    char dir_path_2[256]; // 現在選択されている連番幕のパスを入れる
    char file_path[256];

    char **dir_list = NULL;
    char **file_list = NULL;

    char **argv_add = NULL;
    int argc_copy = 0;       // コマンドライン引数コピー用
    char **argv_copy = NULL; // コマンドライン引数コピー用

    /*初期設定*/

    // 変数の初期化
    memset(dir_path, 0, sizeof(dir_path));
    memset(dir_path_buf, 0, sizeof(dir_path));

    // seedの更新
    srand((unsigned int)time(void));

    // パネルの設定
    argv_add = (char **)malloc(sizeof(argv_add_tmp) / sizeof(*argv_add_tmp) * sizeof(char *)); // コマンドライン引数を生成
    for (int i = 0; i < (int)(sizeof(argv_add_tmp) / sizeof(*argv_add_tmp)); i++)
    {
        argv_add[i] = &argv_add_tmp[i][0];
    }

    add_option(&argc_copy, &argv_copy, argc, argv, sizeof(argv_add_tmp) / sizeof(*argv_add_tmp), argv_add); // コマンドライン引数を補完

    /*パネルの初期設定*/

    matrix_options = led_matrix_create_from_options(&options, &argc_copy, &argv_copy); // 設定項目を反映させる
    if (matrix_options == NULL)
    {
        exit(1);
    }

    /*キャンバスの準備*/

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix_options); // キャンバスを生成

    /*画像バッファの準備*/

    image_buf = (unsigned char *)calloc(sizeof(unsigned char), led_width * led_height * 4 * 3 + offset * led_width * 4 * 2);
    if (image_buf == NULL)
    {
        exit(1);
    }

    /*ここからメイン処理*/

    while (1)
    {
        // 車両を選択
        dir_list = get_dir_list(maindir, ext_dir, &dir_num); // /home/metoro/led/ここを読む(車両)
        if (!dir_list)
        {
            error_print("車両データがありません。", 1);
        }

        rand_num = rand() % dir_num;
        sprintf(dir_path_1, "%s", dir_list[rand_num]);
        filelist_free(&dir_list, dir_num);

        // 幕を選択

        // 連番幕の処理
        for (int i = 0;; i++)
        {
            sprintf(dir_path_2, "%s/%d", dir_path_1, i);
            dir_list = get_dir_list(dir_path_2, ext_dir, &dir_num); // /home/metoro/led/Tokyu/n番/ここを読む(種別等)
            if (!dir_list)
            {
                if (i == 0)
                {
                    error_print("幕データが見つかりません", 1);
                }
                else
                {
                    break; // 連番幕の読み込みが終了
                }
            }

            // 種別幕、行先幕等を呼んでいく
            for (int j = 0; j < dir_num; j++)
            {
                sprintf(file_path, "%s/%s", dir_path_2, dir_list[j]);
                file_list = get_dir_list(file_path, ext_png, &file_num); // /home/metoro/led/Tokyu/n番/種別or行先/ここを読む(幕データ)
                if (!file_list)
                {
                    error_print("幕データが見つかりません", 1);
                }

                rand_num = rand() % file_num;
                sprintf(file_path, "%s/%s", dir_list[j], file_list[rand_num]);

                print_canvas(file_path);

                filelist_free(file_list);
            }

            print_panel();
        }
    }
}

// ディレクトリの中身を返す
char **get_dir_list(char *path, char *ext, int *dir_num) // ディレクトリ指定:extに"dir"を渡す
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
        *dir_num = *dir_num + 1;
        buf = (char **)realloc(filename_list, *dir_num * sizeof(char *));
        if (buf == NULL)
        {
            exit(1);
        }
        filename_list = buf;
        buf = NULL;

        filename_list[(*dir_num) - 1] = (char *)calloc(256, sizeof(char)); // readdirはNAME_MAX(255)+1文字を返してくる
        if ((filename_list[(*dir_num) - 1]) == NULL)
        {
            exit(2);
        }

        strcpy(filename_list[(*dir_num) - 1], entry->d_name);

        closedir(dir);
    }
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
void error_print(char[] message, int return_num)
{
    printf("%s,%s\n", strerror(errno), message);
    fflush(stdout);
    exit(return_num);
}

// 画像を読んでCanvasを更新
void print_canvas(char *filepath)
{
    // 画像を読み込んでCanvasに反映させる
    printf("%s\n",filepath);
}

// Canvasをパネルに反映
void print_panel(void)
{
    // Canvasをパネルに反映する
}

// コマンドライン引数を生成
void add_option(int *argc_copy, char ***argv_copy, int argc, char **argv, int argc_add, char **argv_add)
{
    int add_flug = 0;
    if (strcmp(argv[argc - 1], "-add") == 0) // 補完なしを指定された場合
    {
        add_flug = 1;
        argc_add = 0;
    }

    *argc_copy = argc - add_flug + argc_add; // オプション追加後のargcを計算
    *argv_copy = (char **)malloc(sizeof(char *) * (*argc_copy));
    if (*argv_copy == NULL)
    {
        exit(2);
    }

    for (int i = 0; i < argc - add_flug; i++)
    {
        (*argv_copy)[i] = argv[i];
        fflush(stdout);
    }

    for (int i = argc; i < *argc_copy; i++)
    {
        (*argv_copy)[i] = argv_add[i - argc - add_flug];
    }
}
