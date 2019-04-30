#pragma  once

#ifndef SOCKET
#define SOCKET unsigned int
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1 //for linux
#endif //INVALID_SOCKET

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1   //for linux
#endif

#ifdef __linux__
#include <unistd.h>
#define SleepMS(n) sleep(n / 1000)
#define GetSockErr() errno
#define closesocket close
#else
#define SleepMS(n) Sleep(n)
#define GetSockErr() WSAGetLastError()
#endif //__linux__