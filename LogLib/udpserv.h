#pragma once
#include <Windows.h>
#include <string>
#include "thread.h"

using namespace std;

class UdpServEvent {
public:
    virtual void OnUdpServRecvData(sockaddr_in *addr, const string &strRecved, string &strResp) = 0;
    virtual void OnUdpServOnErr() = 0;
};

class CUdpServ : public  ThreadRunable {
public:
    CUdpServ();
    virtual ~CUdpServ();

    bool InitUdpServ(unsigned short localPort, UdpServEvent *listener);
    void Close();

private:
    virtual void run();

private:
    unsigned int mServSocket;
    UdpServEvent *mListener;
    CThread mThread;
};