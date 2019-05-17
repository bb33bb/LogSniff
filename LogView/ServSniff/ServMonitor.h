#pragma once
#include <LogLib/tcpclient.h>
#include <LogLib/LogProtocol.h>
#include "../MonitorBase.h"

#define LOGVIEW_VERSION     "1001"

class CServMonitor : public MonitorBase, public ClientEvent {
public:
    virtual bool Init(const MonitorCfg &cfg, CMonitorEvent *listener);
    virtual bool Start();
    virtual bool AddPath(const std::mstring &path);
    virtual bool Stop();
    virtual bool IsStart();
    virtual std::list<std::mstring> GetPathSet() const;

private:
    void OnSingleResult(const LpResult &result);
    virtual void OnClientConnect(CTcpClient &client);
    virtual void OnClientRecvData(CTcpClient &client, const std::string &strRecved, std::string &strResp);
    virtual void OnClientSocketErr(CTcpClient &client);

private:
    CTcpClient mTcpClient;
    MonitorCfg mCfg;
    CMonitorEvent *mListener;
    std::list<std::mstring> mPathSet;
    std::mstring mLogCache;
    bool mInit;
};