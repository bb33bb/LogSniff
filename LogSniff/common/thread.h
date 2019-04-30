#pragma once
#ifdef __linux__
#include <pthread.h>
#else
#include <Windows.h>
#endif

class ThreadRunable {
public:
    virtual ~ThreadRunable() {}
    virtual void run() = 0;
};

class CThread {
public:
    CThread();
    virtual ~CThread();

    bool StartThread(ThreadRunable *runner, bool autoDelete = true);
    //等待线程结束或者超时,单位秒 0:线程正常退出 1:线程超时退出 -1:错误
    int WaitThread(int s);
    //不提倡暴力结束
    void KillThread();

    void SuspendThread() {}
    void ResumeThread() {}

private:
    void Clear();
#ifdef __linux__
    static void *threadProc(void *param);
    pthread_t mThread;
#else
    static DWORD __stdcall threadProc(void *param);
    HANDLE mThread;
#endif

private:
    unsigned int mTid;
    bool mStart;
    ThreadRunable *mRunner;
    bool mAutoDelete;
};