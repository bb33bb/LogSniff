#pragma once
#include <list>
#include <set>
#include <LogLib/mstring.h>

enum MonitorType {
    em_monitor_local,
    em_monitor_remote
};

struct MonitorCfg {
    MonitorType mType;
    std::list<std::mstring> mPathSet;
    std::mstring mServIp;

    bool operator ==(const MonitorCfg &other) const {
        return (
            mType == other.mType &&
            mPathSet == other.mPathSet &&
            mServIp == other.mServIp
            );
    }
};

class CMonitorEvent {
public:
    virtual void OnLogReceived(const std::mstring &filePath, const std::mstring &content) = 0;
};

class MonitorBase {
public:
    virtual bool Init(const MonitorCfg &cfg, CMonitorEvent *listener) = 0;
    virtual bool Start() = 0;
    virtual bool AddPath(const std::mstring &path) = 0;
    virtual bool Stop() = 0;
    virtual bool IsStart() = 0;
    virtual std::list<std::mstring> GetPathSet() const = 0;

public:
    static bool IsLogFile(const std::mstring &filePath) {
        std::mstring low = filePath;
        low.makelower();
        return (low.endwith(".txt") || low.endwith(".log"));
    }
};