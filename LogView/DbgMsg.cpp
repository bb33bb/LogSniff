#include <Windows.h>
#include <LogLib/LogUtil.h>
#include "DbgMsg.h"

CDbgCapturer *CDbgCapturer::GetInst() {
    static CDbgCapturer *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CDbgCapturer();
    }
    return sPtr;
}

bool CDbgCapturer::InitCapturer() {
    if (mInit)
    {
        return true;
    }

    //Global是全局调试，否则是当前session调试
    mAckEvent = CreateEventA(NULL, FALSE, FALSE, "Global\\DBWIN_BUFFER_READY");
    mReadyEvent = CreateEventA(NULL, FALSE, FALSE, "Global\\DBWIN_DATA_READY");
    mBuffMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "Global\\DBWIN_BUFFER");

    if (NULL == mAckEvent || NULL == mReadyEvent || NULL == mBuffMapping)
    {
        return false;
    }
    mInit = true;
    mDbgThread = CreateThread(NULL, 0, DbgThread, this, 0, NULL);
    return true;
}

CDbgCapturer::CDbgCapturer() {
    mBuffMapping = NULL, mAckEvent = NULL;
    mReadyEvent = NULL, mDbgThread = NULL;
    mInit = false;
}

CDbgCapturer::~CDbgCapturer() {
}

DWORD CDbgCapturer::DbgThread(LPVOID param) {
    DbgBuffer *dbgView = (DbgBuffer *)MapViewOfFile(GetInst()->mBuffMapping, FILE_MAP_READ, 0, 0, 0);

    while (true) {
        SetEvent(GetInst()->mAckEvent);

        WaitForSingleObject(GetInst()->mReadyEvent, INFINITE);

        if (dbgView->mPid != 4908)
        {
            dp("pid:%d msg:%hs", dbgView->mPid, dbgView->mBuffer);
        }
    }
    return 0;
}