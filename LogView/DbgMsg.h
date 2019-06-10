#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <LogLib/locker.h>

class CDbgCapturer {
public:
    static CDbgCapturer *GetInst();
    bool InitCapturer();

private:
    CDbgCapturer();
    ~CDbgCapturer();

    std::mstring GetProcName(DWORD pid);
    void OnDbgMsg(DWORD pid, const std::mstring &content);

    //二级缓存线程,由于共享内存中数据频繁读写,通过二级缓存减少读写竞争情况
    struct DbgMsgNode;
    size_t GetStrSetFromCache(std::list<DbgMsgNode> &set1);
    bool PushStrToCache(DWORD pid, const char *ptr, size_t maxSize);
    static DWORD WINAPI CacheThread(LPVOID param);

    static DWORD WINAPI DbgThread(LPVOID param);
    static BOOL WINAPI ProcHandler(PPROCESSENTRY32 info, void *param);

    struct DbgBuffer {
        DWORD mPid;
        char mBuffer[4096 - sizeof(DWORD)];
    };

    struct DbgMsgCache {
        HANDLE mAckEvent;
        HANDLE mReadyEvent;
        HANDLE mBuffMapping;
        LPVOID mMappingView;

        DbgMsgCache () {
            ZeroMemory(this, sizeof(DbgMsgCache));
        }
    };
private:
    bool mInit;
    std::vector<DbgMsgCache> mDbgCache;
    HANDLE mDbgThread;

    struct BuffNodeDesc {
        DWORD mPid;
        int mStartPos;
        int mLength;

        BuffNodeDesc() {
            mStartPos = 0;
            mLength = 0;
        }
    };

    struct DbgMsgNode {
        DWORD mPid;
        std::mstring mContent;
    };

    HANDLE mCacheInitSucc;
    HANDLE mBuffNotify;
    RLocker mBuffLocker;
    char *mBuffer;
    int mBuffSize;
    int mCurPos;
    std::list<BuffNodeDesc> mBuffDesc;
};