#pragma once

#ifdef __linux__
#include <pthread.h>
#else
#include <Windows.h>
#endif //__linux__

class LockerBase {
public:
    virtual void lock() = 0;
    virtual void trylock() = 0;
    virtual void unlock() = 0;
};

class RLocker : public LockerBase
{
public:
    RLocker();
    ~RLocker();

    void lock();
    void trylock();
    void unlock();

private:

#ifdef __linux__
    pthread_mutex_t mMutex;
#else
    CRITICAL_SECTION mMutex;
#endif
};

class AutoLocker
{
public:
    AutoLocker(LockerBase *locker);
    virtual ~AutoLocker();

private:
    LockerBase *mLocker;
};
