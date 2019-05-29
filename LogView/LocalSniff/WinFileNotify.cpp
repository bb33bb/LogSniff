#include "WinFileNoitfy.h"
#include <LogLib/StrUtil.h>
#include <LogLib/LogUtil.h>
#include <LogLib/crc32.h>
#include "../MonitorBase.h"

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
    mInit = false;
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

DWORD CWinFileNotify::LogCreateNotifyThread(LPVOID param) {
    CWinFileNotify *pThis = (CWinFileNotify *)param;

    while (true) {
        DWORD dwBytesTransferred = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED lpOl = NULL;
        BOOL result = GetQueuedCompletionStatus(pThis->mIocp, &dwBytesTransferred, &key, &lpOl, 1023);

        if (lpOl && key == gsDefaultCompleteKey)
        {
            IoInfo *pIoData = CONTAINING_RECORD(lpOl, IoInfo, ol);

            if (TRUE == result)
            {
                PFILE_NOTIFY_INFORMATION pFni = &pIoData->mNotify;

                dp("@@@@@@@@@@@@@@@11111111111111111");
                while (true) {
                    string filePath = pIoData->mDirPath + "\\" + WtoA(wstring(pFni->FileName, pFni->FileNameLength / sizeof(WCHAR)));

                    GetInst()->OnFileNotify(filePath, true);
                    int size = 0;
                    FILE *fp = fopen(filePath.c_str(), "rb");
                    if (fp)
                    {
                        fseek(fp, 0, SEEK_END);
                        size = ftell(fp);
                        fclose(fp);
                    }
                    dp("test:%hs, size:%d", filePath.c_str(), size);

                    if (pFni->NextEntryOffset)
                    {
                        (DWORD_PTR &)pFni = (DWORD_PTR)((const char *)pFni + pFni->NextEntryOffset);
                    } else {
                        break;
                    }
                }
                dp("@@@@@@@@@@@@@@@22222222222222222");
            }
            pThis->PostRequest(pIoData);
        }
    }
    CloseHandle(pThis->mIocp);
}

DWORD CWinFileNotify::LogChangeNotifyThread(LPVOID param) {
    CWinFileNotify *pThis = (CWinFileNotify *)param;

    while (true) {
        {
            AutoLocker locker(pThis);
            for (map<ULONG, FileCacheData *>::iterator it = pThis->mFileCache.begin() ; it != pThis->mFileCache.end() ; it++)
            {
                FileCacheData *ptr = it->second;

                FILETIME t1 = {0}, t2 = {0};
                HANDLE h = CreateFileA(ptr->mFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                if (h)
                {
                    GetFileTime(h, &t1, NULL, &t2);

                    DWORD s1 = 0;
                    DWORD s2 = GetFileSize(h, &s1);
                    ULONGLONG fileSize = (((ULONGLONG)s1 << 32) | (ULONGLONG)s2);
                    CloseHandle(h);

                    bool notify = false;
                    if (true == ptr->mNewFile)
                    {
                        notify = true;
                        ptr->mNewFile = false;
                    } else if (0 != memcmp(&t2, &ptr->mLastWriteTime, sizeof(FILETIME))) {
                        memcpy(&ptr->mLastWriteTime, &t2, sizeof(FILETIME));
                        notify = true;
                    } else if (fileSize > ptr->mLastSize)
                    {
                        ptr->mLastSize = fileSize;
                        notify = true;
                    }

                    if (notify)
                    {
                        for (list<IoInfo *>::const_iterator it = pThis->mIoSet.begin() ; it != pThis->mIoSet.end() ; it++)
                        {
                            IoInfo *pInfo = *it;
                            if (ptr->mFilePath.startwith(pInfo->mDirPath.c_str()))
                            {
                                pInfo->pfnFileNotify(ptr->mFilePath.c_str(), -1);
                            }
                        }
                    }
                }
            }
        }

        Sleep(1000);
    }
    return 0;
}

void CWinFileNotify::InitNotify() {
    if (true == mInit)
    {
        return;
    }

    mInit = true;
    mIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CloseHandle(CreateThread(NULL, 0, LogCreateNotifyThread, this, 0, NULL));
    CloseHandle(CreateThread(NULL, 0, LogChangeNotifyThread, this, 0, NULL));
}

void CWinFileNotify::PostRequest(IoInfo *info) const {
    const DWORD sDirChangeMask = 
        FILE_NOTIFY_CHANGE_FILE_NAME |
        FILE_NOTIFY_CHANGE_DIR_NAME |
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

bool CWinFileNotify::LogEnumHandler(bool isDir, LPCSTR filePath, void *param) {
    if (isDir)
    {
        return true;
    }

    if (MonitorBase::IsLogFile(filePath))
    {
        GetInst()->OnFileNotify(filePath, false);
    }
    return true;
}

void CWinFileNotify::LoadLogFiles(const std::string &filePath) {
    DWORD attr = GetFileAttributesA(filePath.c_str());

    if (INVALID_FILE_ATTRIBUTES == attr)
    {
        return;
    }

    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        EnumFiles(filePath, TRUE, LogEnumHandler, NULL);
    } else {
        if (MonitorBase::IsLogFile(filePath))
        {
            OnFileNotify(filePath);
        }
    }
}

HFileNotify CWinFileNotify::Register(const std::string &filePath, const std::string &ext, unsigned int mask, pfnWinFileNotify pfn, bool withSub) {
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
        EnumFiles(filePath, TRUE, LogEnumHandler, NULL);

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

ULONG CWinFileNotify::GetFilePathCrc32(const string &filePath) const {
    mstring str = filePath;
    str.makelower();

    return crc32(str.c_str(), str.size(), 0xff11);
}

CWinFileNotify::FileCacheData *CWinFileNotify::GetFileCacheData(const string &filePath) const {
    FileCacheData *cache = new FileCacheData();
    cache->mCRC32 = GetFilePathCrc32(filePath);
    cache->mFilePath = filePath;

    HANDLE h = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (!h)
    {
        return cache;
    }

    DWORD high = 0;
    DWORD low = GetFileSize(h, &high);
    cache->mLastSize = (((ULONGLONG)high << 32) | (ULONGLONG)low);
    GetFileTime(h, &cache->mCreateTime, NULL, &cache->mLastWriteTime);
    CloseHandle(h);
    return cache;
}

void CWinFileNotify::OnFileNotify(const std::string &filePath, bool newFile) {
    ULONG dd = GetFilePathCrc32(filePath);
    AutoLocker locker(this);

    if (mFileCache.end() == mFileCache.find(dd))
    {
        FileCacheData *cache = GetFileCacheData(filePath);
        cache->mNewFile = newFile;
        mFileCache[cache->mCRC32] = cache;
    }
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