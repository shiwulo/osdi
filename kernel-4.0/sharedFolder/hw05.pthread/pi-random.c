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
#include <math.h>

#define loopCount 100000000

int numCPU=-1;
char* exename;

long gettid() {
    return (long int)syscall(__NR_gettid);
}

//注意，我使用了「volatile」
volatile long score[100];

void thread(void *givenName) {
    int id = (intptr_t)givenName;
    struct drand48_data r_data;
    double x, y, dist;
    srand48_r((long)givenName, &r_data);
    drand48_r(&r_data, &x);
    //底下這一行是除錯用，我要確認每個thread的reandom stream是完全不同的
    printf("%f, ", x);
    for (int i=0; i< loopCount; i++) {
        drand48_r(&r_data, &x);
        drand48_r(&r_data, &y);
        //printf("%f, %f\n", x, y);
        dist = sqrt(x*x + y*y);
        if (dist < 1)
            score[id]++;
    }
}

int main(int argc, char **argv) {
    exename = argv[0];
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);

    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i], NULL, (void *) thread, (void*)i);

    for (int i=0; i< numCPU; i++)
	    pthread_join(tid[i], NULL);

    long total=0;
    for (int i=0; i< numCPU; i++) {
        total += score[i];
    }
    //這一行讓我知道飛鏢射中幾次，除錯用
    printf("\n%ld\n", total);
    printf("pi = %f\n", ((double)total)/(loopCount*numCPU)*4);
}
