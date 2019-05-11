#pragma once
#include <string>
#include <map>
#include <set>
#include "thread.h"
#include "FileNotify.h"
#include "tcpserv.h"
#include "LogProtocol.h"
#include "tcpserv.h"

struct LogFileInfo {
    std::string mFilePath;
    long mLastPos;
    unsigned int mLastModified;
    unsigned long mFileSize;
    std::string mLastCache;

    LogFileInfo() {
        mLastPos = 0;
        mLastModified = 0;
        mFileSize = 0;
    }
};

class CLogMonitor : public ThreadRunable, public RLocker, public ServEvent {
public:
    static CLogMonitor *GetInst();
    void InitMonitor(const char *path);
    std::list<std::string> GetPathSet();

private:
    CLogMonitor();
    virtual ~CLogMonitor();
    virtual void run();

    void DispatchLog(LogFileInfo *info, long fileSize) const;
    void OnRecvComplete(unsigned int client, const LpResult &result);
    static void FileNotifyProc(const char *filePath, unsigned int mask);

    //tcp serv event
    virtual void OnServAccept(unsigned int client);
    virtual void OnServRecvData(unsigned int client, const std::string &strRecved, std::string &strResp);
    virtual void OnServSocketErr(unsigned int client);
    virtual void OnServSocketClose(unsigned int client);
private:
    HFileNotify mNotifyHandle;
    //log info and cache
    std::map<std::string, LogFileInfo *> mLogCache;

    std::set<unsigned int> mListener;
    std::map<unsigned int, std::string> mDataCache;
    std::list<std::string> mPathSet;
    CTcpServ mTcpServ;
};