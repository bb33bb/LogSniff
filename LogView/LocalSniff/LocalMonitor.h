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
    void SetMonitor(const char *path);

    virtual bool Init(CMonitorEvent *listener);
    virtual bool Run(const LogServDesc &servDesc);
    virtual bool Stop();
    virtual bool IsRunning();
    virtual std::list<std::mstring> GetPathSet() const;

private:
    bool AddPath(const std::mstring &path);
    bool DelPath(const std::mstring &path);
    bool IsFileInCache(const std::mstring &filePath) const;
    LocalLogCache *GetFileCache(const std::mstring &filePath);
    void OnLogReceived(LocalLogCache *cache);
    static bool FileEnumProc(bool isDir, const char *filePath, void *param);
    static void FileNotify(const char *filePath, unsigned int mask);

private:
    bool mInit;
    std::map<std::mstring, DWORD> mPathSet;
    std::map<std::mstring, LocalLogCache *> mLogCache;
    LogServDesc mCfg;
    CMonitorEvent *mListener;
};