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
    //�ȴ��߳̽������߳�ʱ,��λ�� 0:�߳������˳� 1:�̳߳�ʱ�˳� -1:����
    int WaitThread(int s);
    //���ᳫ��������
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