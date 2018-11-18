#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_N 0
#define DEFAULT_L 100

void ping(char *, int, int);
void endWithHelp();

int main(int argc, char *argv[]) {
    int i;
    int n = DEFAULT_N, l = DEFAULT_L;

    if (argc < 2) {
        endWithHelp();
    }

    for (i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-n") && i + 1 < argc) {
            n = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-l") && i + 1 < argc) {
            l = atoi(argv[i + 1]);
        }
    }

    ping(argv[1], n, l);

    return 0;
}

void ping(char *ipAddress, int n, int l) {
    // TODO
}

void endWithHelp() {
    printf("Usage: ping ipAddress [-n] [sendTimes] [-l] [packageLength]\n");
    exit(0);
}
