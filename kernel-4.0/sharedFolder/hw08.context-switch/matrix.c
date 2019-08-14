#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


long matrix[1024][1024] = {0};    //8MB
volatile int stop = 0;


void sigHandler(int signo) {
    stop = 1;
}

int main(int argc, char** argv) {
    FILE* resultFile = fopen("./result.data", "a+");
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(getpid(), sizeof(set), &set);
    signal(SIGALRM, sigHandler);
    printf("%s: argc = %d, argv[1]=%s secs\n",argv[0], argc, argv[1]);
    
    if (argc == 2) {
        int sec;
        sscanf(argv[1], "%d\n", &sec);
        alarm(sec);
    } else {
        alarm(5);
    }
    //printf("sizeof(matrix)=%.2f M\n", ((double)sizeof(matrix))/1024/1024);
    long long run=0;
    while(stop == 0) {
        run++;
        int x = rand()%1024;
        int y = rand()%1024;
        matrix[x][y] += 1;
    }
    printf("%.2f million operations\n", ((double)run)/1000000);
    fprintf(resultFile, "%lld\n", run);
}

