#pragma once
#include <vector>
#include <map>
#include <LogLib/mstring.h>
#include "MonitorBase.h"

typedef DWORD HLogIndex;

class LogServEvent {
public:
    virtual void OnLogServAdd(const LogServDesc *d) {}
    virtual void OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2) {}
    virtual void OnLogServAlter(const LogServDesc *d) {}
};

class CLogServMgr {
public:
    static CLogServMgr *GetInst();
    void InitMgr();
    HLogIndex Register(LogServEvent *listener);
    void UnRegister(HLogIndex h);
    void SetCurServ(LogServDesc *serv);
    const LogServDesc *GetCurServ();
    bool AddPath(const std::mstring &path);
    bool DelPath(const std::mstring &path);

    bool AddSpecServPath(LogServDesc *serv, const std::mstring &path);
    bool DelSpecServPath(LogServDesc *serv, const std::mstring &path);

    size_t GetServCount();
    const LogServDesc *GetServDesc(size_t index);
    void Refush();

private:
    CLogServMgr();
    virtual ~CLogServMgr();

    void OnRecvDesc(const LpServDesc &desc);
    static void OnGroupRecv(const std::string &recvStr, std::string &respStr);
    static DWORD WINAPI ScanThread(LPVOID param);
    void OnNotifyAdd(const LogServDesc *desc);
    void OnNotifySwitch(const LogServDesc *d1, const LogServDesc *d2);
    void OnNotifyAlter(const LogServDesc *desc);
    void InitConfig();
    void InitCache();
    void LoadCacheFromDb();
    void SaveDescToDb(const LogServDesc *desc);

private:
    bool mInit;
    std::vector<LogServDesc *> mLogServCache;
    std::map<HLogIndex, LogServEvent *> mRegisterSet;
    LogServDesc *mCurLogServ;
    HANDLE mNotifyEvent;
    HANDLE mScanThread;
    std::mstring mCfgDbPath;
};