#include <WinSock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include "GroupSender.h"
#include <LogLib/LogUtil.h>
#include <LogLib/tcpclient.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

CGroupSender::CGroupSender() {
}

CGroupSender::~CGroupSender() {
}

bool CGroupSender::Init(pfnOnGroupRecv receiver) {
    string ip = GetLocalIpAddr();

    mSendSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;

    if (ip.empty())
    {
        localAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    } else {
        localAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    }

    localAddr.sin_port = htons(8891);
    if (0 != bind(mSendSock, (struct sockaddr *)&localAddr, sizeof (localAddr)))
    {
        closesocket(mSendSock);
        mSendSock = 0;
        return false;
    }
    mReceiver = receiver;
    mThread.StartThread(this, false);
    return true;
}

bool CGroupSender::Bind(const string &ip, unsigned int port) {
    mRemoteIp = ip;
    mRemotePort = port;

    return true;
}

bool CGroupSender::Send(const std::string &content) {
    sockaddr_in addr;
    addr.sin_addr.S_un.S_addr = inet_addr(mRemoteIp.c_str());
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mRemotePort);

    int c = sendto(mSendSock, content.c_str(), content.size(), 0, (const sockaddr *)&addr, sizeof(addr));
    int d = WSAGetLastError();
    return true;
}

string CGroupSender::GetIpBySocket() const {
    unsigned int fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (INVALID_SOCKET == fd)
    {
        return "";
    }

    hostent *result = gethostbyname("www.baidu.com");
    sockaddr_in servAddr ;
    servAddr.sin_addr.S_un.S_addr = (*(u_long *)result->h_addr_list[0]);
    servAddr.sin_family = AF_INET ;
    servAddr.sin_port = htons(80);

    string dd = inet_ntoa(servAddr.sin_addr);

    unsigned long ul = 1;
    ioctlsocket(fd, FIONBIO, (unsigned long*)&ul);

    bool bStat = false;
    string localIp;
    if (0 == connect(fd, (sockaddr *)&servAddr, sizeof(servAddr)))
    {
        bStat = true;
    } else {
        struct timeval timeout = {0};
        fd_set r, w;
        FD_ZERO(&r);
        FD_ZERO(&w);
        FD_SET(fd, &r);
        FD_SET(fd, &w);
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000 * 1000;

        int res = 0;
        if ((res = select(fd + 1, &r, &w, 0, &timeout)) > 0)
        {
            if (FD_ISSET(fd, &w))
            {
                bStat = true;
            }
        }
    }
    ul = 0 ;
    ioctlsocket(fd, FIONBIO, (unsigned long*)&ul);

    if (bStat)
    {
        sockaddr_in localAddr = {0};
        int addrLen = sizeof(localAddr);

        getsockname(fd, (sockaddr *)&localAddr, &addrLen);
        localIp = inet_ntoa(localAddr.sin_addr);
    }

    int ee = WSAGetLastError();
    closesocket(fd);
    return localIp;
}

string CGroupSender::GetIpByTable() const {
    char buffer[4096];
    DWORD bufSize = sizeof(buffer);
    char *ptr = buffer;
    bool delBuf = false;
    bool stat = false;

    stat = (NO_ERROR == GetTcpTable((PMIB_TCPTABLE)ptr, &bufSize, FALSE));
    if (!stat && (bufSize > sizeof(buffer)))
    {
        delBuf = true;
        bufSize += 1024;
        ptr = new char[bufSize];
        stat = (NO_ERROR == GetTcpTable((PMIB_TCPTABLE)ptr, &bufSize, FALSE));
    }

    string maxIp;
    if (stat)
    {
        int maxCount = 0;
        map<mstring, int> ss;
        PMIB_TCPTABLE pTable = (PMIB_TCPTABLE)ptr;
        for (int i = 0 ; i < (int)pTable->dwNumEntries ; i++)
        {
            MIB_TCPROW *tb = pTable->table + i;
            if (tb->dwState == MIB_TCP_STATE_LISTEN || tb->dwState == MIB_TCP_STATE_ESTAB)
            {
                in_addr addr;
                addr.S_un.S_addr = tb->dwLocalAddr;
                mstring localIp = inet_ntoa(addr);
                if (localIp.startwith("127.0") || (localIp == "0.0.0.0"))
                {
                    continue;
                }

                ss[localIp]++;
                if (ss[localIp] > maxCount)
                {
                    maxCount = ss[localIp];
                    maxIp = localIp;
                }
            }
        }
    }

    if (delBuf)
    {
        delete []ptr;
    }
    return maxIp;
}

string CGroupSender::GetLocalIpAddr() const {
    vector<AdapterMsg> adSet;
    GetAdapterSet(adSet);

    if (adSet.empty())
    {
        return "";
    }

    if (adSet.size() == 1)
    {
        return adSet[0].m_ip;
    }

    string ip = GetIpBySocket();
    if (!ip.empty())
    {
        return ip;
    }

    return GetIpByTable();
}

std::string CGroupSender::Recv(int timeOut) {
    DWORD tmp = timeOut;
    setsockopt(mSendSock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeOut, sizeof(DWORD));

    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;

    char buffer[1024];
    int addrLen = sizeof(addr);
    int c = recvfrom(mSendSock, buffer, sizeof(buffer), 0, (sockaddr *)&addr, &addrLen);

    int dd = WSAGetLastError();
    if (c > 0)
    {
        return string(buffer, c);
    }
    return "";
}

void CGroupSender::run() {
    char buffer[1024];
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;

    while (true) {
        int addrLen = sizeof(addr);
        int c = recvfrom(mSendSock, buffer, sizeof(buffer), 0, (sockaddr *)&addr, &addrLen);

        if (c <= 0)
        {
            break;
        }

        string reply;
        mReceiver(string(buffer, c), reply);

        if (!reply.empty())
        {
        }
    }
}