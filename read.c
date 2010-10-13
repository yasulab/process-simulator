#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STR_MAX 256 /* 文字列入力用配列長 */
#define LINE_MAX 30

int readProgram(char *fname, char prog[][STR_MAX]){
  FILE *fp;          /* ファイルポインタ用 */
  //char fname[80];    /* ファイル名用 */
  char buff[STR_MAX], *pp;    /* 文字列用 */
  //char prog[LINE_MAX][STR_MAX];

  //printf("ファイル名 : ");    /* プロンプト表示 */
  //gets(fname);              /* ファイル名入力 */
  fp = fopen(fname, "r");    /* ファイルオープン */
  if(fp == NULL){            /* オープン失敗 */
    printf("ファイルがオープンできません\n");
    exit(1);               /* 強制終了 */
  }

  int i=0;
  while(1){    /* 永久ループ */
    pp = fgets(buff, STR_MAX, fp);    /* 1行読み込み */
    strcpy(prog[i], buff);
    if(pp == NULL){    /* 読み込み終了 */
      break;           /* ループ脱出 */
    }
    //printf("%s", buff);    /* 1行表示 */
    i++;
  }

  fclose(fp);    /* ファイルクローズ */
  return(0);
}

int main(){
  char *p = "prog1";
  char prog[LINE_MAX][STR_MAX];
  readProgram(p, prog);
  int i=0;
  while(strcmp(prog[i],"E\n")){
    printf("prog[%d]=%s", i, prog[i]);
    i++;
  }
  printf("prog[%d]=%s", i, prog[i]);
}
