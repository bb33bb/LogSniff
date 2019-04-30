#include "tcpserv.h"
#include "LogUtil.h"
#ifdef __linux__
#include<sys/socket.h>
#include<sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include<netinet/in.h>
#include <netinet/tcp.h>
#include<arpa/inet.h>
#include <errno.h>
#else
#include <MSTcpIP.h>
#pragma comment(lib, "Ws2_32.lib")
#endif //__linux__

using namespace std;

CTcpServ::CTcpServ() {
    mListener = NULL;
    mInit = 0;
    mStop = 0;
    mServSocket = INVALID_SOCKET;
    mLocalPort = 0;
}

bool CTcpServ::SetKeepAlive()
{
#ifdef __linux__
    int keepalive = 1;
    int keepidle = 60;
    int keepinterval = 5;
    int keepcount = 3;
    setsockopt(mServSocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive ));
    setsockopt(mServSocket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle , sizeof(keepidle ));
    setsockopt(mServSocket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval ));
    setsockopt(mServSocket, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount ));
    return true;
#else
    tcp_keepalive live = {0};
    tcp_keepalive liveout = {0};
    live.keepaliveinterval = 1000 * 10;
    live.keepalivetime = 1000 * 10;
    live.onoff = 1;
    int iKeepLive = 1;
    int iRet = setsockopt(mServSocket, SOL_SOCKET, SO_KEEPALIVE,(char *)&iKeepLive, sizeof(iKeepLive));
    if(iRet == 0){
        DWORD dw;
        if(WSAIoctl(mServSocket, SIO_KEEPALIVE_VALS, &live, sizeof(live), &liveout, sizeof(liveout), &dw, NULL, NULL)== SOCKET_ERROR)
        {
            return false;
        }
    }
    return true;
#endif
}

bool CTcpServ::InitServ(unsigned short uLocalPort, ServEvent *pListener) {
    mServSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == mServSocket)
    {
        return 0;
    }

    bool bResult = 0;
    do
    {
#ifdef __linux__
        int opt = 1;
        if(setsockopt(mServSocket, SOL_SOCKET,SO_REUSEADDR, (const void *) &opt, sizeof(opt))){
            dp(L"setsockopt err");
            return -1;
        }
#endif //__linux__

        struct sockaddr_in sin;
        sin.sin_family = AF_INET;
        sin.sin_port = htons(uLocalPort);

#ifdef __linux__
        sin.sin_addr.s_addr = INADDR_ANY;
#else
        sin.sin_addr.S_un.S_addr = INADDR_ANY;
#endif

        if(bind(mServSocket,(const sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
        {
            break;
        }

        //监听
        if(listen(mServSocket, 5) == SOCKET_ERROR)
        {
            break;
        }

        SetKeepAlive();
        mListener = pListener;
        mThread.StartThread(this, false);
        bResult = 1;
        mInit = 1;
    } while (0);

    if (!bResult && INVALID_SOCKET != mServSocket)
    {
        closesocket(mServSocket);
        mServSocket = INVALID_SOCKET;
    }
    return bResult;
}

void CTcpServ::Close() {
    return;
}

void CTcpServ::run()
{
    fd_set writeSet;
    fd_set readSet;
    fd_set errSet;
    vector<SOCKET>::iterator it;
    int iBufSize = (1024 * 1024 * 4);
    char *buffer = new char[iBufSize];
    unsigned int maxSocket = mServSocket;

    while (1)
    {
        FD_ZERO(&readSet);
        FD_ZERO(&writeSet);
        FD_ZERO(&errSet);

        FD_SET(mServSocket, &readSet);
        FD_SET(mServSocket, &errSet);
        maxSocket = mServSocket;
        for (it = mClientSet.begin() ; it != mClientSet.end() ; it++)
        {
            if (*it > maxSocket)
            {
                maxSocket = *it;
            }

            FD_SET(*it, &readSet);
            FD_SET(*it, &writeSet);
            FD_SET(*it, &errSet);
        }

        int res = 0;
        if ((res = select(maxSocket + 1, &readSet, 0, &errSet, NULL)) != SOCKET_ERROR)
        {
            if (SOCKET_ERROR == res)
            {
                dp(L"select err:%d", GetSockErr());
                break;
            }

            if (FD_ISSET(mServSocket, &errSet))
            {
                dp(L"server socket err:%d", GetSockErr());
                break;
            }

            if (FD_ISSET(mServSocket, &readSet))
            {
                //新连接
                SOCKET client = accept(mServSocket, NULL, NULL);
                if (client > maxSocket)
                {
                    maxSocket = client;
                }

                dp(L"accept");
                if (client != INVALID_SOCKET)
                {
                    mClientSet.push_back(client);
                    mListener->OnServAccept(client);
                }
            }

            for (it = mClientSet.begin() ; it != mClientSet.end() ;)
            {
                bool bDelete = 0;
                SOCKET sock = *it;
                if (FD_ISSET(sock, &readSet))
                {
                    {
                        //接收数据
                        int iSize = recv(sock, buffer, iBufSize, 0);
                        if (iSize > 0)
                        {
                            string strResp;
                            mListener->OnServRecvData(sock, string(buffer, iSize), strResp);
                            if (strResp.size() > 0)
                            {
                                ::send(sock, strResp.c_str(), static_cast<int>(strResp.size()), 0);
                            }
                        }
                        //接收出错
                        else
                        {
                            dp(L"recv data err:%d", GetSockErr());
                            mListener->OnServSocketClose(sock);
                            closesocket(sock);
                            bDelete = 1;
                        }
                    }
                }

                if (FD_ISSET(sock, &errSet))
                {
                    dp(L"client socket err:%d", GetSockErr());
                    mListener->OnServSocketClose(sock);
                    closesocket(sock);
                    bDelete = 1;
                }

                if (bDelete)
                {
                    it = mClientSet.erase(it);
                    continue;
                }
                it++;
            }
        } else {
        }
    }

    if (mServSocket != INVALID_SOCKET)
    {
        dp(L"serv socket close");
        closesocket(mServSocket);
        mServSocket = INVALID_SOCKET;
    }
    delete []buffer;
}