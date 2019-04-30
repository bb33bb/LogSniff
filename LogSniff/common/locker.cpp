#include "Util.h"
#include "locker.h"

AutoLocker::AutoLocker(LockerBase *locker)
    : mLocker(locker)
{
    if (mLocker)
    {
        mLocker->lock();
    }
}
AutoLocker::~AutoLocker()
{
    if (mLocker)
    {
        mLocker->unlock();
    }
}

RLocker::RLocker()
{
#ifdef __linux__
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mMutex, &attr);
    pthread_mutexattr_destroy(&attr);
#else
    InitializeCriticalSection(&mMutex);
#endif
}

RLocker::~RLocker()
{
#ifdef __linux__
    pthread_mutex_destroy(&mMutex);
#else
    DeleteCriticalSection(&mMutex);
#endif
}

void RLocker::lock()
{
#ifdef __linux__
    pthread_mutex_lock(&mMutex);
#else
    EnterCriticalSection(&mMutex);
#endif
}

void RLocker::trylock()
{
#ifdef __linux__
    pthread_mutex_trylock(&mMutex);
#else
    TryEnterCriticalSection(&mMutex);
#endif
}

void RLocker::unlock()
{
#ifdef __linux__
    pthread_mutex_unlock(&mMutex);
#else
    LeaveCriticalSection(&mMutex);
#endif
}
