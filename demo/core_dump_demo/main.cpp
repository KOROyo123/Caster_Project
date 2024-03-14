#include "iostream"


#include <sys/prctl.h>
#include <sys/resource.h>

int main(int argc, char *argv[])
{

    prctl(PR_SET_DUMPABLE, 1);

    struct rlimit rlimit_core;
    rlimit_core.rlim_cur = RLIM_INFINITY;
    rlimit_core.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &rlimit_core);



    char *addr = (char *)0; // 设置 addr 变量为内存地址 "0"
 
    *addr = '\0';           // 向内存地址 "0" 写入数据
 
    return 0;
}