#include <WinSock2.h>
#include "LogReceiver.h"
#include <LogLib/LogProtocol.h>
#include <LogLib/StrUtil.h>
#include <LogLib/LogUtil.h>
#include "LocalSniff/LocalMonitor.h"
#include "ServSniff/ServMonitor.h"
#include "MainView.h"
#include "LogServMgr.h"

using namespace std;

CLogReceiver *CLogReceiver::GetInst() {
    static CLogReceiver *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLogReceiver();
    }
    return sPtr;
}

void CLogReceiver::InitReceiver() {
    CLocalMonitor::GetInst()->Init(this);
    CLogServMgr::GetInst()->Register(this);
}

CLogReceiver::CLogReceiver() {
    mInit = false;
    mCurMonitor = NULL;
}

CLogReceiver::~CLogReceiver() {
}

bool CLogReceiver::Run(const LogServDesc *cfg) {
    if (cfg == mCfg)
    {
        if (mCurMonitor)
        {
            mCurMonitor->Run(cfg);
        }
    } else {
        if (mCurMonitor)
        {
            mCurMonitor->Stop();
        }

        if (em_log_serv_local == cfg->mLogServType)
        {
            mCurMonitor = CLocalMonitor::GetInst();
        } else {
            mCurMonitor = new CServMonitor();
        }

        mCfg = cfg;
        mCurMonitor->Run(mCfg);
    }
    return true;
}

bool CLogReceiver::IsRunning() {
    if (!mCurMonitor)
    {
        return false;
    }

    return mCurMonitor->IsRunning();
}

void CLogReceiver::Stop() {
    if (mCurMonitor)
    {
        mCurMonitor->Stop();

        if (mCfg->mLogServType == em_log_serv_remote)
        {
            delete mCurMonitor;
        }
        mCurMonitor = NULL;
    }
}

void CLogReceiver::PushLog(const mstring &filePath, const mstring &content) {
    LogInfoCache *cache = new LogInfoCache();
    cache->mContent = content;

    SYSTEMTIME time = {0};
    GetLocalTime(&time);
    cache->mTime = FormatA(
        "%04d-%02d-%02d %02d:%02d:%02d %03d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds
        );
    cache->mFilePath = filePath;
    PushLogContent(cache);
}

bool CLogReceiver::AddPathMonitor(const std::mstring &filePath) {
    return (CLogServMgr::GetInst()->AddPath(filePath) && mCurMonitor->AddPath(filePath));
}

bool CLogReceiver::DelPathMonitor(const std::mstring &filePath) {
    return (CLogServMgr::GetInst()->DelPath(filePath) && mCurMonitor->DelPath(filePath));
}

void CLogReceiver::OnNewLogFiles(const list<mstring> &fileSet) {
}

void CLogReceiver::OnLogReceived(const mstring &filePath, const mstring &content) {
    PushLog(filePath, content);
}

void CLogReceiver::OnLogServAdd(const LogServDesc *d) {
}

void CLogReceiver::OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2) {
    Run(d2);
}

void CLogReceiver::OnLogServAlter(const LogServDesc *d) {
    Run(d);
}