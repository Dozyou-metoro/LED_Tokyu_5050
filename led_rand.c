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

/*#define*/

#define STB_IMAGE_IMPLEMENTATION

/*#include*/

#include <stdio.h>        //標準入出力
#include <stdlib.h>       //メモリ関係
#include <dirent.h>       //ディレクトリ関係
#include <string.h>       //文字列関係
#include <errno.h>        //エラー関係
#include <time.h>         //rand()用
#include <led-matrix-c.h> //パネル制御

/*構造体宣言*/

struct LedCanvas *offscreen_canvas;  // キャンバス(ライブラリの仕様のためグローバル変数にしざるを得ない)
struct RGBLedMatrix *matrix_options; // 関数がパネルの設定を入れる構造体(同上)
struct RGBLedMatrixOptions options;  // 設定を入れる構造体(同上)

typedef struct
{
    char maindir[512];
    char dir_path_1[756];
    char dir_path_2[759];
    char file_path[1024];

} file_path;

/*グローバル変数*/

// 拡張子指定用
char ext_dir[] = "dir";
char ext_png[] = ".png";

/*プロトタイプ宣言*/

char **get_dir_list(char *, char *, size_t *);
void filelist_free(char ***, int);
void error_print(const char[], int);
void add_option(char *, int *, char ***);
void print_canvas(char *);
void print_panel(void);

int main(int argc, char **argv)
{

    /*プログラム用変数*/

    file_path path; // pathを入れる

    size_t dir_num = 0; // file_path構造体とセットで使用
    size_t file_num = 0;

    size_t rand_num = 0; // rand()を処理した結果を入れる

    char maindir[1024] = "/home/metoro/led"; // 指定されなかった時のデフォルトパス

    char **dir_list = NULL; // get_filepath()を受けるバッファ
    char **file_list = NULL;

    int argc_copy = 0; // コマンドライン引数を入れる
    char **argv_copy = NULL;

    /*初期設定*/

    // 変数の初期化
    memset(path.dir_path_1, 0, sizeof(path.dir_path_1));
    memset(path.dir_path_2, 0, sizeof(path.dir_path_2));
    memset(path.file_path, 0, sizeof(path.file_path));

    // seedの更新
    srand((unsigned int)time(NULL));

    // panel_config.iniの位置を指定されていたら上書き
    if (argc > 1)
    {
        strcpy(maindir, argv[1]);
    }

    // ライブラリに渡すコマンドライン引数を読み込み
    add_option(maindir, &argc_copy, &argv_copy);

    /*ここからメイン処理*/

    while (1)
    {
        // 車両を選択
        dir_list = get_dir_list(maindir, ext_dir, &dir_num); // /home/metoro/led/ここを読む(車両)
        if (dir_list == NULL)
        {
            error_print("車両データがありません。", 1);
        }

        rand_num = rand() % dir_num;
        sprintf(path.dir_path_1, "%s/%s", maindir, dir_list[rand_num]);
        filelist_free(&dir_list, dir_num);

        // 幕を選択

        // 連番幕の処理
        for (int i = 0;; i++)
        {
            sprintf(path.dir_path_2, "%s/%d", path.dir_path_1, i);
            dir_list = get_dir_list(path.dir_path_2, ext_dir, &dir_num); // /home/metoro/led/Tokyu/n番/ここを読む(種別等)
            if (dir_list == NULL)
            {
                if (i == 0)
                {
                    error_print("幕データが見つかりません", 1);
                }
                else
                {
                    break; // 連番幕の読み込みが終了
                    print_canvas(path.dir_path_2);
                }
            }

            // 種別幕、行先幕等を呼んでいく
            for (size_t j = 0; j < dir_num; j++)
            {
                sprintf(path.file_path, "%s/%s", path.dir_path_2, dir_list[j]);
                file_list = get_dir_list(path.file_path, ext_png, &file_num); // /home/metoro/led/Tokyu/n番/種別or行先/ここを読む(幕データ)
                if (file_list == NULL)
                {
                    error_print("幕データが見つかりません", 1);
                }

                rand_num = rand() % file_num;
                sprintf(path.file_path, "%s/%s/%s", path.dir_path_2, dir_list[j], file_list[rand_num]);

                print_canvas(path.file_path);

                filelist_free(&file_list, file_num);
            }

            filelist_free(&dir_list, dir_num);
            print_panel();
        }
    }
}

