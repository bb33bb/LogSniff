#pragma once
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/inotify.h>

#include <string>
#include <list>
#include <map>
#include <set>
#include "common/locker.h"
#include "common/thread.h"

#define FD_NOTIFY_MODIFIED      (1 << 0)
#define FD_NOTIFY_CREATE        (1 << 2)
#define FD_NOTIFY_DELETE        (1 << 3)
#define FD_NOTIFY_ALL           (-1)

typedef unsigned int HFileNotify;
typedef void (* pfnFileNotify)(const char *filePath, unsigned int mask);

struct FileNotifyRegister {
    HFileNotify mIndex;
    pfnFileNotify mNotify;
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

class CFileNotify : public ThreadRunable, public RLocker {
public:
    static CFileNotify *GetInst();

    void InitNotify();
    HFileNotify Register(const std::string &filePath, unsigned int mask, pfnFileNotify pfn, bool withSub = true);
    void UnResister(HFileNotify h);

private:
    CFileNotify();
    virtual ~CFileNotify();
    virtual void run();
    void OnFileEvent(const inotify_event *notifyEvent);
    void MonitorPath(const char *filePath, FileNotifyRegister *ptr);
    static bool FileEnumProc(bool isDir, const char *filePath, void *param);

private:
    CThread mThread;
    unsigned int mSerial;
    std::list<FileNotifyRegister> mRegisterCache;
    std::map <std::string, int> mWdGlobal1;
    std::map <int, std::string> mWdGlobal2;
    unsigned int mFd;   //linux fd
};