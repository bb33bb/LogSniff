#include "SyntaxCache.h"
#include <assert.h>
#include <LogLib/LogUtil.h>
#include <Shlwapi.h>

using namespace std;

#define TIMER_CACHE     (WM_USER + 7001)
map<HWND, CSyntaxCache *> CSyntaxCache::msTimerCache;

CSyntaxCache::CSyntaxCache() {
    mInterval = 0;
    mWorkMode = 0;
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

    //INDIC_ROUNDBOX
    SendMsg(SCI_INDICSETSTYLE, NOTE_KEYWORD, INDIC_ROUNDBOX);
    SendMsg(SCI_INDICSETALPHA, NOTE_KEYWORD, 100);
    SendMsg(SCI_INDICSETFORE, NOTE_KEYWORD, RGB(0, 0xff, 0));

    SendMsg(SCI_INDICSETSTYLE, NOTE_SELECT, INDIC_ROUNDBOX);
    SendMsg(SCI_INDICSETALPHA, NOTE_SELECT, 100);
    SendMsg(SCI_INDICSETFORE, NOTE_SELECT, RGB(0, 0, 0xff));
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

        AutoLocker locker(ptr);
        if (ptr->mCache.empty())
        {
            return;
        }

        ptr->AppendText(ptr->mLabel, ptr->mCache);
        ptr->mCache.clear();
    }
}

void CSyntaxCache::SwitchWorkMode(int mode) {
    if (mWorkMode == mode)
    {
        return;
    }

    mWorkMode = mode;
    mKeyword.clear();
    mShowData = mContent;
    SetText(mLabel, mShowData);
}

mstring CSyntaxCache::GetFilterStr(const mstring &content, const mstring &key) const {
    size_t pos1 = 0, pos2 = 0;
    size_t pos3 = 0, pos4 = 0;

    if (key.empty())
    {
        return content;
    }

    mstring result;
    while (true) {
        pos1 = content.find_in_rangei(key, pos2);

        if (mstring::npos == pos1)
        {
            break;
        }

        pos3 = content.rfind("\n", pos1);
        if (mstring::npos == pos3)
        {
            pos3 = 0;
        } else {
            pos3++;
        }

        pos4 = content.find("\n", pos1);
        if (mstring::npos == pos4)
        {
            pos4 = content.size() - 1;
        }

        result += content.substr(pos3, pos4 - pos3 + 1);
        pos2 = pos4 + 1;
    }
    return result;
}

void CSyntaxCache::OnFilter() {
    //清理缓存,防止数据重复录入
    mCache.clear();
    if (mKeyword.empty())
    {
        mShowData = mContent;
        SetText(mLabel, mContent);
    } else {
        mShowData = GetFilterStr(mContent, mKeyword);
        SetText(mLabel, mShowData);
    }
    OnViewUpdate();
}

bool CSyntaxCache::SetKeyword(const std::mstring &keyWord) {
    AutoLocker locker(this);
    if (0 == mWorkMode)
    {
        if (keyWord == mKeyword)
        {
            return true;
        }

        mKeyword = keyWord;
        OnFilter();
        return true;
    } else if (1 == mWorkMode)
    {
        mKeyword = keyWord;
        OnViewUpdate();
        if (JmpLastPos(mKeyword))
        {
            return true;
        }
    }

    return false;
}

mstring CSyntaxCache::GetKeyword() {
    return mKeyword;
}

bool CSyntaxCache::JmpNextPos(const mstring &str) {
    return false;
}

bool CSyntaxCache::JmpFrontPos(const mstring &str) {
    return false;
}

bool CSyntaxCache::JmpFirstPos(const mstring &str) {
    return false;
}

bool CSyntaxCache::JmpLastPos(const mstring &str) {
    if (str.empty() || mShowData.empty())
    {
        return false;
    }

    size_t lastPos = 0;
    for (size_t i = mShowData.size() - 1 ; i != 0 ; i--) {
        if (0 == mShowData.comparei(str, i))
        {
            lastPos = i;
            break;
        }
    }

    if (lastPos)
    {
        size_t line = SendMsg(SCI_LINEFROMPOSITION, lastPos, 0);

        if (line >= 10)
        {
            SendMsg(SCI_LINESCROLL, 0, line - 10);
        } else {
            SendMsg(SCI_LINESCROLL, 0, line);
        }
        SendMsg(SCI_SETSEL, lastPos, lastPos + str.size());
        return true;
    }
    return false;
}

void CSyntaxCache::UpdateView() const {
    SendMsg(SCI_COLOURISE, 0, -1);
}

void CSyntaxCache::OnViewUpdate() const {
    int firstLine = SendMsg(SCI_GETFIRSTVISIBLELINE, 0, 0);
    int lineCount = SendMsg(SCI_LINESONSCREEN, 0, 0);
    int lastLine = firstLine + lineCount;
    int curLine = firstLine;

    int startPos = SendMsg(SCI_POSITIONFROMLINE, firstLine, 0);
    int lastPos = SendMsg(SCI_GETLINEENDPOSITION, lastLine, 0);

    if (lastPos <= startPos)
    {
        return;
    }

    mstring str = mShowData.substr(startPos, lastPos - startPos);

    if (mKeyword.empty())
    {
        return;
    }

    SendMsg(SCI_SETINDICATORCURRENT, NOTE_KEYWORD, 0);
    SendMsg(SCI_INDICATORCLEARRANGE, 0, mShowData.size());

    size_t pos1 = 0;
    size_t pos2 = 0;
    while (mstring::npos != (pos1 = str.find_in_rangei(mKeyword, pos2))) {
        SendMsg(SCI_INDICATORFILLRANGE, pos1 + startPos, mKeyword.size());

        pos2 = pos1 + mKeyword.size();
    }
}

mstring CSyntaxCache::GetViewStr(int startPos, int length) const {
    return mShowData.substr(startPos, length);
}

void CSyntaxCache::PushToCache(const std::mstring &content) {
    AutoLocker locker(this);

    if (content.empty())
    {
        return;
    }

    bool flag = false;
    mstring str = content;
    if (0 == mWorkMode)
    {
        if (mKeyword.empty())
        {
            flag = true;
        } else {
            str = GetFilterStr(content, mKeyword);

            if (!str.empty())
            {
                flag = true;
            }
        }
    } else {
        flag = true;
    }

    if (flag)
    {
        mCache += str;
        mShowData += str;
    }

    mContent += str;
    if (!str.empty() && str[str.size() - 1] != '\n')
    {
        mContent += "\n";

        if (flag)
        {
            mCache += "\n";
            mShowData += "\n";
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