// g++cmd:g++ -o LED_01 LED_01.c -Wall -I /home/metoro/rpi-rgb-led-matrix/include -L /home/metoro/rpi-rgb-led-matrix/lib -lrgbmatrix -lwiringPi -I /home/metoro/stb
//フォルダに入っている画像を1枚表示 
#define STB_IMAGE_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stb_image.h>
#include <led-matrix-c.h>
#include <wiringPi.h>
#include <errno.h>

char ext[] = ".png";
char path[] = "/home/metoro/led";

char **get_filename(char *, int *);
/*直接叩けばよくね？*/

int main(int argc, char **argv)
{
    /*変数の宣言*/
    char **list = NULL;
    unsigned char *pixel = NULL;
    int image_x = 0, image_y = 0, image_bpp = 0;
    int led_width = 0, led_height = 0, n = 0;
    struct RGBLedMatrixOptions options;
    struct RGBLedRuntimeOptions options_runtime;
    struct RGBLedMatrix *matrix_options;
    struct LedCanvas *offscreen_canvas;

    memset(&options, 0, sizeof(options));
    memset(&options_runtime, 0, sizeof(options_runtime));

    list = get_filename(path, &n);
    printf("%s\n", list[0]);
    if (list == NULL)
    {
        printf("画像もしくはディレクトリがありません\n");
        return 0;
    }
    pixel = stbi_load(list[0], &image_x, &image_y, &image_bpp, 3);
    if (pixel == NULL)
    {
        exit(10);
    }

    /*初期設定*/
    options.rows = 32; // パネルの行(デフォルト32)
    options.cols = 64; // パネルの列
    options.chain_length = 3;
    options.pwm_bits = 4;
    options_runtime.gpio_slowdown = 2;

    /*設定を構造体に格納 */
    matrix_options = led_matrix_create_from_options_and_rt_options(&options, &options_runtime);
    if (matrix_options == NULL)
    {
        return 1;
    }

    /*キャンバスの準備*/
    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix_options);
    led_canvas_get_size(offscreen_canvas, &led_width, &led_height);

    /*ここから画像を表示する処理*/
    printf("%s\n", list[0]);
    printf("%d,%d\n", led_height, led_width);

    for (int y = 0; y < led_height; y++)
    {
        for (int x = 0; x < led_width; x++)
        {
            n = (y * led_width + x) * 3;
            led_canvas_set_pixel(offscreen_canvas, x, y, pixel[n], pixel[n + 1], pixel[n + 2]);
        }
    }
    offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas);

    while(1);
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

        snprintf(filename_list[(*n) - 1], 257, "%s/%s", path, entry->d_name);
    }

    return filename_list;
}
