#include <WinSock2.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <map>
#include <LogLib/LogUtil.h>
#include <LogLib/StrUtil.h>
#include "DbgMsg.h"
#include "MainView.h"

using namespace std;

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

    DbgMsgCache cache;
    //Global是全局调试，否则是当前session调试
    cache.mAckEvent = CreateEventA(NULL, FALSE, FALSE, "Global\\DBWIN_BUFFER_READY");
    cache.mReadyEvent = CreateEventA(NULL, FALSE, FALSE, "Global\\DBWIN_DATA_READY");
    cache.mBuffMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "Global\\DBWIN_BUFFER");
    cache.mMappingView = (DbgBuffer *)MapViewOfFile(cache.mBuffMapping, FILE_MAP_READ, 0, 0, 0);
    mDbgCache.push_back(cache);

    cache.mAckEvent = CreateEventA(NULL, FALSE, FALSE, "DBWIN_BUFFER_READY");
    cache.mReadyEvent = CreateEventA(NULL, FALSE, FALSE, "DBWIN_DATA_READY");
    cache.mBuffMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "DBWIN_BUFFER");
    cache.mMappingView = (DbgBuffer *)MapViewOfFile(cache.mBuffMapping, FILE_MAP_READ, 0, 0, 0);
    mDbgCache.push_back(cache);

    mInit = true;
    mDbgThread = CreateThread(NULL, 0, DbgThread, this, 0, NULL);
    return true;
}

CDbgCapturer::CDbgCapturer() {
    mInit = false;
}

CDbgCapturer::~CDbgCapturer() {
}

struct ProcEnumInfo {
    DWORD mPid;
    mstring mProcName;
};

BOOL CDbgCapturer::ProcHandler(PPROCESSENTRY32 info, void *param) {
    ProcEnumInfo *ptr = (ProcEnumInfo *)param;

    if (ptr->mPid == info->th32ProcessID)
    {
        ptr->mProcName = WtoA(info->szExeFile);
        return FALSE;
    }
    return TRUE;
}

mstring CDbgCapturer::GetProcName(DWORD pid) {
    static map<DWORD, mstring> sProcSet;

    if (pid == 4 || pid == 0)
    {
        return "系统进程";
    }

    mstring result;
    bool findProc = false;
    map<DWORD, mstring>::const_iterator it = sProcSet.find(pid);
    if (sProcSet.end() == it)
    {
        findProc = true;
    } else {
        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

        if (hProc || (NULL == hProc && 5 == GetLastError()))
        {
            result = it->second;
        } else {
            sProcSet.erase(pid);
            findProc = true;
        }
    }

    if (findProc)
    {
        ProcEnumInfo info;
        info.mPid = pid;
        ProcIterateProc(ProcHandler, &info);

        result = info.mProcName;
    }
    return result;
}

void CDbgCapturer::OnDbgMsg(DWORD pid, const mstring &content) {
    mstring procName = GetProcName(pid);

    mstring procMsg = FormatA("%hs:%d ", procName.c_str(), pid);
    size_t pos1 = 0;
    size_t pos2 = 0;

    mstring logContent;
    while (true) {
        pos1 = content.find("\n", pos2);
        if (mstring::npos == pos1)
        {
            break;
        }

        if (pos1 > pos2)
        {
            logContent += procMsg;
            logContent += content.substr(pos2, pos1 - pos2 + 1);
        }
        pos2 = pos1 + 1;
    }

    if (pos2 < content.size())
    {
        logContent += procMsg;
        logContent += content.substr(pos2, content.size() - pos2);
        logContent += "\n";
    }
    PushDbgContent(logContent);
}

DWORD CDbgCapturer::DbgThread(LPVOID param) {
    CDbgCapturer *pThis = (CDbgCapturer *)param;
    HANDLE *arry = new HANDLE[pThis->mDbgCache.size() + 16];

    size_t size = pThis->mDbgCache.size();
    for (size_t i = 0 ; i < size ; i++)
    {
        arry[i] = pThis->mDbgCache[i].mReadyEvent;
    }

    while (true) {
        for (size_t i = 0 ; i < size ; i++)
        {
            SetEvent(pThis->mDbgCache[i].mAckEvent);
        }

        DWORD dw = WaitForMultipleObjects(size, arry, FALSE, INFINITE);

        size_t index = (dw - WAIT_OBJECT_0);
        if (index >= 0 && index < size)
        {
            list<DbgMsgCache> set1;
            set1.push_back(pThis->mDbgCache[index]);

            for (size_t j = index + 1 ; j < size ; j++)
            {
                if (WAIT_OBJECT_0 == WaitForSingleObject(arry[j], 0))
                {
                    set1.push_back(pThis->mDbgCache[j]);
                }
            }

            for (list<DbgMsgCache>::iterator it = set1.begin() ; it != set1.end() ; it++)
            {
                DbgBuffer *ptr = (DbgBuffer *)it->mMappingView;
                pThis->OnDbgMsg(ptr->mPid, ptr->mBuffer);
            }
        }
    }

    if (arry)
    {
        delete []arry;
    }
    return 0;
}