#include "tcpclient.h"
#include "LogUtil.h"
#include "tcpclient.h"

#ifdef __linux__
#include<sys/socket.h>
#include<sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <errno.h>
#else
#pragma comment(lib, "Ws2_32.lib")
#endif //__linux__

using namespace std;

CTcpClient::CTcpClient() :
mListener(NULL),
mInit(false),
mStop(false),
mClientSock(INVALID_SOCKET),
mServPort(0),
mTestConnent(false) {
}

bool CTcpClient::Connect(const string &strIp, unsigned short uPort, int iTimeOut) {
    sockaddr_in servAddr ;
    servAddr.sin_family = AF_INET ;
    servAddr.sin_port = htons(uPort);
#ifdef __linux__
    int flags = fcntl(mClientSock, F_GETFL);
    int d2 = fcntl(mClientSock, F_SETFL, flags | O_NONBLOCK);
    dp(L"flags:%d b:%d c:%d sock:%d", flags, d2, errno, mClientSock);
    servAddr.sin_addr.s_addr = inet_addr(strIp.c_str());
#else
    unsigned long ul = 1;
    ioctlsocket(mClientSock, FIONBIO, (unsigned long*)&ul);
    servAddr.sin_addr.S_un.S_addr = inet_addr(strIp.c_str());
#endif

    bool bStat = false;
    if (0 == connect(mClientSock, (sockaddr *)&servAddr, sizeof(servAddr)))
    {
        dp(L"connect success1");
        bStat = true;
    } else {
        dp(L"111222");
        struct timeval timeout = {0};
        fd_set r, w;
        FD_ZERO(&r);
        FD_ZERO(&w);
        FD_SET(mClientSock, &r);
        FD_SET(mClientSock, &w);
        timeout.tv_sec = 0;
        timeout.tv_usec = iTimeOut * 1000;

#ifdef __linux__
        if (errno == EINPROGRESS)
        {
            int res = 0;
            if ((res = select(mClientSock + 1, &r, &w, 0, &timeout)) > 0)
            {
                if (FD_ISSET(mClientSock, &w))
                {
                    int error = 0;
                    socklen_t length = sizeof( error );
                    if(getsockopt(mClientSock, SOL_SOCKET, SO_ERROR, &error, &length ) < 0 || error != 0){
                        Logger::logger.warning( "get socket option failed, err:%d\n" ,error);
                    } else {
                        dp(L"result222:%d", res);
                        bStat = true;
                    }
                } 
            } else {
                bStat = false;
            }
        }
        fcntl(mClientSock, F_SETFL, flags);
#else
        int res = 0;
        if ((res = select(mClientSock + 1, &r, &w, 0, &timeout)) > 0)
        {
            if (FD_ISSET(mClientSock, &w))
            {
                bStat = true;
            }
        }
        unsigned long ul1= 0 ;
        ioctlsocket(mClientSock, FIONBIO, (unsigned long*)&ul1);
#endif //__linux__
    }

    if (false == bStat)
    {
        dp(L"ioctlsocket err:%d", GetSockErr());
        closesocket(mClientSock);
        mClientSock = INVALID_SOCKET;
        return false;
    }
    dp(L"connect success");
    return true;
}

bool CTcpClient::InitClient(const string &strIp, unsigned short uPort, ClientEvent *pListener, int iTimeOut) {
    if (mInit)
    {
        return true;
    }

    mStop = false;
    mListener = pListener;
    mClientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == mClientSock)
    {
        return false;
    }

    mSerIp = strIp;
    mServPort = uPort;
    if (Connect(strIp, uPort, iTimeOut)) {
        mInit = true;
        mListener->OnClientConnect(*this);
        mThread.StartThread(this, false);
        return true;
    }
    closesocket(mClientSock);
    mClientSock = INVALID_SOCKET;
    return false;
}

bool CTcpClient::Send(const string &strMsg) const {
    if (!mInit) {
        return false;
    }
    return (SOCKET_ERROR != ::send(mClientSock, strMsg.c_str(), static_cast<int>(strMsg.size()), 0));
}

void CTcpClient::Close() {
    if (mInit && INVALID_SOCKET != mClientSock)
    {
        mStop = true;
        closesocket(mClientSock);
        mThread.WaitThread(3);
        mClientSock = INVALID_SOCKET;
        mInit = false;
    }
}

bool CTcpClient::TestConnect() {
    mClientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    dp(L"test connect ip:%hs, port:%d", mSerIp.c_str(), mServPort);
    if (Connect(mSerIp, mServPort, 3000))
    {
        mListener->OnClientConnect(*this);
        mTestConnent = false;
        return true;
    } else {
        mTestConnent = true;
        return false;
    }
}

void CTcpClient::run() {
    char buffer[2048];
    int iRecv = 0;
    while (true) {
        if (mTestConnent)
        {
            if (!TestConnect())
            {
                dp(L"test connect err:%d", GetSockErr());
                SleepMS(5000);
                continue;
            }
        }

        if ((iRecv = ::recv(mClientSock, buffer, sizeof(buffer), 0)) > 0) {
            dp(L"recv:%d", iRecv);

            string strResp;
            mListener->OnClientRecvData(*this, string(buffer, iRecv), strResp);

            if (!strResp.empty())
            {
                Send(strResp);
            }
        } else {
            dp(L"recv data err:%d", GetSockErr());
            mListener->OnClientSocketErr(*this);
            //close for socket err and test connect again
            closesocket(mClientSock);
            mClientSock = INVALID_SOCKET;
            mTestConnent = true;

            if (mStop) {
                dp(L"stop socket");
                break;
            }
        }
    }
}