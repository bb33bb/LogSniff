#pragma once
#include <Windows.h>
#include <vector>

class CDbgCapturer {
public:
    static CDbgCapturer *GetInst();
    bool InitCapturer();

private:
    CDbgCapturer();
    ~CDbgCapturer();

    void OnDbgMsg(DWORD pid, const std::mstring &content);
    static DWORD WINAPI DbgThread(LPVOID param);

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
};