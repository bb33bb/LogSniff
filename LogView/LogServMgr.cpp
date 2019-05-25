#include "LogServMgr.h"
#include "GroupSender.h"
#include <LogLib/SqliteOperator.h>
#include <LogLib/StrUtil.h>

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
        InitConfig();

        mNotifyEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
        mScanThread = CreateThread(NULL, 0, ScanThread, NULL, 0, NULL);
    }
}

HLogIndex CLogServMgr::Register(LogServEvent *listener) {
    static DWORD sMagic = 0xff11;
    DWORD h = sMagic++;
    mRegisterSet[h] = listener;

    for (vector<LogServDesc *>::const_iterator it = mLogServCache.begin() ; it != mLogServCache.end() ; it++)
    {
        LogServDesc *ptr = *it;
        listener->OnLogServAdd(ptr);
    }
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
    SaveDescToDb(desc);
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
    SaveDescToDb(desc);
}

void CLogServMgr::LoadCacheFromDb() {
    SqliteOperator opt(mCfgDbPath);
    SqliteResult &result = opt.Select("select * from tSessionDesc");

    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        LogServDesc *newItem = new LogServDesc();
        newItem->mLogServType = (LogServType)atoi(it.GetValue("type").c_str());

        string pathSet;
        list<string> strSet;
        if (em_log_serv_local == newItem->mLogServType)
        {
            LocalServDesc tmp1;
            tmp1.mUnique = it.GetValue("servUnique");
            tmp1.mSystem = it.GetValue("system");
            pathSet = it.GetValue("pathSet");
            list<mstring> ss = SplitStrA(pathSet, ";");

            for (list<mstring>::const_iterator it = ss.begin() ; it != ss.end() ; it++) 
            {
                tmp1.mPathSet.push_back(*it);
            }
            newItem->SetLocalDesc(tmp1);
        } else if (em_log_serv_remote == newItem->mLogServType)
        {
            RemoteServDesc tmp2;
            tmp2.mUnique = it.GetValue("servUnique");
            tmp2.mSystem = it.GetValue("system");
            pathSet = it.GetValue("pathSet");
            list<mstring> ss = SplitStrA(pathSet, ";");

            for (list<mstring>::const_iterator it = ss.begin() ; it != ss.end() ; it++)
            {
                tmp2.mPathSet.push_back(*it);
            }
            newItem->SetRemoteDesc(tmp2);
        }
        OnNotifyAdd(newItem);
        mLogServCache.push_back(newItem);
    }
}

void CLogServMgr::InitConfig() {
    extern mstring gCfgPath;

    if (mCfgDbPath.empty())
    {
        mCfgDbPath = gCfgPath;
        mCfgDbPath += "\\config.db";
    }

    SqliteOperator opt(mCfgDbPath);
    opt.Exec("create table if not exists tSessionDesc (id INTEGER PRIMARY KEY, servUnique CHAR(32), type INTEGER, system TEXT, ipSet TEXT, pathSet TEXT, time CHAR(32))");
    SqliteResult &result = opt.Select("select count(*) from tSessionDesc");

    int count = atoi(result.begin().GetValue("count(*)").c_str());
    opt.Close();
    if (0 != count) {
        LoadCacheFromDb();
    } else {
        InitCache();
    }
}

void CLogServMgr::InitCache() {
    srand(GetTickCount());
    string unique = FormatA(
        "%02x%02x%02x%02x",
        rand() % 0xff,
        rand() % 0xff,
        rand() % 0xff,
        rand() % 0xff
        );

    LocalServDesc localServ;
    localServ.mUnique = unique;
    LogServDesc *desc = new LogServDesc();
    desc->SetLocalDesc(localServ);
    OnNotifyAdd(desc);
    mLogServCache.push_back(desc);
}

void CLogServMgr::SaveDescToDb(const LogServDesc *desc) {
    mstring ipSet;
    if (desc->mLogServType == em_log_serv_local)
    {
        ipSet = "本地服务";
    } else {
        const list<string> &ipList = desc->mRemoteServDesc.mIpSet;
        for (list<string>::const_iterator it = ipList.begin() ; it != ipList.end() ; it++)
        {
            ipSet += *it;
            ipSet += ";";
        }
    }

    mstring pathSet;
    for (list<mstring>::const_iterator ij = desc->mPathSet.begin() ; ij != desc->mPathSet.end() ; ij++)
    {
        pathSet += *ij;
        pathSet += ";";
    }

    mstring sql;
    SqliteOperator opt(mCfgDbPath);
    sql = FormatA("SELECT count(*) FROM tSessionDesc WHERE servUnique='%hs'", desc->mUnique.c_str());
    SqliteResult &result = opt.Select(sql.c_str());
    int d = atoi(result.begin().GetValue("count(*)").c_str());

    if (0 == d) {
        sql = FormatA(
            "INSERT INTO tSessionDesc (servUnique, type, system, ipSet, pathSet)VALUES('%hs', %d, '%hs', '%hs', '%hs')",
            desc->mUnique.c_str(),
            desc->mLogServType,
            desc->mSystem.c_str(),
            ipSet.c_str(),
            pathSet.c_str()
            );
    } else {
        sql = FormatA(
            "UPDATE tSessionDesc SET system='%hs', ipSet='%hs', pathSet='%hs' WHERE servUnique='%hs'",
            desc->mSystem.c_str(),
            ipSet.c_str(),
            pathSet.c_str(),
            desc->mUnique.c_str()
            );
    }
    opt.Exec(sql);
}