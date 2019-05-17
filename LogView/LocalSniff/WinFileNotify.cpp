#include "WinFileNoitfy.h"
#include <LogLib/StrUtil.h>

using namespace std;

const DWORD gsDefaultCompleteKey = 112;

CWinFileNotify *CWinFileNotify::GetInst() {
    static CWinFileNotify *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CWinFileNotify();
    }
    return sPtr;
}

CWinFileNotify::CWinFileNotify() {
    mIocp = NULL;
}

CWinFileNotify::~CWinFileNotify() {
}

void CWinFileNotify::OnFileEvent(const string &filePath, DWORD action, const IoInfo *info) {
    info->pfnFileNotify(filePath.c_str(), action);
}

bool CWinFileNotify::IsPathInCache(const std::string &filePath) const {
    for (list<IoInfo *>::const_iterator it = mIoSet.begin() ; it != mIoSet.end() ; it++)
    {
        IoInfo *ptr = *it;
        mstring str1 = filePath;
        str1.makelower();
        mstring str2 = ptr->mDirPath;
        str2.makelower();

        if (mstring::npos != str2.find(str1))
        {
            return true;
        }
    }
    return false;
}

void CWinFileNotify::run() {
    while (true) {
        DWORD dwBytesTransferred = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED lpOl = NULL;
        BOOL result = GetQueuedCompletionStatus(mIocp, &dwBytesTransferred, &key, &lpOl, 1023);

        if (lpOl && key == gsDefaultCompleteKey)
        {
            IoInfo *pIoData = CONTAINING_RECORD(lpOl, IoInfo, ol);

            if (TRUE == result)
            {
                PFILE_NOTIFY_INFORMATION pFni = &pIoData->mNotify;

                while (true) {
                    OnFileEvent(pIoData->mDirPath + "\\" + WtoA(wstring(pFni->FileName, pFni->FileNameLength / sizeof(WCHAR))), pFni->Action, pIoData);

                    if (pFni->NextEntryOffset)
                    {
                        (DWORD_PTR &)pFni = (DWORD_PTR)pFni + pFni->NextEntryOffset;
                    } else {
                        break;
                    }
                }
            }
            PostRequest(pIoData);
        }
    }
    CloseHandle(mIocp);
}

void CWinFileNotify::InitNotify() {
    mIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    mThread.StartThread(this, false);
}

void CWinFileNotify::PostRequest(IoInfo *info) const {
    const DWORD sDirChangeMask = 
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_LAST_WRITE |
        FILE_NOTIFY_CHANGE_CREATION;

    ReadDirectoryChangesW(
        info->mhDir,
        info->mBuffer,
        sizeof(info->mBuffer),
        TRUE,
        sDirChangeMask,
        NULL,
        &info->ol,
        NULL
        );
}

HFileNotify CWinFileNotify::Register(const std::string &filePath, unsigned int mask, pfnWinFileNotify pfn, bool withSub) {
    static DWORD sMagic = 0xff11;

    if (IsPathInCache(filePath))
    {
        return INVALID_HFileNotify;
    }
    IoInfo *newInfo = new IoInfo();
    newInfo->mIndex = sMagic++;
    newInfo->mDirPath = filePath;
    newInfo->pfnFileNotify = pfn;
    newInfo->mhDir = CreateFileA(
        newInfo->mDirPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
        );

    if (INVALID_HANDLE_VALUE != newInfo->mhDir)
    {
        newInfo->mIocp = CreateIoCompletionPort(newInfo->mhDir, mIocp, gsDefaultCompleteKey, 0);
        PostRequest(newInfo);
        mIoSet.push_back(newInfo);
        return newInfo->mIndex;
    } else {
        delete newInfo;
        return INVALID_HFileNotify;
    }
}

void CWinFileNotify::Close(const IoInfo *info) const {
    if (info->mIocp)
    {
        CloseHandle(info->mIocp);
    }

    if (info->mhDir)
    {
        CloseHandle(info->mhDir);
    }
}

void CWinFileNotify::CloseAll() {
    AutoLocker locker(this);
    for (list<IoInfo *>::const_iterator it = mIoSet.begin() ; it != mIoSet.end() ; it++)
    {
        Close(*it);
    }
    mIoSet.clear();
}

void CWinFileNotify::UnRegister(HFileNotify h) {
    AutoLocker locker(this);
    for (list<IoInfo *>::iterator it = mIoSet.begin() ; it != mIoSet.end() ; it++)
    {
        IoInfo *ptr = *it;

        if (ptr->mIndex == h)
        {
            Close(ptr);
            mIoSet.erase(it);
            return;
        }
    }
}