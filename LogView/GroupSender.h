#pragma once
#include <string>
#include <LogLib/thread.h>

#define LOG_BROADCAST_ADDR "228.4.5.6"

typedef void (* pfnOnGroupRecv)(const std::string &recvStr, std::string &respStr);

class CGroupSender : public ThreadRunable {
public:
    CGroupSender();
    virtual ~CGroupSender();

    bool Init(pfnOnGroupRecv pfn);
    bool Bind(const std::string &ip, unsigned int port);
    bool Send(const std::string &content);
    std::string Recv(int timeOut);
private:
    std::string GetIpByTable() const;
    std::string GetIpBySocket() const;
    //获取最可能的本地ip地址
    std::string GetLocalIpAddr() const;
    virtual void run();

private:
    unsigned int mSendSock;
    unsigned short mRemotePort;
    std::string mRemoteIp;
    pfnOnGroupRecv mReceiver;
    CThread mThread;
};