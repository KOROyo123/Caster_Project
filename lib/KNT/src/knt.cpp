#include "knt/knt.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <chrono>

#include <unistd.h>
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <windows.h>
#include <psapi.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <sys/resource.h>
#endif

#include <random>

std::string util_random_string(int string_len)
{
    std::string rand_str;

    std::random_device rd;  // non-deterministic generator
    std::mt19937 gen(rd()); // to seed mersenne twister.

    for (int i = 0; i < string_len; i++)
    {
        switch (gen() % 3)
        {
        case 0:
            rand_str += gen() % 26 + 'a';
            break;
        case 1:
            rand_str += gen() % 26 + 'A';
            break;
        case 2:
            rand_str += gen() % 10 + '0';
            break;

        default:
            break;
        }
    }

    return rand_str;
}

std::string util_cal_connect_key(const char *ServerIP, int serverPort, const char *ClientIP, int clientPort)
{
    std::string base = "0123456789ABCDEF"; // 定义16进制表示的基本符号集合

    char hexIp1[9] = "", hexIp2[9] = ""; // 存放转换后的十六进制字符串
    char hexPort1[5] = "", hexPort2[5] = "";

    int octets1[4]; // IPv4地址由四个八位组成
    sscanf(ServerIP, "%d.%d.%d.%d", &octets1[0], &octets1[1], &octets1[2], &octets1[3]);

    for (int i = 0; i < 4; ++i)
    {
        snprintf(hexIp1 + i * 2, sizeof(hexIp1), "%02X", static_cast<unsigned>(octets1[i]));
    }

    int octets2[4]; // IPv4地址由四个八位组成
    sscanf(ClientIP, "%d.%d.%d.%d", &octets2[0], &octets2[1], &octets2[2], &octets2[3]);

    for (int i = 0; i < 4; ++i)
    {
        snprintf(hexIp2 + i * 2, sizeof(hexIp2), "%02X", static_cast<unsigned>(octets2[i]));
    }

    snprintf(hexPort1, sizeof(hexPort1), "%04X", serverPort);
    snprintf(hexPort2, sizeof(hexPort2), "%04X", clientPort);

    char key[25] = "";

    sprintf(key, "%s%s%s%s", hexIp1, hexPort1, hexIp2, hexPort2);

    return std::string() = key;
}

std::string util_cal_half_key(const char *IP, int Port)
{
    std::string base = "0123456789ABCDEF"; // 定义16进制表示的基本符号集合

    char hexIp[9] = "";
    char hexPort[5] = "";

    int octets1[4]; // IPv4地址由四个八位组成
    sscanf(IP, "%d.%d.%d.%d", &octets1[0], &octets1[1], &octets1[2], &octets1[3]);

    for (int i = 0; i < 4; ++i)
    {
        snprintf(hexIp + i * 2, sizeof(hexIp), "%02X", static_cast<unsigned>(octets1[i]));
    }
    snprintf(hexPort, sizeof(hexPort), "%04X", Port);
    char key[25] = "";

    sprintf(key, "%s%s", hexIp, hexPort);
    return std::string() = key;
}

std::string util_cal_connect_key(int fd)
{
    std::string ServerIP, ClientIP;
    int ServerPort, ClientPort;

    // // 获取本地端点信息
    // sockaddr_in localAddress;
    // int localAddressLength = sizeof(localAddress);
    // getsockname(fd, reinterpret_cast<sockaddr*>(&localAddress), &localAddressLength);
    // char localIP[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &localAddress.sin_addr, localIP, INET_ADDRSTRLEN);
    // //std::cout << "Local IP: " << localIP << ", Port: " << ntohs(localAddress.sin_port) << std::endl;

    // ServerIP=localIP;
    // ServerPort=ntohs(localAddress.sin_port);

    // // 获取远程端点信息
    // sockaddr_in remoteAddress;
    // int remoteAddressLength = sizeof(remoteAddress);
    // getpeername(fd, reinterpret_cast<sockaddr*>(&remoteAddress), &remoteAddressLength);
    // char remoteIP[INET_ADDRSTRLEN];
    // inet_ntop(AF_INET, &remoteAddress.sin_addr, remoteIP, INET_ADDRSTRLEN);
    // //std::cout << "Remote IP: " << remoteIP << ", Port: " << ntohs(remoteAddress.sin_port) << std::endl;

    // ClientIP=remoteIP;
    // ClientPort=ntohs(remoteAddress.sin_port);

    struct sockaddr_in sa1;
    socklen_t len1 = sizeof(sa1);
    if (getsockname(fd, (struct sockaddr *)&sa1, &len1))
    {
        return std::string();
    }

    struct sockaddr_in sa2;
    socklen_t len2 = sizeof(sa2);
    if (getpeername(fd, (struct sockaddr *)&sa2, &len2))
    {
        return std::string();
    }
    ServerIP = inet_ntoa(sa1.sin_addr);
    ServerPort = ntohs(sa1.sin_port);
    ClientIP = inet_ntoa(sa2.sin_addr);
    ClientPort = ntohs(sa2.sin_port);

    return util_cal_connect_key(ServerIP.c_str(), ServerPort, ClientIP.c_str(), ClientPort);
}

