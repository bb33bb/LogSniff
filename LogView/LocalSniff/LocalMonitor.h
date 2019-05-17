#pragma once
#include <Windows.h>
#include <string>
#include <list>
#include <map>
#include <LogLib/locker.h>
#include <LogLib/mstring.h>
#include "../MonitorBase.h"

class CLocalMonitor : public RLocker, public MonitorBase {
    struct LocalLogCache {
        std::mstring mFilePath;
        long mLastPos;
        unsigned int mLastModified;
        unsigned long mFileSize;
        std::mstring mLastCache;

        LocalLogCache() {
            mLastPos = 0;
            mLastModified = 0;
            mFileSize = 0;
        }
    };

public:
    static CLocalMonitor *GetInst();
    void InitMonitor();
    void SetMonitor(const char *path);

    virtual bool Init(const MonitorCfg &cfg, CMonitorEvent *listener);
    virtual bool Start();
    virtual bool AddPath(const std::mstring &path);
    virtual bool Stop();
    virtual bool IsStart();
    virtual std::list<std::mstring> GetPathSet() const;

private:
    LocalLogCache *GetFileCache(const std::mstring &filePath);
    void OnLogReceived(LocalLogCache *cache);
    static void FileNotify(const char *filePath, unsigned int mask);
private:
    std::list<std::mstring> mPathSet;
    std::map<std::mstring, LocalLogCache *> mLogCache;
    MonitorCfg mCfg;
    CMonitorEvent *mListener;
};