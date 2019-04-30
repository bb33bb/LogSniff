#ifndef TCPSERV_GDLIB_H_H_
#define TCPSERV_GDLIB_H_H_
#ifdef __linux__
#include <sys/unistd.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#endif //__linux__

#include <string>
#include <vector>
#include "SocketDef.h"
#include "thread.h"

class ServEvent {
public:
    virtual void OnServAccept(unsigned int client) = 0;
    virtual void OnServRecvData(unsigned int client, const std::string &strRecved, std::string &strResp) = 0;
    virtual void OnServSocketErr(unsigned int client) = 0;
    virtual void OnServSocketClose(unsigned int client) = 0;
};

class CTcpServ : public ThreadRunable {
public:
    CTcpServ();
    bool InitServ(unsigned short uLocalPort, ServEvent *listener);
    void Close();

private:
    bool SetKeepAlive();
    bool Bind(const std::string &ip, unsigned short uPort, int iTimeOut);

    //work thread
    virtual void run();
private:
    CThread mThread;
    ServEvent *mListener;
    bool mInit;
    bool mStop;
    SOCKET mServSocket;
    unsigned short mLocalPort;
    std::vector<unsigned int> mClientSet;
};
#endif