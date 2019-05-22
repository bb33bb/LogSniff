#pragma once
#include <LogLib/tcpclient.h>
#include <LogLib/LogProtocol.h>
#include "../MonitorBase.h"

#define LOGVIEW_VERSION     "1001"

class CServMonitor : public MonitorBase, public ClientEvent {
public:
    virtual bool Init(CMonitorEvent *listener);
    virtual bool Run(const LogServDesc &servDesc);
    virtual bool IsRunning();
    virtual bool Stop();
    virtual std::list<std::mstring> GetPathSet() const;

private:
    void OnSingleResult(const LpResult &result);
    virtual void OnClientConnect(CTcpClient &client);
    virtual void OnClientRecvData(CTcpClient &client, const std::string &strRecved, std::string &strResp);
    virtual void OnClientSocketErr(CTcpClient &client);

private:
    CTcpClient mTcpClient;
    LogServDesc mCfg;
    CMonitorEvent *mListener;
    std::list<std::mstring> mPathSet;
    std::mstring mLogCache;
    bool mInit;
};