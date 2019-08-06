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

volatile long score[100];

void thread(void *givenName) {
    int id = (intptr_t)givenName;
    struct drand48_data r_data;
    double x, y, dist;
    srand48_r((long)givenName, &r_data);
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
    printf("%ld\n", total);
    printf("pi = %f\n", ((double)total)/(loopCount*numCPU)*4);
}
