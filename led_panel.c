/*#define*/

#define STB_IMAGE_IMPLEMENTATION

/*#include*/

#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <string.h>       //文字列関係
#include <led-matrix-c.h> //パネル制御
#include <stb_image.h>    //画像を読み込む
#include <unistd.h>       //sleep()用
#include <wiringPi.h>     //delay()用
#include <stdint.h>       //uint8_t用

const int bpp = 3;

int panel_x = 192; // パネルの幅
int panel_y = 32;  // パネルの高さ

int offset = 8;       // 幕どうしの隙間
int scroll_time = 50; // スクロール速度(1段移動する速度をmsで指定)

uint8_t *image_buf = NULL; // 画像合成用バッファ
uint8_t *pixel_buf = 0;    // スクロール処理用バッファ

uint8_t *display_buf = NULL; // ディスプレイ表示用
uint8_t debug_mode = 0;

struct LedCanvas *offscreen_canvas;  // キャンバス(ライブラリの仕様のためグローバル変数にしざるを得ない)
struct RGBLedMatrix *matrix_options; // 関数がパネルの設定を入れる構造体(同上)
struct RGBLedMatrixOptions options;  // 設定を入れる構造体(同上)

void scroll_up_panel(void);
void scroll_down_panel(void);
void change_panel(void);
void print_dispray(void);

void panel_config(int *argc_copy, char ***argv_copy)
{
    matrix_options = led_matrix_create_from_options(&options, argc_copy, argv_copy); // 設定項目を反映させる
    if (matrix_options == NULL)
    {
        exit(1);
    }

    /*キャンバスの準備*/

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix_options); // キャンバスを生成
    led_canvas_get_size(offscreen_canvas, &panel_x, &panel_y);             // キャンバスの幅・高さを取得

    image_buf = (uint8_t *)calloc(sizeof(uint8_t), panel_x * panel_y * bpp);
    if (!image_buf)
    {
        exit(1);
    }

    pixel_buf = (uint8_t *)calloc(sizeof(uint8_t), panel_x * (panel_y * 3 + offset * 2) * bpp);
    if (!pixel_buf)
    {
        exit(1);
    }

    if (debug_mode == 1)
    {
        display_buf = (uint8_t *)calloc(sizeof(uint8_t), panel_x * panel_y * bpp);
        if (!pixel_buf)
        {
            exit(1);
        }
    }
}

void canvas_reset(void)
{
    memset(pixel_buf, 0, panel_x * panel_y * bpp);
}

// 画像を読んでimage_bufを更新
void print_canvas(char *filepath)
{
    // 画像を読み込んでCanvasに反映させる
    int image_x = 0, image_y = 0, image_bpp = 0;
    uint8_t *pixel = stbi_load(filepath, &image_x, &image_y, &image_bpp, bpp);
    if (!pixel)
    {
        exit(2);
    }

    for (int i = 0; i < panel_x * panel_y * bpp; i++)
    {
        if (pixel[i])
        {
            image_buf[i] = pixel[i]; // 黒色指定はスキップ
        }
    }
    stbi_image_free(pixel);
    pixel = NULL;
}

/*
    image_buf領域図(x=192,y=32と仮定)
     0---------------<panel_x行>--------------192
    0
    |
    <panel_y列>下スクロール先領域
    |
    32
    |
    <offset列>offset領域
    |
    40
    |
    <panel_y列>スクロール元領域
    |
    72
    |
    <offset列>offset領域
    |
    80
    |
    <panel_y列>上スクロール先領域
    |
    122

    */

