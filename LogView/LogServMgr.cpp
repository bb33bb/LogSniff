#include "LogServMgr.h"
#include "GroupSender.h"

CLogServMgr::CLogServMgr() {
    mScanThread = NULL;
    mCurLogServ = NULL;
    mNotifyEvent = NULL;
    mInit = false;
}

CLogServMgr::~CLogServMgr() {
}

CLogServMgr *CLogServMgr::GetInst() {
    static CLogServMgr *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLogServMgr();
    }
    return sPtr;
}

void CLogServMgr::InitMgr() {
    if (!mInit)
    {
        mInit = true;
        mNotifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        mScanThread = CreateThread(NULL, 0, ScanThread, NULL, 0, NULL);
    }
}

HLogIndex CLogServMgr::Register(LogServEvent *listener) {
    static DWORD sMagic = 0xff11;
    DWORD h = sMagic++;
    mRegisterSet[h] = listener;

    return h;
}

void CLogServMgr::UnRegister(HLogIndex h) {
    mRegisterSet.erase(h);
}

void CLogServMgr::SetCurServ(LogServDesc *serv) {
    if (mCurLogServ != serv)
    {
        OnNotifySwitch(mCurLogServ, serv);
    }
    mCurLogServ = serv;
}

const LogServDesc *CLogServMgr::GetCurServ() {
    return mCurLogServ;
}

bool CLogServMgr::AddPath(LogServDesc *serv, const std::mstring &path) {
    if (serv->AddPath(path))
    {
        OnNotifyAlter(serv);
        return true;
    }
    return false;
}

bool CLogServMgr::DelPath(LogServDesc *serv, const std::mstring &path) {
    if (serv->DelPath(path))
    {
        OnNotifyAlter(serv);
        return true;
    }
    return false;
}

size_t CLogServMgr::GetServCount() {
    return mLogServCache.size();
}

const LogServDesc *CLogServMgr::GetServDesc(size_t index) {
    if (index >= mLogServCache.size())
    {
        return NULL;
    }

    return mLogServCache[index];
}

void CLogServMgr::OnRecvDesc(const LpServDesc &desc) {
    for (vector<LogServDesc *>::iterator it = mLogServCache.begin() ; it != mLogServCache.end() ; it++) {
        LogServDesc *ptr = *it;

        if (ptr->mUnique == desc.mUnique)
        {
            ptr->SetRemoteDesc(desc);
            //检查变化并通知变化
            OnNotifyAlter(ptr);
            return;
        }
    }
    LogServDesc *newItem = new LogServDesc();
    newItem->SetRemoteDesc(desc);
    OnNotifyAdd(newItem);
    
}

void CLogServMgr::Refush() {
    SetEvent(mNotifyEvent);
}

void CLogServMgr::OnGroupRecv(const string &recvStr, string &respStr) {
    Value content;
    Reader().parse(recvStr, content);

    if (content.type() != objectValue || content["cmd"].type() != stringValue)
    {
        return;
    }

    string cmd = content["cmd"].asString();
    if (cmd == GROUT_MSG_DESC)
    {
        LpServDesc desc;
        CLogProtocol::GetInst()->DecodeDesc(recvStr, desc);
        GetInst()->OnRecvDesc(desc);
    }
}

DWORD CLogServMgr::ScanThread(LPVOID param) {
    static CGroupSender *sSender = NULL;
    static string sScanStr;

    if (NULL == sSender)
    {
        sSender = new CGroupSender();
        sSender->Init(OnGroupRecv);
        sSender->Bind(GROUP_ADDR, GROUP_PORT);
        sScanStr = "{\"cmd\":\"scan\"}";
    }

    while (true) {
        WaitForSingleObject(GetInst()->mNotifyEvent, INFINITE);

        int count = 1;
        while (count-- > 0) {
            sSender->Send(sScanStr);

            Sleep(500);
        }
    }
    return 0;
}

void CLogServMgr::OnNotifyAdd(const LogServDesc *desc) {
    for (map<HLogIndex, LogServEvent *>::iterator it = mRegisterSet.begin() ; it != mRegisterSet.end() ; it++)
    {
        it->second->OnLogServAdd(desc);
    }
}

void CLogServMgr::OnNotifySwitch(const LogServDesc *d1, const LogServDesc *d2) {
    for (map<HLogIndex, LogServEvent *>::iterator it = mRegisterSet.begin() ; it != mRegisterSet.end() ; it++)
    {
        it->second->OnLogServSwitch(d1, d2);
    }
}

void CLogServMgr::OnNotifyAlter(const LogServDesc *desc) {
    for (map<HLogIndex, LogServEvent *>::iterator it = mRegisterSet.begin() ; it != mRegisterSet.end() ; it++)
    {
        it->second->OnLogServAlter(desc);
    }
}
