#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MILLI_SECOND 1000000 //nano秒単位でmsを表現
#define N 10000

typedef struct mass_point{
    double x0; //初期位置x座標
    double y0; //初期位置y座標
    double x;  //x座標
    double y;  //y座標
    double v0; //初速度
    double v;  //速度
    double vx; //x方向速度成分
    double vy; //y方向速度成分
    double angle; //仰角（角度）
    double angle_rad; //仰角（弧度）
    double pt;//経過時間
}MASS;

void calc(MASS [], MASS [], double, double, double, int, int*);//関数calcのプロトタイプ宣言
void graph_draw1(MASS[], MASS[], int, struct timespec);
void graph_draw2(MASS[], MASS[], int, struct timespec);
void graph_draw3(MASS[], MASS[], int, struct timespec);
void data_write(MASS[], MASS[], int);

int main(void)
{
    FILE *fp, *gp;

    struct timespec req={0, 50 * MILLI_SECOND};//50 ms時間待ち
//    struct timespec req={0, 20 * MILLI_SECOND};//20 ms時間待ち

    int total = 0;//計算回数

    MASS monkey[N];//のデータ
    MASS ball[N];//弾丸のデータ

    //弾丸の初期値
    double ball_v0 = 40;  //初速度
    double ball_vx0, ball_vy0; //初速度x,y成分
    double ball_y0 = 0;      //y座標
    double ball_x0 = 0;      //x座標
    double ball_angle = 30;   //弾丸の仰角
    //の初期値
    double monkey_v0 = 0.0; //サルの初速度
    double monkey_vy0 = 0.0;
    double monkey_vx0 = 0.0;
    double monkey_y0 = 40;      //猿の初期位置（垂直方向)
    double monkey_x0 = 40;      //猿の初期位置（水平方向）

    double t = 0.0;
    double ts = 0.005;      //時間分割
    double gravity = 9.8;       //重力加速度
    double ref = -0.4;//反発係数

    ball_vx0 = ball_v0 * cos(ball_angle*M_PI/180.0);
    ball_vy0 = ball_v0 * sin(ball_angle*M_PI/180.0);

    ball[0].v0 = ball_v0;
    ball[0].y = ball[0].y0 = ball_y0;
    ball[0].x = ball[0].y0 = ball_x0;
    ball[0].angle = ball_angle;
    ball[0].angle_rad = ball_angle*M_PI/180.0;
    ball[0].vx = ball_vx0;
    ball[0].vy = ball_vy0;
    ball[0].pt = 0.0;

    monkey[0].v0 = monkey_v0;
    monkey[0].vy = monkey_vy0;
    monkey[0].vx = monkey_vx0;
    monkey[0].y = monkey_y0;
    monkey[0].x0 = monkey_x0;
    monkey[0].x = monkey_x0;
    monkey[0].pt = 0.0;

    calc(ball, monkey, gravity, ref, ts, N, &total);
    graph_draw1(ball, monkey, total, req);//アニメーション関数１<-使う場合なgraph_draw2,3をコメント文にする
    //graph_draw2(ball, monkey, total, req);//アニメーション関数２<-使う場合はgraph_draw1,3をコメント文にする
    //graph_draw3(ball, monkey, total, req);//アニメーション関数３<-使う場合はgraph_draw1,2をコメント文にする

    printf("データをファイルに書き込み保存\n");
    data_write(ball, monkey, total);

    return 0;
}


