// g++cmd:g++ -o led_03 led_03.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -I /home/metoro/stb

#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <dirent.h>       //ディレクトリ関係
#include <string.h>       //文字列関係
#include <errno.h>        //エラー関係
#include <led-matrix-c.h> //パネル制御
#include <stb_image.h>    //画像を読み込む
#include <unistd.h>       //sleep()用

const uint8_t add_number = 6;

char argv_1[] = "--led-slowdown-gpio=2";
char argv_2[] = "--led-no-drop-privs";
char argv_3[] = "--led-cols=64";
char argv_4[] = "--led-rows=32";
char argv_5[] = "--led-chain=3";
char argv_6[] = "--led-pwm-bits=4";

char path[] = "/home/metoro/led";

int wait_time = 5; // スライドの待機時間

char **get_filename(char *path, int *n);

int main(int argc, char **argv)
{

    int argc_copy = 0;                           // コマンドライン引数コピー用
    char **argv_copy = NULL;                     // コマンドライン引数コピー用
    char **file_list = NULL;                     // ファイル名一覧
    int file_num = 0;                            // ファイルの数
    int led_width = 0, led_height = 0;           // パネルの幅。高さ
    int image_x = 0, image_y = 0, image_bpp = 0; // 画像の幅・高さ
    int n = 0;                                   // 画像関係の処理の使う
    char *buf = NULL;                            // ファイルパス関係の処理

    struct RGBLedMatrix *matrix_options; // 関数がパネルの設定を入れる構造体
    struct RGBLedMatrixOptions options;  // 設定を入れる構造体
    struct LedCanvas *offscreen_canvas;  // キャンバス

    unsigned char *pixel = NULL; // 画像を格納

    memset(&options, 0, sizeof(options));

    buf = (char *)malloc(sizeof(char) * 256);
    if (buf == NULL)
    {
        exit(4);
    }

    /*初期設定*/

    if (argc == 1) // 引数なしで起動された場合は初期設定を補完
    {
        argc_copy = add_number + 1;
        argv_copy = (char **)malloc(sizeof(char **) * argc_copy);
        if (argv_copy == NULL)
        {
            exit(1);
        }
        argv_copy[1] = argv_1;
        argv_copy[2] = argv_2;
        argv_copy[3] = argv_3;
        argv_copy[4] = argv_4;
        argv_copy[5] = argv_5;
        argv_copy[6] = argv_6;
    }
    else if (strcmp(argv[1], "-add") == 0) //-add付きで起動された場合は入力された引数に初期設定を追加
    {
        argc_copy = argc + add_number;
        argv_copy = (char **)malloc(sizeof(char **) * argc_copy);
        if (argv_copy == NULL)
        {
            exit(1);
        }
        argv_copy[argc] = argv_1;
        argv_copy[argc + 1] = argv_2;
    }
    else // 引数付きで起動した場合は補完なし
    {
        argc_copy = argc;
        argv_copy = (char **)malloc(sizeof(char **) * argc_copy);
        if (argv_copy == NULL)
        {
            exit(1);
        }
        for (int i = 0; i < argc; i++)
        {
            argv_copy[i] = argv[i];
        }
    }

    /*ファイル名を取得*/

    file_list = get_filename(path, &file_num);
    if (file_list == NULL)
    {
        exit(11);
    }

    /*パネルの初期設定*/

    matrix_options = led_matrix_create_from_options(&options, &argc_copy, &argv_copy); // 設定項目を反映させる
    if (matrix_options == NULL)
    {
        exit(1);
    }

    /*キャンバスの準備*/

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix_options); // キャンバスを生成
    led_canvas_get_size(offscreen_canvas, &led_width, &led_height);        // キャンバスの幅・高さを取得

    /*ここから画像を表示する処理*/
    while (1)
    {
        for (int i = 0; i < file_num; i++)
        {
            /*一枚目の画像を表示*/
            snprintf(buf, 257, "%s/%s", path, file_list[i]);           // フルパスを生成
            pixel = stbi_load(buf, &image_x, &image_y, &image_bpp, 3); // 画像を読み込み
            if (pixel == NULL)
            {
                exit(10);
            }

            for (int y = led_height - 1; y >= 0; y--)
            {
                for (int x = led_width - 1; x >= 0; x--)
                {
                    n = (y * led_width + x) * 3;
                    led_canvas_set_pixel(offscreen_canvas, x, y, pixel[n], pixel[n + 1], pixel[n + 2]); // キャンバスの座標x,yの色を設定
                }
            }
            stbi_image_free(pixel);                                                        // メモリを解放
            offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスをパネルに反映

            sleep(wait_time);

            int option_num = 0;
            int option_sintyoku = 0;
            int option_long = 0;
            char buf_copy[256];
            char buf_copy_ext[256];

            option_num = 0;
            for (int j = 0; j < 257; j++) // オプションの数をカウント
            {
                if (file_list[i][j] == '-')
                {
                    option_num++;
                }
            }

            for (int j = 0; j < option_num; j++)
            {
                /*二枚目以降の表示*/

                for (option_sintyoku; 1; option_sintyoku++) // オプションが何文字目から始まるかチェック
                {
                    if (file_list[i][option_sintyoku] == '-')
                    {
                        break;
                    }
                }

                for (option_long = 1; 1; option_long++) // オプションの長さをチェック
                {
                    if ((file_list[i][option_sintyoku + option_long] == '-') || file_list[i][option_sintyoku + option_long] == '.')
                    {
                        break;
                    }
                }

                for (int k = 0; k < option_long - 1; k++) // オプションをコピー
                {
                    buf_copy[k] = file_list[i][option_sintyoku + k + 1];
                }
                buf_copy[option_long - 1] = '\0';

                snprintf(buf_copy_ext, 257, "%s_.png", buf_copy); // 拡張子を追加
                snprintf(buf, 257, "%s/%s", path, buf_copy_ext);  // フルパスを生成

                option_sintyoku = option_sintyoku + option_long; // オプションの読み込み具合を更新

                /*画像を表示*/
                printf("%s\n", buf);
                pixel = stbi_load(buf, &image_x, &image_y, &image_bpp, 3); // 画像を読み込み
                if (pixel == NULL)
                {
                    exit(10);
                }

                for (int y = 0; y < led_height; y++)
                {
                    for (int x = 0; x < led_width; x++)
                    {
                        n = (y * led_width + x) * 3;
                        led_canvas_set_pixel(offscreen_canvas, x, y, pixel[n], pixel[n + 1], pixel[n + 2]); // キャンバスの座標x,yの色を設定
                    }
                }
                stbi_image_free(pixel);                                                        // メモリを解放
                offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスをパネルに反映

                sleep(wait_time);
            }
        }
    }
}