std::string util_port_to_key(int port)
{
    std::string base = "0123456789ABCDEF"; // 定义16进制表示的基本符号集合

    char key[5] = "";

    snprintf(key, sizeof(key), "%04X", port);
    // if(key.size())

    return std::string() = key;
}

std::string util_get_user_ip(int fd)
{
    struct sockaddr_in sa2;
    socklen_t len2 = sizeof(sa2);
    if (getpeername(fd, (struct sockaddr *)&sa2, &len2))
    {
        return std::string();
    }

    std::string Clientip = inet_ntoa(sa2.sin_addr);
    int clientport = ntohs(sa2.sin_port);

    return Clientip;
}
int util_get_user_port(int fd)
{
    struct sockaddr_in sa2;
    socklen_t len2 = sizeof(sa2);
    if (getpeername(fd, (struct sockaddr *)&sa2, &len2))
    {
        return 0;
    }
    std::string Clientip = inet_ntoa(sa2.sin_addr);
    int clientport = ntohs(sa2.sin_port);

    return clientport;
}
std::string util_get_date_time()
{

    time_t now = time(0);                 // 获取当前时间的time_t类型值
    struct tm *tm_info = localtime(&now); // 将time_t类型值转换为struct tm类型的本地时间信息
    char buffer[80];                      // 存放格式化后的日期时间字符串
    strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", tm_info);

    std::string tm = buffer;

    return tm;
}
std::string util_get_space_time()
{
    return std::string("0000/00/00 00:00:00");
}
std::string util_get_http_date()
{
    std::time_t now = std::time(nullptr);
    std::tm *gmt = std::gmtime(&now);

    std::ostringstream oss;
    oss << std::put_time(gmt, "%a, %d %b %Y %H:%M:%S GMT");
    return oss.str();

    return std::string();
}

int util_get_use_memory()
{

#ifdef WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    SIZE_T virtualMemUsedByMe;
    SIZE_T physicalMemUsedByMe;
    if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), sizeof(pmc)))
    {
        virtualMemUsedByMe = pmc.PrivateUsage;    // 当前进程使用的虚拟内存大小
        physicalMemUsedByMe = pmc.WorkingSetSize; // 当前进程使用的物理内存大小

        // std::cout << "Virtual Memory Used: " << virtualMemUsedByMe / (1024 * 1024) << " MB" << std::endl;
        // std::cout << "Physical Memory Used: " << physicalMemUsedByMe / (1024 * 1024) << " MB" << std::endl;
    }
    else
    {
        virtualMemUsedByMe = 0;  // 当前进程使用的虚拟内存大小
        physicalMemUsedByMe = 0; // 当前进程使用的物理内存大小
        // std::cerr << "GetProcessMemoryInfo failed\n";
    }
    return virtualMemUsedByMe; //

#else
    struct rusage usage;
    // 调用 getrusage() 函数获取当前进程的资源使用情况
    if (getrusage(RUSAGE_SELF, &usage) == -1)
    {
        // std::cerr << "Failed to retrieve resource usage." << std::endl;
        return 0;
    }

    // 输出程序占用的物理内存大小（单位为字节）
    // std::cout << "Memory used by the program in bytes: " << usage.ru_maxrss * 1024 << std::endl;

    return usage.ru_maxrss; // KB
#endif
}