//モンキーハンティングのシミュレーション関数
//引数１：弾丸
//引数２：サル
//引数３：重力加速度
//引数４：反発係数
//引数５：時間間隔
//引数６：配列の要素数
//引数７：計算回数を返すためのポインタ変数
void calc(MASS b[], MASS m[], double g, double r, double ts, int n, int *total)
{
int monkey_bound = 0;
int ball_bound = 0;
int count=0;
int flg1 = 0;
int i = 1;
double t = 0.0;
do{
b[i].x = b[i-1].x + b[i-1].vx * ts;   //弾丸のx座標の計算
b[i].y = b[i-1].y + b[i-1].vy * ts;   //弾丸のy座標の計算
b[i].vx = b[i-1].vx;                  //弾丸のx方向速度の計算
b[i].vy = b[i-1].vy - g * ts;         //弾丸のy方向速度の計算
m[i].x = m[i-1].x + m[i-1].vx * ts;   //サルのx座標の計算
m[i].vx = m[i-1].vx;                  //サルのx方向速度の計算
m[i].y = m[i-1].y + m[i-1].vy * ts;   //サルのy方向位置の計算
m[i].vy = m[i-1].vy - g * ts;         //サルのy方向速度の計算
double diff_x = b[i].x - m[i].x;//弾丸とサルの水平位置の差
double diff_y = b[i].y - m[i].y;//弾丸とサルの垂直位置の差
if(diff_x < 0.0) diff_x *= -1.0;//差の絶対値
if(diff_y < 0.0) diff_y *= -1.0;//差の絶対値
//弾丸とサルとの衝突判定うまく判定できない場合にはdiff_xとdiff_yの比較条件（値）を変更すること
if(count > 10 && diff_x < 0.5 && diff_y < 0.5){
b[i].vx = b[i].vx * r + m[i].vx * r;
m[i].vx = m[i].vx * r + b[i].vx * r;
b[i].vy = b[i].vy * r + m[i].vy * r;
m[i].vy = m[i].vy * r + b[i].vy * r;
}
//弾丸と地面との衝突判定
if(b[i].y <= 0.0 && b[i].vy < 0.0){
b[i].vy  *= r;
b[i].vx -= b[i].v0/1000.0;//横方向の地面との摩擦を適当に
ball_bound++;
}
//サルと地面との衝突判定
if(m[i].y <= 0 && m[i].vy < 0){
m[i].vy *= r;
m[i].vx -= m[i-1].vx/1000;//横方向の地面との摩擦を適当に
monkey_bound++;
}
b[i].pt = t;
m[i].pt = t;
t += ts;
count++;
i++;
if(t > 0.0 )flg1 = 1;
else if( b[i].y > -0.1 ) flg1 = 1;
else flg1 = 0;
if(monkey_bound > 5)flg1 = 0;//サルが4回以上地面に落下した場合
if(ball_bound > 2) flg1 = 0;//弾丸が1回以上地面に落下した場合
else if( count > n - 1) flg1 = 0;//用意した配列の要素数以上になった場合
}while(flg1 == 1);
*total = i - 1;//計算した値の個数
}


//計算結果をグラフ表示する関数１
//引数１：弾丸
//引数２：サル
//引数３：データ数
//引数４：時間待ちの値
void graph_draw1(MASS b[], MASS m[], int n, struct timespec req)
{
    FILE *fp, *gp;

    //プログラムからgnuplotを呼び出すためのpopen
    if( (gp = popen("gnuplot", "w"))==NULL){
        printf("gnuplot open error\n");
        exit(EXIT_FAILURE);
    }

    //gnuplotの縦横のスケールを一致させる
    fprintf(gp,"set size ratio -1\n");

    //グラフ内に格子を描画する設定
    fprintf(gp,"set grid linestyle 2 linewidth 1 linecolor \"green\"\n");

    //縦軸・横軸の描画範囲をそれぞれ0-90に指定
    fprintf(gp, "set xrange[0.0:90.0]\n");
    fprintf(gp, "set yrange[0.0:90.0]\n");

    //時間待ち
    nanosleep(&req, NULL);

    //グラフにアニメーションを描画するfor文
    for( int i = 0; i < n; i+=10){//データ数が多い場合にはi+=10などと描画するデータを間引いても良い

        fp = fopen("move_data.dat", "w");//gnuplotに描画する値を一時的にファイルに書き込む
        //printf("%5d %f %f %f %f %f %f %f %f\n", count, b[i].x, b[i].y, b[i].vx, b[i].vy, m[i].x, m[i].y, m[i].vx, m[i].vy);
        fprintf(fp,"%f %f %f %f %f %f %f %f\n",  b[i].x, b[i].y, b[i].vx, b[i].vy, m[i].x, m[i].y, m[i].vx, m[i].vy);
        fclose(fp);

        fprintf(gp, "plot 'move_data.dat' w p pt 4 title 'Bullet', 'move_data.dat' using 5:6 w p pt 6 title 'Monkey'\n");
        fflush(gp);//ファイルバッファをクリア
        nanosleep(&req, NULL);//時間待ち
    }

    puts("グラフ描画終了");
    puts("Enterキーを押してください");
    getchar();
    fprintf(gp, "exit\n");
    fflush(gp);
    pclose(gp);

}

