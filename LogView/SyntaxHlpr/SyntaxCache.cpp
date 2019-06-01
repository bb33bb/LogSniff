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

void CSyntaxCache::SetFilter(const mstring &rule) {
    if (rule == mRule)
    {
        return;
    }

    //清理缓存,防止数据重复录入
    mCache.clear();
    mRule = rule;
    if (mRule.empty())
    {
        SetText(mLabel, mContent);
    } else {
        mstring result;
        size_t pos1 = 0;
        size_t pos2 = 0;
        size_t pos3 = 0;
        size_t pos4 = 0;

        while (true) {
            pos1 = mContent.find_in_rangei(mRule, pos2);

            if (mstring::npos == pos1)
            {
                break;
            }

            pos3 = mContent.rfind("\n", pos1);
            if (mstring::npos == pos3)
            {
                pos3 = 0;
            }

            pos4 = mContent.find("\n", pos1);
            result += mContent.substr(pos3 + 1, pos4 - pos3);
            pos2 = pos1 + mRule.size();
        }
        SetText(mLabel, result);
    }
}

void CSyntaxCache::PushToCache(const std::mstring &content) {
    AutoLocker locker(this);

    bool flag = false;
    if (mRule.empty() || mstring::npos != content.find_in_rangei(mRule.c_str()))
    {
        mCache += content;
        flag = true;
    }

    mContent += content;
    if (content.size())
    {
        char c = mCache[content.size() - 1];

        if (c != '\n')
        {
            if (flag)
            {
                mCache += "\n";
            }

            mContent += "\n";
        }
    }
}