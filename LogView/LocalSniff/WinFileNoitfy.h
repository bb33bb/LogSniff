#pragma once
#include <Windows.h>
#include <string>
#include <set>
#include <map>
#include <list>
#include <LogLib/thread.h>
#include <LogLib/locker.h>

#define FD_NOTIFY_MODIFIED      (1 << 0)
#define FD_NOTIFY_CREATE        (1 << 2)
#define FD_NOTIFY_DELETE        (1 << 3)
#define FD_NOTIFY_ALL           (-1)

typedef unsigned int HFileNotify;
#define INVALID_HFileNotify -1
typedef void (* pfnWinFileNotify)(const char *filePath, unsigned int mask);

struct FileNotifyRegister {
    HFileNotify mIndex;
    pfnWinFileNotify mNotify;
    unsigned int mask;
    std::string mMonitorPath;
    bool mWithSubDir;
    std::set<int> mWdSet;

    FileNotifyRegister() {
        mIndex = 0;
        mNotify = NULL;
        mask = 0;
        mWithSubDir = false;
    }
};

class CWinFileNotify : public ThreadRunable, public RLocker {
    struct IoInfo {
        HANDLE mIocp;
        std::string mDirPath;
        OVERLAPPED ol;

        union {
            FILE_NOTIFY_INFORMATION mNotify;
            char mBuffer[4096];
        };
        
        HANDLE mhDir;
        DWORD mIndex;
        pfnWinFileNotify pfnFileNotify;

        IoInfo () {
            mIocp = NULL;
            memset(&ol, 0x00, sizeof(ol));
            mhDir = NULL;
            memset(&mNotify, 0x00, sizeof(FILE_NOTIFY_INFORMATION));
            mIndex = 0;
            pfnFileNotify = NULL;
        }
    };
public:
    static CWinFileNotify *GetInst();

    void InitNotify();
    HFileNotify Register(const std::string &filePath, unsigned int mask, pfnWinFileNotify pfn, bool withSub = true);
    void UnRegister(HFileNotify h);

private:
    CWinFileNotify();
    virtual ~CWinFileNotify();
    virtual void run();
    void PostRequest(IoInfo *info) const; 
    void OnFileEvent(const std::string &filePath, DWORD action, const IoInfo *info);
    bool IsPathInCache(const std::string &filePath) const;
    void Close(const IoInfo *info) const;
    void CloseAll();

private:
    std::list<IoInfo *> mIoSet;
    HANDLE mIocp;
    CThread mThread;
    bool mInit;
};