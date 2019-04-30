#pragma once

#ifdef __linux__
#include <pthread.h>
#else
#include <Windows.h>
#endif

class CNotifyEvent {
public:
    CNotifyEvent();
    virtual ~CNotifyEvent();

    void SetEvent();
    void ResetEvent();
    void Wait(unsigned int iTimeOut = -1);
private:
    void init();
private:
#ifdef __linux__
    pthread_mutex_t mMutex;
    pthread_cond_t mEvent;
#else
    HANDLE mEvent;
#endif
};