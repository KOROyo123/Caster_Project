// #include <coroutine>
// #include <iostream>

// struct HelloCoroutine
// {
//     struct HelloPromise
//     {
//         HelloCoroutine get_return_object()
//         {
//             std::cout << "==============call get_return_object==============" << std::endl;
//             return std::coroutine_handle<HelloPromise>::from_promise(*this);
//         }
//         std::suspend_never initial_suspend()
//         {
//             std::cout << "==============call initial_suspend==============" << std::endl;
//             return {};
//         }

//         std::suspend_always final_suspend() noexcept
//         {
//             std::cout << "==============call final_suspend==============" << std::endl;
//             return {};
//         }
//         void unhandled_exception() {}
//     };

//     using promise_type = HelloPromise;
//     HelloCoroutine(std::coroutine_handle<HelloPromise> h) : handle(h) {}

//     std::coroutine_handle<HelloPromise> handle;
// };

// std::coroutine_handle<> coroutine_handle;
// struct AWaitableObject
// {
//     AWaitableObject() {}

//     bool await_ready() const
//     {
//         std::cout << "==============call await_ready==============" << std::endl;
//         return false;
//     }

//     int await_resume()
//     {
//         std::cout << "==============call await_resume==============" << std::endl;
//         return 0;
//     }

//     void await_suspend(std::coroutine_handle<> handle)
//     {
//         std::cout << "==============call await_suspend==============" << std::endl;
//         coroutine_handle = handle;
//     }
// };

// HelloCoroutine hello()
// {
//     std::cout << "Hello " << std::endl;
//     co_await AWaitableObject{};
//     std::cout << "world!" << std::endl;
// }

// int main()
// {
//     std::cout << "start" << std::endl;

//     HelloCoroutine coro1 = hello();
//     HelloCoroutine coro2 = hello();
//     HelloCoroutine coro3 = hello();
//     HelloCoroutine coro4 = hello();

//     std::cout << "calling resume" << std::endl;
//     coro1.handle.resume();
//     coro2.handle.resume();
//     coro3.handle.resume();
//     coro4.handle.resume();


//     std::cout << "destroy" << std::endl;
//     coro1.handle.destroy();
//     coro2.handle.destroy();
//     coro3.handle.destroy();
//     coro4.handle.destroy();

//     return 0;
// }


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
 
#define BUF_SIZE 500
 
int main(int argc, char **argv)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];
 
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
 
    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
 
    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */
 
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;
 
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */
 
        close(sfd);
    }
 
    freeaddrinfo(result);           /* No longer needed */
 
    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
 
    /* Read datagrams and echo them back to sender. */
 
    for (;;) {
        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
            continue;               /* Ignore failed request */
 
        char host[NI_MAXHOST], service[NI_MAXSERV];
 
        s = getnameinfo((struct sockaddr *) &peer_addr,
                        peer_addr_len, host, NI_MAXHOST,
                        service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0)
            printf("Received %zd bytes from %s:%s\n",
                    nread, host, service);
        else
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
 
        if (sendto(sfd, buf, nread, 0,
                    (struct sockaddr *) &peer_addr,
                    peer_addr_len) != nread)
            fprintf(stderr, "Error sending response\n");
    }
 
    return 0;
}