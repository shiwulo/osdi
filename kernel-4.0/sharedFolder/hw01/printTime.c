#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
int timespec2str(char *buf, uint len, struct timespec *ts) {
	int ret;
	struct tm t;
	tzset();
	if (localtime_r(&(ts->tv_sec), &t) == NULL)
			return 1;
	ret = strftime(buf, len, "%F %T", &t);
	if (ret == 0)
			return 2;
	len -= ret - 1;
	ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
	if (ret >= len)
		return 3;
	return 0;
}

int main(int argc, char **argv) {
	clockid_t clk_id = CLOCK_REALTIME;
	char timestr[1000];
	struct timespec ts;
	FILE *a, *b;
	a = fopen("./a.time", "w+");
	b = fopen("./b.time", "w+");
	while(1) {
			clock_gettime(clk_id, &ts);
			timespec2str(timestr, sizeof(timestr), &ts);
			fprintf(a, "time=%s\n", timestr);
			fprintf(b, "time=%s\n", timestr);
			sync();
    	}
}

