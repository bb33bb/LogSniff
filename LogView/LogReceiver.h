#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <LogLib/LogProtocol.h>
#include <LogLib/tcpclient.h>
#include <LogLib/mstring.h>
#include "MonitorBase.h"
#include "LogServMgr.h"

struct LogInfoCache {
    std::string mTime;
    std::string mFilePath;
    std::string mContent;
};

#define LOGVIEW_VERSION     "1001"

class CLogReceiver : public CMonitorEvent, public LogServEvent {
public:
    static CLogReceiver *GetInst();
    void InitReceiver();

public:
    CLogReceiver();
    virtual ~CLogReceiver();
    bool Run(const LogServDesc *cfg);
    bool IsRunning();
    void Stop();

    void PushLog(const std::mstring &filePath, const std::mstring &content);
    virtual void OnLogReceived(const std::mstring &filePath, const std::mstring &content);

    //LogServ Event
    virtual void OnLogServAdd(const LogServDesc *d);
    virtual void OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2);
    virtual void OnLogServAlter(const LogServDesc *d);
private:
    bool mInit;
    std::vector<LogInfoCache *> mLogSet;
    std::vector<LogInfoCache *> mShowSet;

    std::string mFltStr;
    std::string mLogCache;
    const LogServDesc *mCfg;
    MonitorBase *mCurMonitor;
};