#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义错误类型
typedef unsigned int ErrorType;
#define ERROR_TYPE__PARAMS_NOT_ENOUGH 0
#define ERROR_TYPE__ERROR_IP_ADDRESS_FORMAT 1
#define ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_N 2
#define ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_L 3

// 定义默认设置
#define DEFAULT_SEND_TIME 4
#define DEFAULT_PACKAGE_LENGTH 100

// 定义参数范围
#define PARAM_RANGE_N_START 1
#define PARAM_RANGE_N_END 20
#define PARAM_RANGE_L_START 100
#define PARAM_RANGE_L_END 200

// 预定义函数
void endWithError(ErrorType errorType);
int checkIpAddress(char *ipAddress);
int checkIfNumberInRange(int target, int start, int end);
void ping(char *ipAddress, int sendTime, int packageLength);

/**
 * main()
 * @description main函数
 * @param {int} argc 参数个数
 * @param {char *[]} argv 程序外部传参
 * @return
 */
int main(int argc, char *argv[]) {
    int i, n = DEFAULT_SEND_TIME, l = DEFAULT_PACKAGE_LENGTH, number;

    // 参数判断，如果参数小于两个，则说明没有输入IP地址
    if (argc < 2) { endWithError(ERROR_TYPE__PARAMS_NOT_ENOUGH); }

    // 校验 IP 地址格式是否正确
    char *ipAddress = argv[1];
    if (!ipAddress) { endWithError(ERROR_TYPE__ERROR_IP_ADDRESS_FORMAT); }

    // 遍历后面的参数
    for (i = 2; i < argc; i++) {
        // 如果有 -n
        if (!strcmp(argv[i], "-n")) {
            if (i + 1 >= argc) { endWithError(ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_N); }
            // 查找其下一个参数并且转换成数字
            number = atoi(argv[i + 1]);
            if (!checkIfNumberInRange(number, PARAM_RANGE_N_START, PARAM_RANGE_N_END)) {
                endWithError(ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_N);
            }
            n = number;
            i++;
        }
        // 如果有 -l
        if (!strcmp(argv[i], "-l")) {
            if (i + 1 >= argc) { endWithError(ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_L); }
            // 一样
            number = atoi(argv[i + 1]);
            if (!checkIfNumberInRange(number, PARAM_RANGE_L_START, PARAM_RANGE_L_END)) {
                endWithError(ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_L);
            }
            l = number;
            i++;
        }
    }

    // 执行 ping 行为函数
    ping(ipAddress, n, l);

    return 0;
}

/**
 * endWithError()
 * @description 报错并结束程序
 * @param {ErrorType} errorType 错误类型
 */
void endWithError(ErrorType errorType) {
    // 报错
    switch (errorType) {
        case ERROR_TYPE__PARAMS_NOT_ENOUGH:
            printf("Have no enough params.\n");
            printf("Usage: ping ip_address [-n] [send_time] [-l] [ICMP_package_length]\n");
            break;
        case ERROR_TYPE__ERROR_IP_ADDRESS_FORMAT:
            printf("Error IP address format. Please try again.\n");
            break;
        case ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_N:
            printf("Error param or not in range: -n\n");
            printf("Usage: [-n] [number in range %d-%d]\n", PARAM_RANGE_N_START, PARAM_RANGE_N_END);
            break;
        case ERROR_TYPE__ERROR_PARAM_OR_NOT_IN_RANGE_L:
            printf("Error param or not in range: -l\n");
            printf("Usage: [-l] [number in range %d-%d]\n", PARAM_RANGE_L_START, PARAM_RANGE_L_END);
        default:
            printf("Unknown error. Please try again.\n");
            break;
    }
    // 退出程序
    exit(0);
}

/**
 * checkIpAddress()
 * @description 校验IP地址
 * @param {char *} ipAddress IP地址
 * @return {0|1} IP地址是否符合规范
 */
int checkIpAddress(char *ipAddress) {
    int i, count = 0, number;
    char *temp;

    // 拷贝字符串
    char *ipAddressCopy = (char *) malloc((sizeof char) * strlen(ipAddress) + 1);
    for (i = 0; i < strlen(ipAddress); i++) { ipAddressCopy[i] = ipAddress[i]; }
    ipAddressCopy[i] = '\0';

    // 依次分割字符串，看是否有三个点，并且点之间的每一个字符串都满足数字范围
    temp = strtok(ipAddressCopy, ".");
    while (temp != NULL) {
        count++;
        number = atoi(temp);
        if (number == 0 && temp[0] != '0') { return 0; }
        if (!(number >= 0 && number <= 255)) { return 0; }
    }
    if (count != 3) { return 0; }

    return 1;
}

/**
 * checkIfNumberInRange()
 * @description 校验数字是否在范围内
 * @param {int} target 目标
 * @param {int} start 范围开始
 * @param {int} end 范围结束
 */
int checkIfNumberInRange(int target, int start, int end) {
    if (target < start || target > end) { return 0; }
    return 1;
}

/**
 * ping()
 * @description ping程序行为函数
 * @param {char *} ipAddress IP地址
 * @param {int} sendTime 发送次数
 * @param {int} 包长度
 */
void ping(char *ipAddress, int sendTime, int packageLength) {
    printf("ipAddress: %s\n", ipAddress);
    printf("sendTime: %d\n", sendTime);
    printf("packageLength: %d\n", packageLength);
    // TODO
}