// Canvasをパネルに反映
void print_panel(char scroll_type[])
{

    memcpy(&pixel_buf[panel_x * (panel_y + offset) * bpp], &image_buf[0], sizeof(uint8_t) * panel_x * panel_y * bpp);         // スクロール先をスクロール元へ移動
    memcpy(&pixel_buf[0], &image_buf[0], sizeof(uint8_t) * panel_x * panel_y * bpp);                                          // image_bufを下スクロール先領域に入れる
    memcpy(&pixel_buf[panel_x * (panel_y * 2 + offset * 2) * bpp], &image_buf[0], sizeof(uint8_t) * panel_x * panel_y * bpp); // image_bufを上スクロール先領域に入れる

    memset(image_buf, 0, sizeof(uint8_t) * panel_x * panel_y * bpp); // image_bufをリセット

    if (strcmp(scroll_type, "us") == 0) // 上スクロール指定
    {
        scroll_up_panel();
    }
    else if (strcmp(scroll_type, "ds") == 0) // 下スクロール指定
    {
        scroll_down_panel();
    }
    else if (strcmp(scroll_type, "c") == 0) // 即変更
    {
        change_panel();
    }
    else
    {
        // 何もしない(将来何か追加するかも)
    }
}

void scroll_up_panel(void)
{
    for (int i = 0; i <= panel_y + offset; i++) // スクロール処理
    {
        for (int y = 0; y < panel_y; y++)
        {
            for (int x = 0; x < panel_x; x++)
            {
                int n = (y * panel_x + x) * bpp;                // imageコピー用
                int m = (i + panel_y + offset) * panel_x * bpp; // スクロールのoffset用

                if (debug_mode != 1)
                {
                    led_canvas_set_pixel(offscreen_canvas, x, y, image_buf[n + m], image_buf[n + m + 1], image_buf[n + m + 2]); // キャンバスの座標x,yの色を設定
                }
                else
                {
                    for (int j = 0; j < 3; j++)
                    {
                        display_buf[n + j] = image_buf[n + m + j];
                    }
                }
            }
        }
        if (debug_mode != 1)
        {
            offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスをパネルに反映
        }
        else
        {
            print_dispray();
        }

        delay(scroll_time);
    }
}

void scroll_down_panel(void)
{
    for (int i = 0; i >= panel_y + offset; i--) // スクロール処理
    {
        for (int y = 0; y < panel_y; y++)
        {
            for (int x = 0; x < panel_x; x++)
            {
                int n = (y * panel_x + x) * bpp;                // imageコピー用
                int m = (i + panel_y + offset) * panel_x * bpp; // スクロールのoffset用

                if (debug_mode != 1)
                {
                    led_canvas_set_pixel(offscreen_canvas, x, y, image_buf[n + m], image_buf[n + m + 1], image_buf[n + m + 2]); // キャンバスの座標x,yの色を設定
                }
                else
                {
                    for (int j = 0; j < 3; j++)
                    {
                        display_buf[n + j] = image_buf[n + m + j];
                    }
                }
            }
        }
        if (debug_mode != 1)
        {
            offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスをパネルに反映
        }
        else
        {
            print_dispray();
        }

        delay(scroll_time);
    }
    delay(scroll_time);
}

void change_panel(void)
{
    int i = panel_y + offset;
    for (int y = 0; y < panel_y; y++)
    {
        for (int x = 0; x < panel_x; x++)
        {
            int n = (y * panel_x + x) * bpp;                // imageコピー用
            int m = (i + panel_y + offset) * panel_x * bpp; // スクロールのoffset用

            if (debug_mode != 1)
            {
                led_canvas_set_pixel(offscreen_canvas, x, y, image_buf[n + m], image_buf[n + m + 1], image_buf[n + m + 2]); // キャンバスの座標x,yの色を設定
            }
            else
            {
                for (int j = 0; j < 3; j++)
                {
                    display_buf[n + j] = image_buf[n + m + j];
                }
            }
        }
    }
    if (debug_mode != 1)
    {
        offscreen_canvas = led_matrix_swap_on_vsync(matrix_options, offscreen_canvas); // キャンバスをパネルに反映
    }
    else
    {
        print_dispray();
    }

    delay(scroll_time);
}

void print_dispray(void)
{
    printf("\e[J"); // 画面リセット
    for (int y = 0; y < panel_y; y++)
    {
        for (int x = 0; x < panel_x; x++)
        {
            int n = (y * panel_x + x) * bpp; // imageコピー用
            printf("\e[%dm　", 40 + (display_buf[n] != 0) + (display_buf[n + 1] != 0) * 2 + (display_buf[n + 2] != 0) * 4);
        }
        printf("\e[0m\n");
        
    }
    printf("\e[32F");
    
}