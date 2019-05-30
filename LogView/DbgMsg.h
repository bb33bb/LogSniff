#pragma once

class CDbgCapturer {
public:
    static CDbgCapturer *GetInst();
    bool InitCapturer();

private:
    CDbgCapturer();
    ~CDbgCapturer();

    static DWORD WINAPI DbgThread(LPVOID param);

    struct DbgBuffer {
        DWORD mPid;
        char mBuffer[4096 - sizeof(DWORD)];
    };
private:
    bool mInit;
    HANDLE mBuffMapping;
    HANDLE mAckEvent;
    HANDLE mReadyEvent;
    HANDLE mDbgThread;
};