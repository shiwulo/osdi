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
int stop = 0;
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



//結束時間到會設定stop = 1
void sigAlarm(int signo) {
    static int time = 9;
    time++;
    if (time == 15)
        stop = 1;
    printf("時間：%2d點\n", time);
    alarm(1);
}

//➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
//Peterson's solution的共用變數
//Peterson's solution只適用於二個thread
atomic_int turn=0;
atomic_int flag[2]={0};


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
        //flag[1] = 1;     //想要進去 
        atomic_store(&flag[1], 1);
        //turn = 0;    //讓先
        atomic_store(&turn, 0);
        //等待回圈，如果輪到對方執行，而且對方想進入CS，就讓對方先進去
        while(atomic_load(&turn) == 0 && atomic_load(&flag[0]) == 1)
            ;


        //底下程式碼從source轉amount的錢到dest
        bankAccount[source] -= amount;
        bankAccount[dest] += amount;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        //flag[1] = 0;
        atomic_store(&flag[1], 0);

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
        //flag[0] = 1;     //想要進去 
        atomic_store(&flag[0], 1);
        //turn = 1;    //讓先
        atomic_store(&turn, 1);
        //等待回圈，如果輪到對方執行，而且對方想進入CS，就讓對方先進去
        while(atomic_load(&turn) == 1 && atomic_load(&flag[0]) == 1)
            ;

        //底下程式碼從source轉amount的錢到dest
        bankAccount[source] -= amount;
        bankAccount[dest] += amount;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        //flag[0] = 0;    
        atomic_store(&flag[0], 1);   

        //成功地進行一次轉帳
        KPI++;
    }
    kpi[name] = KPI;
    printf(YELLOW"%ld行員收工，共處理%ld次轉帳\n"RESET, gettid(), KPI);
}


//底下程式碼也是對的，但我沒有使用，留給大家做參考
void moneyTransfer(void *givenName) {
    unsigned rand_value = (unsigned)gettid();
    int name = (intptr_t)givenName;

    //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
    //Peterson's solution，定義名字
    int self = name;
    int theother = 1-name;

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
        //flag[self] = 1;     //想要進去
        atomic_store(&flag[self], 1);
        //turn = theother;    //讓先
        atomic_thread_fence(memory_order_seq_cst);
        atomic_store(&turn, theother);
        //等待回圈，如果輪到對方執行，而且對方想進入CS，就讓對方先進去
        while(atomic_load(&turn) == theother && atomic_load(&flag[theother]) == 1) {
            if (stop == 1) break;
        }

        //底下程式碼從source轉amount的錢到dest
        bankAccount[source] -= amount;
        bankAccount[dest] += amount;

        //➩ ➪ ➫ ➬ ➭ ➮ ➯ ➱ ➜ ➝ ➞ ➟ ➠ ➡
        //Peterson's solution
        //flag[self] = 0;
        atomic_store(&flag[self], 0);        

        //成功地進行一次轉帳
        KPI++;
    }
    kpi[name] = KPI;
    printf(YELLOW"%ld行員收工，共處理%.3f 百萬次轉帳\n"RESET, gettid(), ((double)KPI)/1000000);
}

void printAccountSummary() {
    long sum=0;
    for (int i=0; i<numCustomer; i++) {
        printf("客戶%d 有 %6ld 元\n", i, bankAccount[i]);
        sum += bankAccount[i];
    }
    printf(YELLOW"所有的人的錢共有：%ld\n"RESET, sum);
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
    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i],NULL,(void *) moneyTransfer, (void*)i);
    //等待執行緒完成
    for (long i=0; i< numCPU; i++)
	    pthread_join(tid[i],NULL);
    //列印結果
    printf("下午三點，銀行關門，進行結帳\n");
    printAccountSummary();
    long kpi_sum=0;
    for(int i=0; i<numCPU; i++)
        kpi_sum+=kpi[i];
    printf(YELLOW"共進行%.3f 百萬次轉帳\n"RESET, ((double)kpi_sum)/1000000);    
}
