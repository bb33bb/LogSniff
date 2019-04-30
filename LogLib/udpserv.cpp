#include <WinSock2.h>
#include <Windows.h>
#include "udpserv.h"

CUdpServ::CUdpServ() {
    mServSocket = INVALID_SOCKET;
    mListener = NULL;
}

CUdpServ::~CUdpServ() {
    Close();
}

void CUdpServ::run() {
    char buffer[1024];
    sockaddr_in addr;

    while (true) {
        memset(&addr, 0x00, sizeof(sockaddr_in));
        int fromLen = sizeof(addr);
        int c = recvfrom(mServSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &fromLen);

        if (c <= 0)
        {
            mListener->OnUdpServOnErr();
            break;
        }
        string resp;
        mListener->OnUdpServRecvData(&addr, string(buffer, c), resp);
        sendto(mServSocket, resp.c_str(), resp.size(), 0, (const sockaddr *)&addr, sizeof(addr));
    }
}

bool CUdpServ::InitUdpServ(unsigned short localPort, UdpServEvent *listener) {
    unsigned int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (INVALID_SOCKET == sock)
    {
         return false;
    }

    bool ret = false;
    do 
    {
        sockaddr_in localAddr;
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(localPort);
        localAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

        if (0 != bind(sock, (sockaddr*)&localAddr, sizeof(sockaddr)))
        {
            break;
        }

        mServSocket = sock;
        ret = true;
        mThread.StartThread(this, false);
    } while (false);

    if (!ret && INVALID_SOCKET != sock)
    {
        closesocket(sock);
    }
    return ret;
}