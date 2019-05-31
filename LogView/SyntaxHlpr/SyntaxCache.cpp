#include "SyntaxCache.h"
#include <assert.h>

using namespace std;

#define TIMER_CACHE     (WM_USER + 7001)
map<HWND, CSyntaxCache *> CSyntaxCache::msTimerCache;

CSyntaxCache::CSyntaxCache() {
    mInterval = 0;
}

CSyntaxCache::~CSyntaxCache() {
}

bool CSyntaxCache::InitCache(const mstring &label, int interval) {
    mLabel = label;
    mInterval = interval;

    HWND hwnd = SyntaxView::GetWindow();
    assert(IsWindow(hwnd));

    msTimerCache[hwnd] = this;
    SetTimer(hwnd, TIMER_CACHE, interval, TimerCache);
    return true;
}

void CSyntaxCache::TimerCache(HWND hwnd, UINT msg, UINT_PTR id, DWORD time)
{
    if (TIMER_CACHE == id)
    {
        map<HWND, CSyntaxCache *>::iterator it = msTimerCache.find(hwnd);

        if (it == msTimerCache.end())
        {
            return;
        }
        CSyntaxCache *ptr = it->second;
        ptr->AppendText(ptr->mLabel, ptr->mCache);
        ptr->mCache.clear();
    }
}

void CSyntaxCache::PushToCache(const std::mstring &content) {
    AutoLocker locker(this);
    mCache += content;

    if (mCache.size())
    {
        char c = mCache[mCache.size() - 1];

        if (c != '\n')
        {
            mCache += "\n";
        }
    }
}