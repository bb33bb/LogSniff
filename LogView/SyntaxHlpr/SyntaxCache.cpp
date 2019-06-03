#include "SyntaxCache.h"
#include <assert.h>
#include <LogLib/LogUtil.h>
#include <Shlwapi.h>

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

    RegisterParser(label, LogParser, this);

    msTimerCache[hwnd] = this;
    SetTimer(hwnd, TIMER_CACHE, interval, TimerCache);

    SendMsg(SCI_INDICSETSTYLE, NOTE_KEYWORD, INDIC_ROUNDBOX);
    SendMsg(SCI_INDICSETALPHA, NOTE_KEYWORD, 100);
    SendMsg(SCI_INDICSETFORE, NOTE_KEYWORD, RGB(0x63, 0xb8, 0xff));

    SendMsg(SCI_INDICSETSTYLE, NOTE_SELECT, INDIC_ROUNDBOX);
    SendMsg(SCI_INDICSETALPHA, NOTE_SELECT, 100);
    SendMsg(SCI_INDICSETFORE, NOTE_SELECT, RGB(0, 0xff, 0));
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
        if (ptr->mCache.empty())
        {
            return;
        }

        ptr->AppendText(ptr->mLabel, ptr->mCache);
        ptr->mCache.clear();
    }
}

void CSyntaxCache::SetFilter(const mstring &rule) {
    if (rule == mRuleStr)
    {
        return;
    }

    //清理缓存,防止数据重复录入
    mCache.clear();
    mRuleStr = rule;
    if (mRuleStr.empty())
    {
        mShowData = mContent;
        SetText(mLabel, mContent);
    } else {
        mShowData.clear();
        size_t pos1 = 0;
        size_t pos2 = 0;
        size_t pos3 = 0;
        size_t pos4 = 0;

        while (true) {
            pos1 = mContent.find_in_rangei(mRuleStr, pos2);

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
            mShowData += mContent.substr(pos3 + 1, pos4 - pos3);
            pos2 = pos4 + 1;
        }
        SetText(mLabel, mShowData);
    }
}

void CSyntaxCache::UpdateView() const {
    SendMsg(SCI_COLOURISE, 0, -1);
}

void CSyntaxCache::OnViewUpdate(int startPos, int length) const {
    mstring str = mShowData.substr(startPos, length);

    if (mRuleStr.empty())
    {
        return;
    }

    SendMsg(SCI_SETINDICATORCURRENT, NOTE_KEYWORD, 0);
    SendMsg(SCI_INDICATORCLEARRANGE, 0, mShowData.size());

    size_t pos1 = 0;
    size_t pos2 = 0;
    while (mstring::npos != (pos1 = str.find_in_rangei(mRuleStr, pos2))) {
        SendMsg(SCI_INDICATORFILLRANGE, pos1 + startPos, mRuleStr.size());

        pos2 = pos1 + mRuleStr.size();
    }
}

mstring CSyntaxCache::GetViewStr(int startPos, int length) const {
    return mShowData.substr(startPos, length);
}

void CSyntaxCache::PushToCache(const std::mstring &content) {
    AutoLocker locker(this);

    bool flag = false;
    if (mRuleStr.empty() || mstring::npos != content.find_in_rangei(mRuleStr.c_str()))
    {
        mCache += content;
        mShowData += content;
        flag = true;
    }

    mContent += content;
    if (content.size() && flag)
    {
        char c = mCache[mCache.size() - 1];

        if (c != '\n')
        {
            if (flag)
            {
                mCache += "\n";
                mShowData += "\n";
            }

            mContent += "\n";
        }
    }
}

void CSyntaxCache::LogParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *sc,
    void *param
    )
{
    CSyntaxCache *pThis = (CSyntaxCache *)param;

    sc->SetState(STAT_CONTENT);
    sc->ForwardBytes(length);
    return;
}