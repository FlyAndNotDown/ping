#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>

// 默认参数
#define DEFAULT_N 0
#define DEFAULT_L 64

// 函数声明
void ping(char *, int, int);
void endWithHelp();
void assembleIcmpPackage(struct icmp *, int, int);
unsigned short getCheckSum(unsigned short *, int);

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

    ping(argv[1], n, l, getpid());

    return 0;
}

void ping(char *ipAddress, int n, int l, pid_t pid) {
    // TODO
}

void assembleIcmpPackage(struct icmp *header, int sequence, int length, pid_t pid) {
    int i;

    // icmp类型：回送请求
    header->icmp_type = ICMP_ECHO;
    // icmp代码：0
    header->icmp_code = 0;
    // icmp校验和：0
    header->icmp_cksum = 0;
    // icmp序列号
    header->icmp_seq = sequence;
    // 进程标识为两字节，而 icmp_id 为四字节
    header->icmp_id = pid & 0xffff;

    // 填充数据段，这里 icmp 报文必须大于 64B
    for (i = 0; i < length; i++) {
        header->icmp_data[i] = i;
    }

    // 计算校验和
    header->icmp_cksum = getCheckSum((unsigned short *) header, length);
}

unsigned short getCheckSum(unsigned short *header, int length) {
    int count = length;
    int sum = 0;
    unsigned short *t = header;
    unsigned short result = 0;

    // 每两个字节累加
    while (count > 1) {
        sum += *t++;
        count -= 2;
    }

    // 如果最后剩下一个字节，补齐两个字节，继续累加
    if (count == 1) {
        * (unsigned char *) (&result) = *(unsigned char *) t;
        sum += result;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void endWithHelp() {
    printf("Usage: ping ipAddress [-n] [sendTimes] [-l] [packageLength]\n");
    exit(0);
}
