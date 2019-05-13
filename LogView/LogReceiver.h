#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <LogLib/LogProtocol.h>
#include <LogLib/tcpclient.h>

struct LogInfoCache {
    std::string mTime;
    std::string mFilePath;
    std::string mContent;
};

#define LOGVIEW_VERSION     "1001"

class CLogReceiver : public ClientEvent {
public:
    static CLogReceiver *GetInst();
    bool ConnectServ(const std::string &ip);
    void DisConnect();

private:
    CLogReceiver();
    virtual ~CLogReceiver();
    void OnSingleResult(const LpResult &result);

    virtual void OnClientConnect(CTcpClient &client);
    virtual void OnClientRecvData(CTcpClient &client, const std::string &strRecved, std::string &strResp);
    virtual void OnClientSocketErr(CTcpClient &client);

private:
    bool mInit;
    CTcpClient mTcpClient;
    std::vector<LogInfoCache *> mLogSet;
    std::vector<LogInfoCache *> mShowSet;

    std::string mFltStr;
    std::string mLogCache;
};