#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>
#include <stdbool.h>
int numCPU = -1;


struct CpuInfo {
    float mhz;
};

FILE* cpuinfo;
struct CpuInfo* cpuinfoArray;

int temp[4];
FILE* temp1, *temp2, *temp3, *temp4;

//底下這程式碼是印出執行頻率，我印象當中，底下程式碼是錯誤的
//必須把檔案關掉再打開，這樣才會顯示真正的頻率
void printFeq() {
    char buffer[1000];
    fseek(cpuinfo, 0, SEEK_SET);
    while(1) {
        int cpuid;
        float freq;
        if (fgets(buffer, 1000,cpuinfo) == NULL) break;
        char *typeStr=strtok(buffer, ":");
        if (strcmp("cpu MHz		", typeStr) == 0) {
            char* temp = strtok(NULL, " :");
            sscanf(temp, "%f", &freq);
            if (cpuid == -1) {
                fprintf(stderr, "ERROR: cannot read temp correctly\n");
                break;
            }
            cpuinfoArray[cpuid].mhz = freq;
            cpuid = -1;
            //printf("MHZ = %s\n", temp);
        }
        if (strcmp("processor	", typeStr) == 0) {
            char* cpuidstr = strtok(NULL, " :");
            sscanf(cpuidstr, "%d", &cpuid);
        }
    }
    float totalFreq=0;
    for (int i=0; i<numCPU; i++) {
        totalFreq += cpuinfoArray[i].mhz;
    }
    printf("freq, %.2f, ", totalFreq/numCPU);
}

//底下這程式碼會印出所有CPU的溫度
int printTemp() {
    int eatReturn;
    int tempArray[4];

    temp1 = fopen("/sys/class/hwmon/hwmon1/temp1_input", "r");
    assert(temp1 != NULL);
    temp2 = fopen("/sys/class/hwmon/hwmon2/temp1_input", "r");
    assert(temp2 != NULL);
    temp3 = fopen("/sys/class/hwmon/hwmon3/temp1_input", "r");
    assert(temp3 != NULL);
    temp4 = fopen("/sys/class/hwmon/hwmon4/temp1_input", "r");
    assert(temp4 != NULL);

    for (int i=0; i<4; i++)
        tempArray[i] = -1;
    //int eatReturn;
    fseek(temp1, 0, SEEK_SET);
    eatReturn=fscanf(temp1, "%d", &tempArray[0]);
    fseek(temp2, 0, SEEK_SET);
    eatReturn=fscanf(temp2, "%d", &tempArray[1]);
    fseek(temp3, 0, SEEK_SET);
    eatReturn=fscanf(temp3, "%d", &tempArray[2]);
    fseek(temp4, 0, SEEK_SET);
    eatReturn=fscanf(temp4, "%d", &tempArray[3]);
    
    fclose(temp1); fclose(temp2); fclose(temp3); fclose(temp4);

    int totalTemp=0;
    for (int i=0; i<4; i++)
        totalTemp += tempArray[i];
    //printf("temp, %d, ", totalTemp/4);
    //printf("%d \n", totalTemp);
    int avgTemp = totalTemp/4;
    return avgTemp;
}




atomic_int stop = 0;

long timespec2nano(struct timespec ts) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}

char* exename;


int main(int argc, char **argv) {
    //預設值等到CPU的溫度降到35度
    int tempture=35; 
    //使用者可以在參數列輸入溫度
    if (argc == 2) {
        sscanf(argv[1], "%d", &tempture);
    }
    fprintf(stderr, "target = %d\n", tempture);
    //授課老師自己稍微看一下，這個溫度要乘上1000才是真正的溫度
    tempture = tempture * 1000;
    //底下的程式碼是我以前寫的，留給大家做參考
    //cpuinfo = fopen("/proc/cpuinfo", "r");
    //fprintf(stderr, "wait32, step 1...\n");
    //temp1 = fopen("/sys/class/hwmon/hwmon1/temp1_input", "r");
    //assert(temp1 != NULL);
    //temp2 = fopen("/sys/class/hwmon/hwmon2/temp1_input", "r");
    //assert(temp2 != NULL);
    //temp3 = fopen("/sys/class/hwmon/hwmon3/temp1_input", "r");
    //assert(temp3 != NULL);
    //temp4 = fopen("/sys/class/hwmon/hwmon4/temp1_input", "r");
    //assert(temp4 != NULL);
    //cpuinfoArray = (struct CpuInfo*)malloc(sizeof(struct CpuInfo) * numCPU);

    //一個迴圈，每隔一秒讀取CPU的溫度，直到溫度比設定的還要低
    int temp;
    int try=1;
    while((temp = printTemp()) > tempture) {
        fprintf(stderr, "#=%02d sec 💥 🔥 💥 🔥  %.2f ℃\n", try, ((float)temp)/1000);
        try++;
        sleep(1);
    }
    printf("temp %.2f℃, \n", ((float)temp)/1000);
    exit(0);
}

