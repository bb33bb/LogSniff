#include "thread.h"
#include <time.h>
#include "Util.h"
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#include <errno.h>
#include <signal.h>
#endif

CThread::CThread() {
    mTid = 0;
    mStart = false;
    mThread = 0;
    mRunner = 0;
    mAutoDelete = true;
}

CThread::~CThread() {
    Clear();
}

bool CThread::StartThread(ThreadRunable *runner, bool bAutoDelete) {
    mRunner = runner;
    mAutoDelete = bAutoDelete;
#ifdef __linux__
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if ((pthread_attr_setschedpolicy ( &attr, SCHED_RR ) != 0))
    {
        fprintf(stderr, "pthread_attr_setschedpolicy SCHED_RR failed.\n");
        return false;
    }

    if ((pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0))
    {
        fprintf(stderr, "pthread_attr_setdetachstate PTHREAD_CREATE_DETACHED failed.\n");
        return false;
    }
    pthread_create(&mThread, &attr, threadProc, this);
    pthread_attr_destroy(&attr);
#else
    mThread = CreateThread(
        NULL,
        0,
        threadProc,
        this,
        0,
        (DWORD *)(&mTid)
        );
#endif
    mStart = true;
    return true;
}

int CThread::WaitThread(int s) {
    if (!mStart)
    {
        return -1;
    }

#ifdef __linux__
    struct timespec st = {0};
    clock_gettime(CLOCK_REALTIME, &st);

    st.tv_sec += s;
    int res = pthread_timedjoin_np(mThread, NULL, &st);
    dp("error:%d, a:%d, b:%d, c:%d", errno, (int)st.tv_sec, (int)st.tv_nsec, res);
    if (res == ETIMEDOUT)
    {
        return 1;
    }
#else
    DWORD ret = WaitForSingleObject(mThread, s * 1000);

    if (WAIT_TIMEOUT == ret)
    {
        return 1;
    }
#endif
    Clear();
    return 0;
}

void CThread::KillThread() {
    if (!mStart)
    {
        return;
    }

#ifdef __linux__
    int ret = pthread_kill(mThread, SIGKILL);
    dp("kill result:%d", ret);
#else
    TerminateThread(mThread, 0);
#endif //__linux__
    Clear();
}

#ifdef __linux__
void *CThread::threadProc(void *param) {
    CThread *pThis = (CThread *)param;
    pThis->mTid = pthread_self();
    pThis->mRunner->run();
    pThis->Clear();

    if (pThis->mAutoDelete)
    {
        delete pThis->mRunner;
        pThis->mRunner = 0;
    }
    return 0;
}
#else
DWORD CThread::threadProc(void *param) {
    CThread *pThis = (CThread *)param;
    pThis->mRunner->run();
    pThis->Clear();

    if (pThis->mAutoDelete)
    {
        delete pThis->mRunner;
        pThis->mRunner = 0;
    }
    return 0;
}
#endif

void CThread::Clear() {
    if (!mStart)
    {
        return;
    }

#ifdef __linux__
#else
    CloseHandle(mThread);
#endif
    mThread = 0;
    mStart = false;
}