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

int numCPU=-1;
char* exename;

long gettid() {
    return (long int)syscall(__NR_gettid);
}

long timespec2nano(struct timespec ts) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

__thread char name[100];
__thread int threadID = -1;

void sigAlarm(int signo) {
    printf("my name is %s\n", name);
}

void thread(void *givenName) {
    int givenID = (intptr_t)givenName;
    threadID = gettid();
    printf("\tthread🦖🦕%02d is here.\n", givenID);
    printf("\tmy pid is %ld and my tid is %ld\n", (long)getpid(), threadID);
    sprintf(name, "🦖🦕###%02d###", givenID);
    sleep(1);
    printf("\tmy name is %s\n", name);
}

int main(int argc, char **argv) {

    exename = argv[0];
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );

    signal(SIGALRM, sigAlarm);

    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);

    printf("I am main funciton, my pid is %ld ", (long)getpid());
    printf("and my tid is %ld\n", gettid());

    printf("waiting for child threads\n");
    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i],NULL,(void *) thread, (void*)i);

    for (int i=0; i< numCPU; i++)
	    pthread_join(tid[i],NULL);
    printf("all threads finish their work\n");
}
