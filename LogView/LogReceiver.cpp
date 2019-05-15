#include <WinSock2.h>
#include "LogReceiver.h"
#include <LogLib/LogProtocol.h>
#include <LogLib/StrUtil.h>
#include "MainView.h"

using namespace std;

CLogReceiver *CLogReceiver::GetInst() {
    static CLogReceiver *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLogReceiver();
    }
    return sPtr;
}

bool CLogReceiver::ConnectServ(const string &ip) {
    if (mInit)
    {
        return false;
    }

    return mTcpClient.InitClient(ip, LOG_PORT, this, 3000);
}

CLogReceiver::CLogReceiver() {
    mInit = false;
}

CLogReceiver::~CLogReceiver() {
}

void CLogReceiver::OnClientConnect(CTcpClient &client) {
    LpViewRegisger abc;
    abc.mVersion = LOGVIEW_VERSION;

    string packet;
    client.Send(CLogProtocol::GetInst()->EncodeRegister(abc, packet));
}

void CLogReceiver::OnClientRecvData(CTcpClient &client, const string &strRecved, string &strResp) {
    mLogCache += strRecved;

    list<LpResult> result;
    CLogProtocol::GetInst()->GetRecvResult(mLogCache, result);

    if (result.empty())
    {
        return;
    }

    for (list<LpResult>::const_iterator it = result.begin() ; it != result.end() ; it++)
    {
        OnSingleResult(*it);
    }
}

void CLogReceiver::OnSingleResult(const LpResult &result) {
    switch (result.mCommand) {
        case  em_cmd_log:
            {
                LpLogInfo logInfo;
                CLogProtocol::GetInst()->DecodeLog(result.mContent, logInfo);

                LogInfoCache *cache = new LogInfoCache();
                cache->mContent = logInfo.mContent;

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
                cache->mFilePath = logInfo.mFilePath;

                mLogSet.push_back(cache);
                if (mFltStr.empty() || string::npos != logInfo.mContent.find(mFltStr))
                {
                    mShowSet.push_back(cache);
                    PushLogContent(cache);
                }
            }
            break;
        default:
            break;
    }
}

void CLogReceiver::OnClientSocketErr(CTcpClient &client) {
}