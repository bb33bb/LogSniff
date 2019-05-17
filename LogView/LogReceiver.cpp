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

CLogReceiver::CLogReceiver() {
    mInit = false;
}

CLogReceiver::~CLogReceiver() {
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

    mLogSet.push_back(cache);
    if (mFltStr.empty() || string::npos != cache->mContent.find(mFltStr))
    {
        mShowSet.push_back(cache);
        PushLogContent(cache);
    }
}