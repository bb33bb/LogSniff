#include "SyntaxCache.h"
#include <assert.h>

using namespace std;

#define TIMER_CACHE     (WM_USER + 7001)
map<HWND, CSyntaxCache *> CSyntaxCache::msTimerCache;

CSyntaxCache::CSyntaxCache() {
    mInterval = 0;
    mStartPos = -1;
    mEndPos = -1;
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
        mShowData.clear();
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
            mShowData += mContent.substr(pos3 + 1, pos4 - pos3);
            pos2 = pos1 + mRule.size();
        }
        SetText(mLabel, mShowData);
    }
}

void CSyntaxCache::UpdateView() {
    SendMsg(SCI_COLOURISE, 0, -1);
}

void CSyntaxCache::InsertRuleNode(const RuleStatNode &node) {
    if (mRuleNodeSet.empty())
    {
        mRuleNodeSet.push_back(node);
    } else if (node.mStartPos <= mRuleNodeSet[0].mStartPos)
    {
        mRuleNodeSet.insert(mRuleNodeSet.begin(), node);
    }
    else if (node.mStartPos > mRuleNodeSet[mRuleNodeSet.size() - 1].mStartPos)
    {
        mRuleNodeSet.push_back(node);
    } else {
        int i = 0;
        for (vector<RuleStatNode>::const_iterator it = mRuleNodeSet.begin() ; it != mRuleNodeSet.end() ; it++, i++)
        {
            if (node.mStartPos > it->mStartPos && node.mStartPos <= mRuleNodeSet[i + 1].mStartPos)
            {
                mRuleNodeSet.insert(++it, node);
                break;
            }
        }
    }

    if (-1 == mStartPos)
    {
        mStartPos = node.mStartPos;
        mEndPos = node.mEndPos;
    } else {
        if ((int)node.mStartPos < mStartPos)
        {
            mStartPos = node.mStartPos;
        }

        if ((int)node.mEndPos > mEndPos)
        {
            mEndPos = node.mEndPos;
        }
    }
}

void CSyntaxCache::SetKeyWord(const std::mstring &keyWord, int stat) {
    mstring low(keyWord);
    low.makelower();
    if (keyWord.empty() || mKeyWordSet.end() != mKeyWordSet.find(low))
    {
        return;
    }
    mKeyWordSet[low] = stat;

    size_t pos1 = 0;
    size_t pos2 = 0;
    while (true) {
        pos1 = mShowData.find_in_rangei(low, pos2);

        if (mstring::npos == pos1)
        {
            break;
        }

        RuleStatNode node;
        node.mStartPos = pos1;
        node.mEndPos = pos1 + low.size();
        node.mStat = stat;
        node.mRule = low;
        InsertRuleNode(node);
        pos2 = pos1 + low.size();
    }
    UpdateView();
}

void CSyntaxCache::DelKeyWord(const std::mstring &keyWord) {
    mstring low(keyWord);
    low.makelower();

    for (vector<RuleStatNode>::const_iterator it = mRuleNodeSet.begin() ; it != mRuleNodeSet.end() ;)
    {
        if (it->mRule == keyWord)
        {
            it = mRuleNodeSet.erase(it);
        } else {
            it++;
        }
    }

    if (mRuleNodeSet.empty())
    {
        mStartPos = mEndPos = -1;
    } else {
        mStartPos = mRuleNodeSet[0].mStartPos;
        mEndPos = mRuleNodeSet[mRuleNodeSet.size() - 1].mEndPos;
    }
    UpdateView();
}

void CSyntaxCache::ClearKeyWord() {
    mRuleNodeSet.clear();
    mStartPos = mEndPos = -1;
    UpdateView();
}

void CSyntaxCache::PushToCache(const std::mstring &content) {
    AutoLocker locker(this);

    bool flag = false;
    if (mRule.empty() || mstring::npos != content.find_in_rangei(mRule.c_str()))
    {
        mCache += content;
        mShowData += content;
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

    if (-1 == pThis->mStartPos)
    {
        sc->SetState(STAT_CONTENT);
        sc->ForwardBytes(length);
    } else {
        unsigned int endPos = startPos + length;
        if (pThis->mStartPos <= (int)endPos && pThis->mEndPos >= (int)startPos)
        {
            size_t pos0 = startPos;
            for (vector<RuleStatNode>::const_iterator it = pThis->mRuleNodeSet.begin() ; it != pThis->mRuleNodeSet.end() ; it++)
            {
                size_t pos1 = max(it->mStartPos, pos0);
                size_t pos2 = min(it->mEndPos, endPos);

                if (pos1 < pos2)
                {
                    if (pos0 < pos1)
                    {
                        sc->SetState(STAT_CONTENT);
                        sc->ForwardBytes(pos1 - pos0);
                    }

                    sc->SetState(STAT_KEYWORD);
                    sc->ForwardBytes(pos2 - pos1);
                    pos0 = pos2;
                } else {
                }
            }

            if (pos0 < endPos)
            {
                sc->SetState(STAT_CONTENT);
                sc->ForwardBytes(endPos - pos0);
            }
        }
    }
    return;
}