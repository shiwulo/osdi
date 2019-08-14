#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>

int main(int argc, char** argv) {
    int nMatrixChild=1;
    pid_t matrixChildId[10000], perfId;
    char strbuf[1000]={0};
    FILE* resultFile = fopen("./result.data", "w+");
    if (argc >= 2)
        sscanf(argv[1], "%d", &nMatrixChild);
    else {
        printf("execfile num_of_chld\n");
        exit(0);
    }
    for (int i=0; i< nMatrixChild; i++) {
        char* matrix_para[3];
        matrix_para[0] = "./matrix";
        if (argc == 3)
            matrix_para[1] = argv[2];
        else
            matrix_para[1] = "10";
        matrix_para[2] = NULL;
        if ((matrixChildId[i] = fork()) == 0) {
            execvp("./matrix", matrix_para);
        }
    }
    if ((perfId = vfork()) == 0) {
        sprintf(strbuf, "perf record -e context-switches,cache-references,"
        "cache-misses,mem_inst_retired.lock_loads,offcore_response.demand_data_rd.l3_hit.any_snoop,"
        "mem_load_retired.l3_hit,instructions,cycles -p ");
        for (int i=0; i< nMatrixChild; i++)
            sprintf(strbuf+strlen(strbuf), "%d,", matrixChildId[i]);
        strbuf[strlen(strbuf)-1] = 0;
        printf("%s\n", strbuf);
        system(strbuf);
        exit(0);
    }
    int wstatus;
    struct rusage res={0};
    long totalVoluntaryCtxSw=0;
    long totalInVoluntaryCtxSw=0;
    for (int i=0; i< nMatrixChild; i++) {
        wait4(matrixChildId[i],&wstatus, 0, &res);
        totalVoluntaryCtxSw += res.ru_nvcsw;
        totalInVoluntaryCtxSw += res.ru_nivcsw;
    }
    printf("共%d個行程，執行於vcore-0\n", nMatrixChild);
    printf("自願性行程切換共 %ld\n", totalVoluntaryCtxSw);
    printf("非自願性行程切換共 %ld\n", totalInVoluntaryCtxSw);
    char resultStr[100];
    long long totalOps=0;
    while (fgets(resultStr, 100, resultFile) != NULL) {
        long long tmp=0;
        sscanf(resultStr, "%lld", &tmp); 
        totalOps += tmp;
    }
    printf("所有task的總共運算次數 %f\n", ((double)totalOps)/1000000);
}
