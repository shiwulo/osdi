#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int count=0;

void signal_handler(int signum) {
	count++;
	printf("%d\n", count);
	if (count == 10) exit(1);
}

int main(int argc, char** argv) {
	int a, b;
	signal(SIGFPE, signal_handler);
	b = 0;
	a = 10/b;
}
