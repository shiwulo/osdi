#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int numCPU = -1;
//注意，這個地方我使用了atomic_int
///atomic_int stop = 0;
//試試看另外二種宣告方式，這個地方的正確與否與編譯器相關。最正確的寫法是『volatile int stop = 0;』
//int stop = 0;
volatile int stop = 0;
int numCustomer = -1;
char* exename;

long bankAccount[1000] = {0};
long kpi[1000];

long gettid() {
    return (long int)syscall(__NR_gettid);
}

long timespec2nano(struct timespec ts) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

__thread char name[100];
__thread int threadID = -1;

void printAccountSummary() {
    long sum=0;
    for (int i=0; i<numCustomer; i++) {
        printf("客戶%d 有 %6ld 元\n", i, bankAccount[i]);
        sum += bankAccount[i];
    }
    printf(YELLOW"所有的人的錢共有：%ld\n"RESET, sum);
}

//結束時間到會設定stop = 1
void sigAlarm(int signo) {
    static int time = 9;
    time++;
    if (time == 15) {
        printf("時間到\n");
        printAccountSummary();
        long kpi_sum=0;
        for(int i=0; i<numCPU; i++)
            kpi_sum+=kpi[i];
        printf(YELLOW"共進行%ld次轉帳\n"RESET, kpi_sum);
        stop = 1;
    }
    printf("時間：%2d點\n", time);
    alarm(1);
}

//➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
//Peterson's solution的共用變數
//Peterson's solution只適用於二個thread
int turn=0;
int flag[2]={0};

void moneyTransferP1(void *givenName) {
    unsigned rand_value = (unsigned)gettid();
    int name = (intptr_t)givenName;

    int source; int dest; int amount;
    //KPI：代表這個行員共處理多少次轉帳
    long KPI=0;
    printf("%d行員上線\n", name);
    while(stop == 0) {
        //隨機產生轉入帳戶和轉出帳戶及金額
        rand_value = rand_r(&rand_value);
        source = rand_value%numCustomer;
        rand_value = rand_r(&rand_value);
        dest = rand_value%numCustomer;
        rand_value = rand_r(&rand_value);
        //一次轉帳最多轉1000元
        amount = rand_value%1000;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        flag[1] = 1;     //想要進去 
        turn = 0;    //讓先
        //等待回圈，如果輪到對方執行，而且對方想進入CS，就讓對方先進去
        while(turn == 0 && flag[0] == 1)
            ;


        //底下程式碼從source轉amount的錢到dest
        bankAccount[source] -= amount;
        bankAccount[dest] += amount;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        flag[1] = 0;        

        //成功地進行一次轉帳
        KPI++;
    }
    kpi[name] = KPI;
    printf(YELLOW"%ld行員收工，共處理%ld次轉帳\n"RESET, gettid(), KPI);
}


void moneyTransferP0(void *givenName) {
    unsigned rand_value = (unsigned)gettid();
    int name = (intptr_t)givenName;

    int source; int dest; int amount;
    //KPI：代表這個行員共處理多少次轉帳
    long KPI=0;
    printf("%d行員上線\n", name);
    while(stop == 0) {
        //隨機產生轉入帳戶和轉出帳戶及金額
        rand_value = rand_r(&rand_value);
        source = rand_value%numCustomer;
        rand_value = rand_r(&rand_value);
        dest = rand_value%numCustomer;
        rand_value = rand_r(&rand_value);
        //一次轉帳最多轉1000元
        amount = rand_value%1000;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        flag[0] = 1;     //想要進去 
        turn = 1;    //讓先
        //等待回圈，如果輪到對方執行，而且對方想進入CS，就讓對方先進去
        while(turn == 1 && flag[0] == 1)
            ;

        //底下程式碼從source轉amount的錢到dest
        bankAccount[source] -= amount;
        bankAccount[dest] += amount;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        flag[0] = 0;        

        //成功地進行一次轉帳
        KPI++;
    }
    kpi[name] = KPI;
    printf(YELLOW"%ld行員收工，共處理%ld次轉帳\n"RESET, gettid(), KPI);
}


int main(int argc, char **argv) {

    exename = argv[0];
    //取得系統的處理器數量
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    //在這個例子中，我們只假設有二個銀行 行員
    numCPU = 2;
    //共有20個顧客
    numCustomer = numCPU * 4;
    //每個顧客帳戶裡面，一開始有十萬元
    for (int i=0; i< numCustomer; i++) {
        bankAccount[i] = 100000;    //十萬元
    }
    //設定結束時間
    signal(SIGALRM, sigAlarm);
    alarm(1);
    printf("銀行開門做生意讓客戶轉帳\n");
    printAccountSummary();

    //產生執行緒
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);
    pthread_create(&tid[0],NULL,(void *) moneyTransferP0, (void*)0);
    pthread_create(&tid[1],NULL,(void *) moneyTransferP1, (void*)1);
    //等待執行緒完成
	pthread_join(tid[0],NULL);
    pthread_join(tid[1],NULL);
    //列印結果
    printf("下午三點，銀行關門，進行結帳\n");
    printAccountSummary();
    long kpi_sum=0;
    for(int i=0; i<numCPU; i++)
        kpi_sum+=kpi[i];
    printf(YELLOW"共進行%.3f百萬次轉帳\n"RESET, ((double)kpi_sum)/1000000);
}