char **get_filename(char *path, int *n) // ファイル名を取得
{
    /*変数の宣言*/
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char **filename_list = NULL, **buf = NULL;
    char ext[] = ".png"; // 抜粋する拡張子

    /* str = (char*)malloc(sizeof(char) * 258);
    if (str == NULL)
    {
        exit(3);
    }*/

    dir = opendir(path); // ディレクトリを開く
    if (dir == NULL)     // エラーメッセージ集
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

    /*指定した拡張子のファイルを抜粋*/
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) //.を無視
        {
            continue;
        }

        if (strstr(entry->d_name, ext) == NULL) // 指定した拡張子を含まなかったらcontinue
        {
            continue;
        }

        if (strstr(entry->d_name, "_.") != NULL) // "_."を含む(他の幕とセット指定)ならcontinue
        {
            continue;
        }

        // カウンタを+1して、パスを配列に格納
        *n = *n + 1;
        buf = (char **)realloc(filename_list, *n * sizeof(char *));
        if (buf == NULL)
        {
            exit(1);
        }
        filename_list = buf;
        buf = NULL;

        filename_list[(*n) - 1] = (char *)calloc(257, sizeof(char));
        if ((filename_list[(*n) - 1]) == NULL)
        {
            exit(2);
        }

        // snprintf(filename_list[(*n) - 1], 257, "%s/%s", path, entry->d_name);
        snprintf(filename_list[(*n) - 1], 257, "%s", entry->d_name);
    }

    return filename_list;
}
