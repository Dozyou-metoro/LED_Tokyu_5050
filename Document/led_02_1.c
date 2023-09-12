#define STB_IMAGE_IMPLEMENTATION
// g++cmd:g++ -o led_02_1 led_02_1.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -lwiringPi -I /home/metoro/stb

#include <stdlib.h>       //メモリ関係
#include <stdio.h>        //標準入出力
#include <string.h>       //文字列関係
#include <dirent.h>       //ディレクトリ関係
#include <stb_image.h>    //画像を読み込む
#include <led-matrix-c.h> //パネル制御
#include <wiringPi.h>     //delay()用
#include <errno.h>        //エラー関係

char ext[] = ".png";
char path[] = "/home/metoro/led";

char **get_filename(char *, int *);

int main(/*int argc, char **argv*/ void)
{
    /*変数の宣言*/
    char **list = NULL;                                 // ファイルのパスを入れる配列
    unsigned char *pixel = NULL;                        // 画像のデータを入れる配列
    int image_x = 0, image_y = 0, image_bpp = 0, n = 0; // 画像の幅、高さ、ピクセルあたりサイズ、処理用変数
    int file_num = 0;                                   // 見つかった画像の数
    struct RGBLedMatrixOptions options;                 // 設定を入れる構造体その1
    struct RGBLedRuntimeOptions options_run;            // 設定を入れる構造体その2
    struct RGBLedMatrix *matrix_options;                // 関数がパネルの設定を入れる構造体その1
    struct LedCanvas *offscreen_canvas;                 // キャンバス
    int led_width = 0, led_height = 0;

    memset(&options, 0, sizeof(options));
    memset(&options_run, 0, sizeof(options_run));

    list = get_filename(path, &file_num); // 指定ディレクトリから画像のパスを取得

    if (list == NULL)
    {
        printf("画像もしくはディレクトリがありません\n");
        return 0;
    }

    /*初期設定*/
    options.rows = 32;                // パネルの行(デフォルト32)
    options.cols = 64;                // パネルの列
    options.chain_length = 3;         // パネルの連結数
    options.pwm_bits = 4;             // PWMのビット(高くすると表現できる色の種類が、低くすると動作の安定度が上がる)
    options_run.drop_privileges = -1; // root権限で動作
    options_run.gpio_slowdown = 2;    // GPIO速度を設定

    matrix_options = led_matrix_create_from_options_and_rt_options(&options, &options_run); // 関数に設定を渡して構造体に格納してもらう

    if (matrix_options == NULL)
    {
        exit(1);
    }
    /*キャンバスの準備*/
    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix_options); // 描画用キャンバスを用意する
    led_canvas_get_size(offscreen_canvas, &led_width, &led_height);        // キャンバスの大きさを返す

    /*設定を構造体に格納 */
    while (1)
    {
        for (int i = 0; i < file_num; i++)
        {
            pixel = stbi_load(list[i], &image_x, &image_y, &image_bpp, 3); // 画像を読み込み
            if (pixel == NULL)
            {
                exit(10);
            }

            for (int y = 0; y < led_height; y++)
            {
                for (int x = 0; x < led_width; x++)
                {
                    led_canvas_set_pixel(offscreen_canvas, x, y, 0, 0, 0); // キャンバスの座標x,yの色を(0,0,0)に設定
                }
            }

            for (int y = 0; y < image_y; y++)
            {
                for (int x = 0; x < image_x; x++)
                {
                    n = (y * image_x + x) * 3;
                    led_canvas_set_pixel(offscreen_canvas, x, y, pixel[n], pixel[n + 1], pixel[n + 2]); // 座標(x,y)のrgb値を設定
                }
            }
            offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスを渡してパネルに反映(返してくるキャンバスのポインタを保持すること)

            delay(3000);
        }
    }
}

char **get_filename(char *path, int *n)
{
    /*変数の宣言*/
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char **filename_list = NULL, **buf = NULL;

    dir = opendir("/home/metoro/led"); // ディレクトリを開く
    if (dir == NULL)
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
        exit(9);
    }

    /*指定した拡張子のファイルを抜粋*/
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        { //.を無視
            continue;
        }

        if (strstr(entry->d_name, ext) == NULL)
        {
            continue;
        }

        *n = *n + 1; // 見つかったファイルの数を+1
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

        snprintf(filename_list[(*n) - 1], 257, "%s/%s", path, entry->d_name);
    }

    return filename_list;
}
