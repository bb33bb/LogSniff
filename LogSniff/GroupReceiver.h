#pragma once
#include <string>
#include <thread.h>

class CGroupReceiver : public ThreadRunable {
public:
    static CGroupReceiver *GetInst();
    void InitRecviver(unsigned short port);

private:
    CGroupReceiver();
    virtual ~CGroupReceiver();
    void OnRecvData(const std::string &recvStr, std::string &outStr) const;

    virtual void run();
private:
    unsigned int mLocalPort;
    int mSocket;
    CThread mThread;
};