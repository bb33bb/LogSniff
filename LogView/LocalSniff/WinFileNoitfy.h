#pragma once
#include <Windows.h>
#include <string>
#include <set>
#include <map>
#include <list>
#include <LogLib/thread.h>
#include <LogLib/locker.h>
#include <LogLib/mstring.h>

#define FD_NOTIFY_MODIFIED      (1 << 0)
#define FD_NOTIFY_CREATE        (1 << 2)
#define FD_NOTIFY_DELETE        (1 << 3)
#define FD_NOTIFY_ALL           (-1)

typedef unsigned int HFileNotify;
#define INVALID_HFileNotify -1
typedef void (* pfnWinFileNotify)(const char *filePath, unsigned int mask);

struct FileNotifyRegister {
    HFileNotify mIndex;
    pfnWinFileNotify mNotify;
    unsigned int mask;
    std::string mMonitorPath;
    bool mWithSubDir;
    std::set<int> mWdSet;

    FileNotifyRegister() {
        mIndex = 0;
        mNotify = NULL;
        mask = 0;
        mWithSubDir = false;
    }
};

class CWinFileNotify : public RLocker {
    struct IoInfo {
        HANDLE mIocp;
        std::string mDirPath;
        OVERLAPPED ol;

        union {
            FILE_NOTIFY_INFORMATION mNotify;
            char mBuffer[4096];
        };
        
        HANDLE mhDir;
        DWORD mIndex;
        pfnWinFileNotify pfnFileNotify;

        IoInfo () {
            mIocp = NULL;
            memset(&ol, 0x00, sizeof(ol));
            mhDir = NULL;
            memset(&mNotify, 0x00, sizeof(FILE_NOTIFY_INFORMATION));
            mIndex = 0;
            pfnFileNotify = NULL;
        }
    };

    struct FileCacheData {
        std::mstring mFileUnique;
        std::mstring mFilePath;
        FILETIME mCreateTime;
        FILETIME mLastWriteTime;
        ULONGLONG mLastSize;
        bool mNewFile;

        FileCacheData() {
            mLastSize = 0;
            memset(&mCreateTime, 0x00, sizeof(FILETIME));
            memset(&mLastWriteTime, 0x00, sizeof(FILETIME));
            mNewFile = false;
        }
    };
public:
    static CWinFileNotify *GetInst();

    void InitNotify();
    HFileNotify Register(const std::string &filePath, const std::string &ext, unsigned int mask, pfnWinFileNotify pfn, bool withSub = true);
    void UnRegister(HFileNotify h);

private:
    CWinFileNotify();
    virtual ~CWinFileNotify();
    void PostRequest(IoInfo *info) const; 
    void OnFileEvent(const std::string &filePath, DWORD action, const IoInfo *info);
    bool IsPathInCache(const std::string &filePath) const;
    void Close(const IoInfo *info) const;
    void CloseAll();
    std::mstring GetFilePathUnique(const std::string &filePath) const;
    void OnFileNotify(const std::string &filePath, bool newFile = false);
    FileCacheData *GetFileCacheData(const std::string &filePath) const;
    bool IsUniqueInCache(const std::mstring &unique);
    void SetActive(const std::mstring &unique);
    void SetPassive(const std::mstring &unique);
    void CheckFileChanged(std::map<std::mstring, FileCacheData *> &set1, bool activeMode);
    bool IsFileActive(const FileCacheData *cache) const;

    void LoadLogFiles(const std::string &dir);
    static bool LogEnumHandler(bool isDir, LPCSTR filePath, void *param);
    static DWORD WINAPI LogCreateNotifyThread(LPVOID param);
    static DWORD WINAPI LogChangeNotifyThread(LPVOID param);
private:
    std::list<IoInfo *> mIoSet;
    HANDLE mIocp;
    bool mInit;
    static const DWORD msActiveTimeCount = (1000 * 60 * 60);
    //活跃文件缓存
    std::map<std::mstring, FileCacheData *> mActiveFileSet;
    //非活跃文件缓存
    std::map<std::mstring, FileCacheData *> mPassiveFileSet;
};