//計算結果をグラフ表示する関数２
//引数１：弾丸
//引数２：サル
//引数３：データ数
//引数４：時間待ちの値
void graph_draw2(MASS b[], MASS m[], int n, struct timespec req)
{
    FILE *fp, *gp;

    //プログラムからgnuplotを呼び出すためのpopen
    if( (gp = popen("gnuplot", "w"))==NULL){
        printf("gnuplot open error\n");
        exit(EXIT_FAILURE);
    }

    //gnuplotの縦横のスケールを一致させる
    fprintf(gp,"set size ratio -1\n");

    //グラフ内に格子を描画する設定
    fprintf(gp,"set grid linestyle 2 linewidth 1 linecolor \"green\"\n");

    //縦軸・横軸の描画範囲をそれぞれ0-90に指定
    fprintf(gp, "set xrange[0.0:90.0]\n");
    fprintf(gp, "set yrange[0.0:90.0]\n");

    //時間待ち
    nanosleep(&req, NULL);

    //グラフにアニメーションを描画するfor文
    for( int i = 0; i < n; i+=10){//データ数が多い場合にはi+=10などと描画するデータを間引いても良い

        fp = fopen("move_data.dat", "w");//gnuplotに描画する値を一時的にファイルに書き込む
        int pn = 100;
        if(i - pn <= 0){
            pn = pn - i;
        }
        else if( n - i - 1 < pn ){
            pn = n - i - 1;
        }
        for( int j = 0; j < pn; j+=10){
            fprintf(fp,"%f %f %f %f %f %f %f %f\n",  b[i-j].x, b[i-j].y, b[i-j].vx, b[i-j].vy, m[i-j].x, m[i-j].y, m[i-j].vx, m[i-j].vy);
        }
        fclose(fp);

        fprintf(gp, "plot 'move_data.dat' w p pt 4 title 'Bullet', 'move_data.dat' using 5:6 w p pt 6 title 'Monkey'\n");
        fflush(gp);//ファイルバッファをクリア
        nanosleep(&req, NULL);//時間待ち
    }

    puts("グラフ描画終了");
    puts("Enterキーを押してください");
    getchar();
    fprintf(gp, "exit\n");
    fflush(gp);
    pclose(gp);

}

//計算結果をグラフ表示する関数３
//引数１：弾丸
//引数２：サル
//引数３：データ数
//引数４：時間待ちの値
void graph_draw3(MASS b[], MASS m[], int n, struct timespec req)
{
    FILE *fp, *gp;

    //プログラムからgnuplotを呼び出すためのpopen
    if( (gp = popen("gnuplot", "w"))==NULL){
        printf("gnuplot open error\n");
        exit(EXIT_FAILURE);
    }

    //gnuplotの縦横のスケールを一致させる
    fprintf(gp,"set size ratio -1\n");

    //グラフ内に格子を描画する設定
    fprintf(gp,"set grid linestyle 2 linewidth 1 linecolor \"green\"\n");

    //縦軸・横軸の描画範囲をそれぞれ0-90に指定
    fprintf(gp, "set xrange[0.0:90.0]\n");
    fprintf(gp, "set yrange[0.0:90.0]\n");

    //時間待ち
    nanosleep(&req, NULL);

    //グラフにアニメーションを描画するfor文
    for( int i = 0; i < n; i+=10){//データ数が多い場合にはi+=10などと描画するデータを間引いても良い

        fp = fopen("move_data.dat", "w");//gnuplotに描画する値を一時的にファイルに書き込む
        int pn = 100;
        if(i - pn <= 0){
            pn = pn - i;
        }
        else if( n - i - 1 < pn ){
            pn = n - i - 1;
        }
        for( int j = 0; j <= i; j+=10){
            fprintf(fp,"%f %f %f %f %f %f %f %f\n",  b[j].x, b[j].y, b[j].vx, b[j].vy, m[j].x, m[j].y, m[j].vx, m[j].vy);
        }
        fclose(fp);

        fprintf(gp, "plot 'move_data.dat' w p pt 4 title 'Bullet', 'move_data.dat' using 5:6 w p pt 6 title 'Monkey'\n");
        fflush(gp);//ファイルバッファをクリア
        nanosleep(&req, NULL);//時間待ち
    }

    puts("グラフ描画終了");
    puts("Enterキーを押してください");
    getchar();
    fprintf(gp, "exit\n");
    fflush(gp);
    pclose(gp);
}

//計算結果をファイルに書き込む関数
//引数１：弾丸
//引数２：サル
//引数３：データ数
void data_write(MASS b[], MASS m[], int total)
{
    FILE *fw;

    char filename[50]={'\0'};
    char *pt;

    pt = filename;

    printf("ファイル名を入力してください");
    fgets(filename, 49, stdin);

    while( *pt != '\n'){//改行が現れるまでポインタを進める
        pt++;
    }
    *pt = '\0';  //改行を終端文字に置き換える

    fw = fopen(filename, "w");

    for( int i = 0; i < total; i++){
        fprintf(fw, "%f %f %f %f %f %f %f %f\n",  b[i].x, b[i].y, b[i].vx, b[i].vy, m[i].x, m[i].y, m[i].vx, m[i].vy);
    }
    fclose(fw);
}
