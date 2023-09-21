#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <led-matrix-c.h> //パネル制御
#include <stb_image.h>    //画像を読み込む
#include <unistd.h>       //sleep()用
#include <wiringPi.h>     //delay()用

// 画像を読んでCanvasを更新
void print_canvas(char *filepath)
{
    // 画像を読み込んでCanvasに反映させる
    printf("%s\n", filepath);
}

// Canvasをパネルに反映
void print_panel(void)
{
    // Canvasをパネルに反映する
    // スクロール処理はここ？ファイル分割も視野に
}