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

Todo
リスト構造を採用
realloc()の削減
ファイルパス周りの文字数を管理


*/

/*#define*/

#define STB_IMAGE_IMPLEMENTATION

/*プロトタイプ宣言*/

void divide_option(int argc, char **argv, int *argc_panel, char ***argv_panel);

int main(int argc, char **argv)
{

    int argc_panel = 0;
    char **argv_panel = NULL;

    divide_option(argc, argv, &argc_panel, &argv_panel);
}