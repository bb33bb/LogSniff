#pragma once
#include <Windows.h>
#include <string>
#include <list>
#include <map>
#include <LogLib/locker.h>
#include <LogLib/mstring.h>
#include <LogLib/TextDecoder.h>
#include "../MonitorBase.h"

class CLocalMonitor : public RLocker, public MonitorBase {
    struct LocalLogCache {
        std::mstring mFilePath;
        long mLastPos;
        unsigned int mLastModified;
        unsigned long mFileSize;
        std::mstring mLastCache;
        TextEncodeType mEncodeType;

        LocalLogCache() {
            mLastPos = 0;
            mLastModified = 0;
            mFileSize = 0;
            mEncodeType = em_text_unknown;
        }
    };

public:
    static CLocalMonitor *GetInst();
    void SetMonitor(const char *path);

    virtual bool Init(CMonitorEvent *listener);
    virtual bool Run(const LogServDesc *desc);
    virtual bool Stop();
    virtual bool IsRunning();
    virtual std::list<std::mstring> GetPathSet() const;

private:
    CLocalMonitor();
    virtual ~CLocalMonitor();

    bool AddPath(const std::mstring &path);
    bool DelPath(const std::mstring &path);
    bool IsFileInCache(const std::mstring &filePath) const;
    LocalLogCache *GetFileCache(const std::mstring &filePath);
    void OnLogReceived(LocalLogCache *cache);
    static bool FileEnumProc(bool isDir, const char *filePath, void *param);
    static void FileNotify(const char *filePath, unsigned int mask);

    TextEncodeType GetFileEncodeType(const std::mstring &filePath, long lastPos = 0) const;
    std::mstring GetFileContent(const std::mstring &filePath, long lastPos);
private:
    bool mInit;
    std::map<std::mstring, DWORD> mPathSet;
    std::map<std::mstring, LocalLogCache *> mLogCache;
    const LogServDesc *mCfg;
    CMonitorEvent *mListener;
};