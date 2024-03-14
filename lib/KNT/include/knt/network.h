#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
 

#include <string>



#ifdef _WIN32
#define knt_socket_t intptr_t
#else
#define knt_socket_t int
#endif




knt_socket_t tcpcli_connect_to(std::string host,int port);

knt_socket_t tcpsvr_listen_to(int port);
