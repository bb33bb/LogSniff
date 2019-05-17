#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <vector>
#include <LogLib/LogProtocol.h>
#include <LogLib/tcpclient.h>
#include <LogLib/mstring.h>
#include "MonitorBase.h"

struct LogInfoCache {
    std::string mTime;
    std::string mFilePath;
    std::string mContent;
};

#define LOGVIEW_VERSION     "1001"

class CLogReceiver : public CMonitorEvent {
public:
    static CLogReceiver *GetInst();

public:
    CLogReceiver();
    virtual ~CLogReceiver();
    bool Start(MonitorCfg &cfg);
    bool AddPath(const std::mstring &path);
    bool IsStart();
    void Stop();

    void PushLog(const std::mstring &filePath, const std::mstring &content);
    virtual void OnLogReceived(const std::mstring &filePath, const std::mstring &content);

private:
    bool mInit;
    std::vector<LogInfoCache *> mLogSet;
    std::vector<LogInfoCache *> mShowSet;

    std::string mFltStr;
    std::string mLogCache;
    MonitorCfg mCfg;
    MonitorBase *mCurMonitor;
};