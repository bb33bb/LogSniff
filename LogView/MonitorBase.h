#pragma once
#include <list>
#include <set>
#include <LogLib/LogUtil.h>
#include <LogLib/mstring.h>
#include <LogLib/LogProtocol.h>

using namespace std;

enum LogServType {
    em_log_serv_local,
    em_log_serv_remote
};

enum LogServConnectStat {
    em_log_serv_disconnected,
    em_log_serv_connected,
    em_log_serv_connect_faild
};

enum LogServActiveStat {
    em_log_serv_alive,
    em_log_serv_closed
};

struct LocalServDesc {
    mstring mUnique;
    mstring mSystem;
    mstring mStartTime;
    list<mstring> mPathSet;

    LocalServDesc() {
        static mstring sOsVersion;

        if (sOsVersion.empty())
        {
            sOsVersion = GetOSVersion();
        }

        extern mstring gStartTime;
        mStartTime = gStartTime;
        mSystem = sOsVersion;
    }
};

typedef LpServDesc RemoteServDesc;

struct LogServDesc {
    LogServType mLogServType;
    LogServConnectStat mConnectStat;
    LogServActiveStat mAliveStat;

    LocalServDesc mLocalServDesc;
    RemoteServDesc mRemoteServDesc;

    LogServDesc() {
        mConnectStat = em_log_serv_disconnected;
        mAliveStat = em_log_serv_closed;
    }
};

class CMonitorEvent {
public:
    virtual void OnLogReceived(const std::mstring &filePath, const std::mstring &content) = 0;
};

class MonitorBase {
public:
    virtual bool Init(CMonitorEvent *listener) = 0;
    virtual bool Run(const LogServDesc &servDesc) = 0;
    virtual bool Stop() = 0;
    virtual bool IsRunning() = 0;
    virtual std::list<std::mstring> GetPathSet() const = 0;

public:
    static bool IsLogFile(const std::mstring &filePath) {
        std::mstring low = filePath;
        low.makelower();
        return (low.endwith(".txt") || low.endwith(".log"));
    }
};