#ifndef MSGCLIENT_DPMSG_H_H_
#define MSGCLIENT_DPMSG_H_H_
#ifdef __linux__
#include <sys/socket.h>
#else
#include <WinSock2.h>
#include <Windows.h>
#endif //__linux__
#include <string>
#include "thread.h"
#include "SocketDef.h"

class CTcpClient;
class ClientEvent {
public:
    virtual void OnClientConnect(CTcpClient &client) {}
    virtual void OnClientRecvData(CTcpClient &client, const std::string &strRecved, std::string &strResp) = 0;
    virtual void OnClientSocketErr(CTcpClient &client) = 0;
};

class CTcpClient : public ThreadRunable {
public:
    CTcpClient();
    bool InitClient(const std::string &strIp, unsigned short uPort, ClientEvent *pListener, int iTimeOut);
    bool Send(const std::string &strMsg) const;
    void Close();

private:
    bool TestConnect();
    bool Connect(const std::string &strIp, unsigned short uPort, int iTimeOut);

    //thread 
    virtual void run();

private:
    ClientEvent *mListener;
    bool mInit;
    bool mStop;
    bool mTestConnent;
    SOCKET mClientSock;
    std::string mSerIp;
    unsigned short mServPort;
    CThread mThread;
};
#endif