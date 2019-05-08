#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "LogProtocol.h"
#include "GroupReceiver.h"
#include "common/Util.h"
#include "common/json/json.h"
#include <stdlib.h>
#include "LogMonitor.h"
#include <stdio.h>

using namespace std;
using namespace Json;

CGroupReceiver *CGroupReceiver::GetInst() {
    static CGroupReceiver *sPtr = 0;

    if (0 == sPtr)
    {
        sPtr = new CGroupReceiver();
    }
    return sPtr;
}

void CGroupReceiver::InitRecviver(unsigned short port) {
    mSocket = socket(AF_INET, SOCK_DGRAM, 0);

    int reuse = 1;
    setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));

    sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    bind(mSocket, (const sockaddr *)&localAddr, sizeof(localAddr));

    unsigned int loop = 1;
    if (setsockopt(mSocket, IPPROTO_IP,IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0)
    {
        dp("error1");
    }

    struct ip_mreq group;
    group.imr_interface.s_addr = INADDR_ANY;
    group.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);

    if (setsockopt(mSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        dp("error2");
    }
    mThread.StartThread(this, false);
}

CGroupReceiver::CGroupReceiver() {
}

CGroupReceiver::~CGroupReceiver() {
}

/*
{
    "cmd":"scan"
}

{
    "cmd":"desc",
    "ipSet":["10.10.16.191"],
    "pathSet":["/gdsoft/logs"]
}
*/
void CGroupReceiver::OnRecvData(const std::string &recvStr, std::string &outStr) const {
    Value content;
    Reader().parse(recvStr, content);

    dp("group recv:%s", recvStr.c_str());
    if (content.type() != objectValue || content["cmd"].type() != stringValue)
    {
        dp("parser err");
        return;
    }
    string cmd = content["cmd"].asString();

    extern time_t gStartTime;
    CLogProtocol::GetInst()->EncodeDesc(CLogMonitor::GetInst()->GetPathSet(), gStartTime, outStr);
}

void CGroupReceiver::run() {
    char recvBuffer[1024];
    struct sockaddr_in peerAddr;
    while (true) {
        int bufLen = sizeof(recvBuffer);
        memset(&peerAddr, 0x00, sizeof(peerAddr));
        unsigned int addrLen = sizeof(peerAddr);

        dp("test1");
        int count = recvfrom(mSocket, recvBuffer, bufLen, 0, (sockaddr *)&peerAddr, &addrLen);
        dp("test2 c:%d", count);

        if (count <= 0)
        {
            break;
        }

        string recvStr(recvBuffer, count);
        dp("recv:%s", recvStr.c_str());
        string respStr;
        GetInst()->OnRecvData(recvStr, respStr);

        if (!respStr.empty())
        {
            sendto(mSocket, respStr.c_str(), respStr.size(), 0, (const sockaddr *)&peerAddr, sizeof(peerAddr));
        }
    }
    close(mSocket);
}