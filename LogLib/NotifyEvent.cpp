#include "NotifyEvent.h"

#ifdef __linux__
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#endif

CNotifyEvent::CNotifyEvent(){
    init();
}

void CNotifyEvent::init() {
#ifdef __linux__
    pthread_cond_init(&mEvent, NULL);
    pthread_mutex_init(&mMutex, NULL);
#else
    mEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
#endif
}

CNotifyEvent::~CNotifyEvent() {
#ifdef __linux__
    pthread_cond_destroy(&mEvent);
    pthread_mutex_destroy(&mMutex);
#else
    CloseHandle(mEvent);
#endif
}

void CNotifyEvent::SetEvent() {
#ifdef __linux__
    pthread_cond_signal(&mEvent);
#else
    ::SetEvent(mEvent);
#endif
}

void CNotifyEvent::Wait(unsigned int iTimeOut) {
#ifdef __linux__
    pthread_mutex_lock(&mMutex);

    if (iTimeOut == -1)
    {
        pthread_cond_wait(&mEvent, &mMutex);
    } else {
        timeval now;
        timespec outtime;
        gettimeofday(&now, NULL);
        outtime.tv_sec = now.tv_sec;
        outtime.tv_nsec = now.tv_usec * 1000 + iTimeOut * 1000 * 1000;
        pthread_cond_timedwait(&mEvent, &mMutex, &outtime);
    }
    pthread_mutex_unlock(&mMutex);
#else
    WaitForSingleObject(mEvent, iTimeOut);
#endif
}