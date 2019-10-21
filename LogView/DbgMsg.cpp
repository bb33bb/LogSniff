#include <WinSock2.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <map>
#include <LogLib/LogUtil.h>
#include <LogLib/StrUtil.h>
#include "DbgMsg.h"
#include "view/MainView.h"

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
    CloseHandle(CreateThread(NULL, 0, CacheThread, this, 0, NULL));
    WaitForSingleObject(mCacheInitSucc, INFINITE);
    mDbgThread = CreateThread(NULL, 0, DbgThread, this, 0, NULL);
    return true;
}

CDbgCapturer::CDbgCapturer() {
    mCacheInitSucc = CreateEventA(NULL, TRUE, FALSE, NULL);
    mBuffNotify = CreateEventA(NULL, FALSE, FALSE, NULL);

    mBuffer = NULL;
    mBuffSize = 0;
    mCurPos = 0;
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
    if (pid == 4 || pid == 0)
    {
        return "系统进程";
    }

    //直接获取进程名
    static OSVERSIONINFOEXW sOsVerison = {0};

    if (sOsVerison.dwMajorVersion == 0)
    {
        sOsVerison.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
        GetVersionExW((OSVERSIONINFOW*)&sOsVerison);
    }

    //xp直接获取进程名,system权限进程也能获取
    if (sOsVerison.dwMajorVersion < 6)
    {
        return PathFindFileNameA(GetProcPathFromPid(pid).c_str());
    } else {
        typedef BOOL (WINAPI *pfnQueryFullProcessImageNameA)(HANDLE, DWORD, LPSTR ,PDWORD);

        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        HandleAutoClose abc(process);

        char procPath[512];
        procPath[0] = 0x00;
        DWORD bufSize = sizeof(procPath);
        pfnQueryFullProcessImageNameA pfn = (pfnQueryFullProcessImageNameA)GetProcAddress(GetModuleHandleA("kernel32.dll"), "QueryFullProcessImageNameA");
        pfn(process, 0, procPath, &bufSize);
        return PathFindFileNameA(procPath);
    }
    return "";
}

void CDbgCapturer::OnDbgMsg(DWORD pid, const mstring &content) {
    if (GetCurrentProcessId() == pid)
    {
        return;
    }

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

    for (size_t t = 0 ; t < logContent.size() ; t++)
    {
        if (logContent[t] == 0x00)
        {
            int dd = 1234;
        }
    }
    PushDbgContent(logContent);
}

size_t CDbgCapturer::GetStrSetFromCache(list<DbgMsgNode> &set1) {
    list<BuffNodeDesc> desc;
    mstring content;

    {
        AutoLocker locker(&mBuffLocker);
        desc = mBuffDesc;
        content.append(mBuffer, mCurPos);
        mCurPos = 0;
        mBuffDesc.clear();
    }

    for (list<BuffNodeDesc>::const_iterator it = desc.begin() ; it != desc.end() ; it++)
    {
        DbgMsgNode node;
        node.mPid = it->mPid;
        node.mContent = content.substr(it->mStartPos, it->mLength);
        set1.push_back(node);
    }
    return set1.size();
}

bool CDbgCapturer::PushStrToCache(DWORD pid, const char *ptr, size_t maxSize) {
    AutoLocker locker(&mBuffLocker);
    size_t startPos = mCurPos;
    bool result = false;
    size_t i = 0;
    for (i = 0 ; i < maxSize ; i++)
    {
        if (ptr[i] == 0x00)
        {
            if (i != 0)
            {
                result = true;
            }

            break;
        }

        mBuffer[mCurPos++] = ptr[i];
        if (mCurPos == mBuffSize)
        {
            mCurPos = startPos;
            result = false;
            break;
        }
    }

    if (i == maxSize)
    {
        result = true;
    }

    if (result)
    {
        BuffNodeDesc desc;
        desc.mPid = pid;
        desc.mStartPos = startPos;
        desc.mLength = i;
        mBuffDesc.push_back(desc);
        SetEvent(mBuffNotify);
    }
    return result;
}

DWORD CDbgCapturer::CacheThread(LPVOID param) {
   CDbgCapturer *pThis = (CDbgCapturer *)param;

   pThis->mBuffSize = (1024 * 1024);
   pThis->mBuffer = (char *)malloc(pThis->mBuffSize);
   list<DbgMsgNode> result;
   SetEvent(pThis->mCacheInitSucc);

   while (true) {
       WaitForSingleObject(pThis->mBuffNotify, INFINITE);

       if (0 != pThis->GetStrSetFromCache(result))
       {
           for (list<DbgMsgNode>::const_iterator it = result.begin() ; it != result.end() ; it++)
           {
               pThis->OnDbgMsg(it->mPid, it->mContent);
           }
       }
       result.clear();
   }

   if (pThis->mBuffer)
   {
       free(pThis->mBuffer);
       pThis->mBuffSize = 0;
   }
   return 0;
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
                pThis->PushStrToCache(ptr->mPid, ptr->mBuffer, 4096 - sizeof(DWORD));
            }
        }
    }

    if (arry)
    {
        delete []arry;
    }
    return 0;
}