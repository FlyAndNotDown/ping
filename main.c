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

// 调试模式
#define DEV_MODE 0

// 默认参数
#define DEFAULT_N 0
#define DEFAULT_L 56

// L 范围
#define L_MIN 10
#define L_MAX 1024 * 1024

// 缓冲区大小
#define SEND_BUFFER_SIZE 1024 * 1024
#define RECV_BUFFER_SIZE 1024 * 1024

// 超时设置
#define OUT_TIMEVAL_USEC 0
#define OUT_TIMEVAL_SEC 1000

// 函数声明
int ping(char *, int, int);
void assembleIcmpPackage(struct icmp *, int, int, pid_t);
struct timeval getOffsetTime(struct timeval, struct timeval);
unsigned short getCheckSum(unsigned short *, int);

int main(int argc, char *argv[]) {
    int i;
    int n = DEFAULT_N, l = DEFAULT_L;

    if (argc < 2) {
        printf("Usage: ping addressArg [-n] [sendTimes] [-l] [packageLength]\n");
        return 0;
    }

    for (i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "-n") && i + 1 < argc) {
            n = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "-l") && i + 1 < argc) {
            l = atoi(argv[i + 1]);
        }
    }

    if (l < L_MIN || l > L_MAX) {
        printf("\'l\' is out of range\n");
        return 0;
    }

    ping(argv[1], n, l);

    return 0;
}

int ping(char *addressArg, int n, int l) {
    if (DEV_MODE) {
        printf("n: %d\n", n);
        printf("l: %d\n", l);
    }

    char sendBuffer[SEND_BUFFER_SIZE], recvBuffer[RECV_BUFFER_SIZE];
    memset(sendBuffer, 0, sizeof(sendBuffer));
    memset(recvBuffer, 0, sizeof(recvBuffer));

    int count = 0, nt = n == 0 ? 1 : n;

    // 获取进程标识符
    pid_t pid = getpid();

    // 建立套接字
    struct protoent* protocol = getprotobyname("icmp");
    int sock = socket(AF_INET, SOCK_RAW, protocol->p_proto);
    if (sock < 0) {
        printf("Can't create socket.\n");
        return 0;
    }

    // 设置接收缓冲区大小
    int recvBufferSize = RECV_BUFFER_SIZE;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, sizeof(recvBufferSize));

    // 改造 ip 地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    unsigned int internetAddress = inet_addr(addressArg);
    if (internetAddress == INADDR_NONE) {
        // 如果输入的是域名地址
        struct hostent *host = gethostbyname(addressArg);
        if (host == NULL) {
            printf("Fail to get host name.\n");
            return 0;
        }

        memcpy((char *) &address.sin_addr, host->h_addr, host->h_length);
    } else {
        memcpy((char *) &address.sin_addr, &internetAddress, sizeof(internetAddress));
    }
    internetAddress = address.sin_addr.s_addr;

    // 输出信息
    printf(
        "PING %s, (%d, %d, %d, %d) (%d)%d bytes of data.\n",
        addressArg,
        internetAddress & 0x000000ff,
        (internetAddress & 0x0000ff00) >> 8,
        (internetAddress & 0x00ff0000) >> 16,
        (internetAddress & 0xff000000) >> 24,
        l,
        l + 28
    );

    // 读文件描述符
    fd_set readFd;

    // 超时设置
    struct timeval tv;
    tv.tv_usec = OUT_TIMEVAL_USEC;
    tv.tv_sec = OUT_TIMEVAL_SEC;
    struct timeval beginTime, endTime, offsetTime;

    while (nt > 0) {
        // 封装 icmp 包
        assembleIcmpPackage((struct icmp *) sendBuffer, ++count, l, pid);
        // 发送 icmp 包
        if (sendto(sock, sendBuffer, l, 0, (struct sockaddr *) &address, sizeof(address)) < 0) {
            printf("Send data fail.\n");
            continue;
        }

        if (DEV_MODE) {
            printf("Send data success\n");
        }

        // 计时
        gettimeofday(&beginTime, NULL);

        // 开始收包
        FD_ZERO(&readFd);
        FD_SET(sock, &readFd);

        int recvSize;

        switch (select(sock + 1, &readFd, NULL, NULL, &tv)) {
            case -1:
                printf("Fail to select\n");
                break;
            case 0:
                break;
            default:
                recvSize = recv(sock, recvBuffer, sizeof(recvBuffer), 0);
                // 接受数据到缓冲区
                if (recvSize < 0) {
                    printf("Fail to receive data\n");
                    continue;
                }

                if (DEV_MODE) {
                    printf("recvSize: %d\n", recvSize);
                }

                // 解包
                // 获取 ip 包头
                struct ip *ipHeader = (struct ip *) recvBuffer;
                // 获取 ip 包头长度 (ip_hl是以字节为单位)
                int ipHeaderLength = ipHeader->ip_hl * 4;
                // 获取 icmp 包头
                struct icmp *icmpHeader = (struct icmp *) (recvBuffer + ipHeaderLength);
                // 获取 icmp 包长度
                int icmpPackgeLength = recvSize - ipHeaderLength;

                // 如果小于 8 字节说明不是 icmp 包
                if (icmpPackgeLength < 8) {
                    printf("Invalid icmp package, because its length it less than 8 byte\n");
                    continue;
                }

                if (DEV_MODE) {
                    printf("icmp->icmp_type: %d ICMP_ECHOREPLY:%d\n", icmpHeader->icmp_type, ICMP_ECHOREPLY);
                    printf("icmp->icmp_id: %d pid: %d\n", icmpHeader->icmp_id, pid);
                    printf("icmp->icmp_seq: %d\n", icmpHeader->icmp_seq);
                }

                // 判断是 icmp 回应包而且是本机发的
                if (icmpHeader->icmp_type == ICMP_ECHOREPLY && icmpHeader->icmp_id == (pid & 0xffff)) {
                    if (icmpHeader->icmp_seq < 0 || n != 0 && icmpHeader->icmp_seq > n) {
                        printf("Sequence of icmp package is out of range.\n");
                        continue;
                    }

                    // 记下收包时间
                    gettimeofday(&endTime, NULL);
                    // 计算时差
                    offsetTime = getOffsetTime(beginTime, endTime);

                    // 输出结果
                    printf(
                        "%d byte from %s: icmp_seq=%u ttl=%d rtt=%ds%dus\n",
                        l + 8,
                        inet_ntoa(ipHeader->ip_src),
                        icmpHeader->icmp_seq,
                        ipHeader->ip_ttl,
                        offsetTime.tv_sec,
                        offsetTime.tv_usec
                    );
                } else continue;

                break;
        }

        if (n != 0) nt--;
        sleep(1);
    }
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

struct timeval getOffsetTime(struct timeval beginTime, struct timeval endTime) {
    struct timeval offsetTime;

    offsetTime.tv_sec = endTime.tv_sec - beginTime.tv_sec;
    offsetTime.tv_usec = endTime.tv_usec - beginTime.tv_usec;

    if (offsetTime.tv_usec < 0) {
        offsetTime.tv_sec--;
        offsetTime.tv_usec += 1000000;
    }

    return offsetTime;
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
