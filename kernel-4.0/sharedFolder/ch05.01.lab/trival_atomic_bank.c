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
#include <stdbool.h>

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

//假設系統最多100000個帳戶
//每個帳戶都有一個鎖，初始值是 「unlock」
atomic_long bankAccount[100000] = {0};
atomic_int lock_ary[100000] = {0};

long kpi[1000];

//自行實現的spinlock，符合mutual exculsion、progress
//但不符合bounded waiting
void simple_spinlock_lock(atomic_int* lock) {
    int isunlock=0;
    //注意，這在這個地方我先檢查「*lock == 0」，這一行是多的，但在一些處理器上，這一行會在增加效能，這個技巧稱之為test_and_test_and_set
    //其實 atomic_compare_exchange_weak 已經有 test_and_test_and_set，但自行加入可避免硬體並未實現這個功能
    //isulock有二個功能，第一個功能是：判斷lock和isulock是否一樣
    //第二個功能是，如果lock和isunlock的值不一樣時，硬體會將lock目前的值填入「isunlock」，這通常用於判斷目前lock是由誰所持有
    //atomic_compare_exchange_weak的第三個參數是，如果判斷結果為true，那麼lock要被設定為何？
    //atomic_compare_exchange_weak中的「weak」代表，有可能所以人都失敗
    //例如：lock為0照理說應該要讓一個人進入CS，但weak的版本，可能沒有任何人會進去CS，在這個範例中，因為迴圈會再執行一次，因此使用weak的就可以了
    //如果「一定要立即決定一個人」，使用 atomic_compare_exchange_strong
    while (*lock == 0  &&  atomic_compare_exchange_weak(lock, &isunlock, 1) == false) {
        isunlock=0;
    }
}

//解除鎖定
void simple_spinlock_unlock(atomic_int* lock) {
    atomic_store(lock, 0);
}

//交換二個變數
void swap(int* a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}


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

void moneyTransfer(void *givenName) {
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
        //一次轉帳最多轉999元
        amount = rand_value%1000;

        //如果轉出和轉入的戶頭是一樣的，那麼就下一個迴圈（等於這次不用做）
        if (source == dest) {
            continue;
        }

        //避免deadlock，例如二個行員一個要從A轉到B，另一個要從B轉到A，如果第一個行員先鎖定A，第二個行員先鎖定B，那麼他們都會無法繼續往下處理
        //請試著把底下這二行拿掉看看
        //底下程式碼確保鎖定的方向是一樣的，先鎖小的再鎖大的
        if (source > dest) {
            swap(&source , &dest);
        }
        //開始上鎖
        simple_spinlock_lock(&(lock_ary[source]));
        simple_spinlock_lock(&(lock_ary[dest]));

        //底下程式碼從source轉amount的錢到dest
        bankAccount[source]-=amount;
        bankAccount[dest]+=amount;

        //解鎖
        simple_spinlock_unlock(&(lock_ary[source]));
        simple_spinlock_unlock(&(lock_ary[dest]));

        //成功地進行一次轉帳
        KPI++;
    }
    kpi[name] = KPI;
    printf(YELLOW"%ld行員收工，共處理%ld次轉帳\n"RESET, gettid(), KPI);
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
    //假設銀行中，行員的數量等同於core的數量
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    //試試看，如果行員只有一個，跟多個行員，哪一個效率比較好
    //numCPU = 1;
    //客戶的數量是行員數量的10倍
    numCustomer = numCPU * 10;
    //每個顧客帳戶裡面，一開始有十萬元
    for (int i=0; i< numCustomer; i++) {
        bankAccount[i] = 100000;    //十萬元
    }
    //設定結束時間
    signal(SIGALRM, sigAlarm);
    alarm(1);
    printf("銀行開門做生意讓客戶轉帳\n");
    printf("行員共有%d個", numCPU);
    printAccountSummary();

    //產生執行緒
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);
    for (int i=0; i< numCPU; i++) {
        pthread_create(&tid[0],NULL,(void *) moneyTransfer, (void*)(long)i);
    }
    //等待執行緒完成
    for (int i=0; i< numCPU; i++) {
        pthread_join(tid[i],NULL);
    }
    //列印結果
    printf("下午三點，銀行關門，進行結帳\n");
    printAccountSummary();
    long kpi_sum=0;
    for(int i=0; i<numCPU; i++)
        kpi_sum+=kpi[i];
    printf(YELLOW"共進行%.3f 百萬次轉帳\n"RESET, ((double)kpi_sum)/1000000);
}
