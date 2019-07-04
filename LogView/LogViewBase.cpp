#include "LogViewBase.h"
#include <LogLib/LogUtil.h>

using namespace std;

CLogViewBase::CLogViewBase() {
    mLastPos = 0, mTotalCount = 0;
    mScriptEngine = new CScriptEngine();
}

void CLogViewBase::InitLogBase() {
    mLastPos = 0, mTotalCount = 0;
    RegisterParser(LABEL_LOG_CONTENT, LogContentParser, this);

    SetStyle(STYLE_LOG_KEYWORD_BASE + 0, RGB(0, 0, 0), RGB(0, 0xff, 0));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 1, RGB(0, 0, 0), RGB(0xd0, 0xd0, 0xff));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 2, RGB(0, 0, 0), RGB(0xd0, 0xff, 0xd0));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 3, RGB(0, 0, 0), RGB(0xcd, 0xcd, 0x00));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 4, RGB(0, 0, 0), RGB(0xca, 0xff, 0x70));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 5, RGB(0, 0, 0), RGB(0xb0, 0xe2, 0xff));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 6, RGB(0, 0, 0), RGB(0xff, 0xd7, 0x00));

    int i = 0, j = 0;
    for (i = STYLE_LOG_KEYWORD_BASE + 7 ; i < 255 ; i++)
    {
        static DWORD sMagic = 0x12f;
        srand(GetTickCount() + sMagic++);
        BYTE r = rand() % 128 + 128;
        BYTE g = rand() % 128 + 128;
        BYTE b = rand() % 128 + 128;
        SetStyle(i, RGB(0, 0, 0), RGB(r, g, b));
    }
}

CLogViewBase::~CLogViewBase() {
}

void CLogViewBase::PushLog(const std::mstring &content) {
    if (content.empty())
    {
        return;
    }

    size_t pos1 = 0, pos2 = 0;
    while ((pos1 = content.find("/n", pos2)) != mstring::npos) {
        pos2 = pos1 + 1;
        mTotalCount++;
    }

    mContent += content;
    const mstring &showStr = mFilterResult.mContent;
    size_t lastShow = showStr.size();
    mScriptEngine->InputLog(mContent, mLastPos, mFilterResult);
    mLastPos = mContent.size();

    if (showStr.size() > lastShow)
    {
        mstring test1 = showStr.substr(lastShow, showStr.size() - lastShow);
        dp("show:%hs", test1.c_str());
        PushToCache(LABEL_LOG_CONTENT, showStr.substr(lastShow, showStr.size() - lastShow));
    }
}

void CLogViewBase::SetFilter(const std::mstring &newFilter) {
    mstring str(newFilter);
    str.trim();

    bool check = false;
    if (str != mRuleStr)
    {
        if (str.empty())
        {
            mScriptEngine->Compile("");
            check = true;
        } else {
            CScriptEngine test;

            if (test.Compile(str))
            {
                mScriptEngine->Compile(str);
                check = true;
            }
        }
    }

    if (check)
    {
        //Update Colour Cache
        mRuleStr = str;
        mFilterResult.Clear();
        mScriptEngine->InputLog(mContent, 0, mFilterResult);
        ClearCache();
        ClearView();
        SetText(LABEL_LOG_CONTENT, mFilterResult.mContent);
        mLastPos = mContent.size();
    }
    return;
}

void CLogViewBase::GetLineCount(int &show, int &total) {
    show = SendMsg(SCI_GETLINECOUNT, 0, 0);
    total = mTotalCount;
}

mstring CLogViewBase::GetRuleStr() {
    return mRuleStr;
}

void CLogViewBase::ClearLogView() {
    mContent.clear();
    mFilterResult.Clear();
    mLastPos = 0;
    mTotalCount = 0;

    ClearView();
}

void CLogViewBase::OnLogStrStyle(const char *ptr, unsigned int startPos, int length, StyleContextBase *sc) const {
    /*
    通过二分查找法定位语法位置
    */
    const vector<LogKeyword> &keyWordSet = mFilterResult.mKeywordSet;
    if (keyWordSet.empty())
    {
        sc->SetState(STYLE_CONTENT);
        sc->ForwardBytes(length);
        return;
    }

    size_t pos1 = 0, pos2 = keyWordSet.size() - 1;
    size_t startIndex = 0;
    while (true) {
        if (pos2 == pos1)
        {
            const LogKeyword &t1 = keyWordSet[pos1];
            if (startPos < t1.mKeywordEnd)
            {
                startIndex = pos1;
            } else {
                startIndex = pos1 + 1;
            }
            break;
        } else if ((pos2 - pos1) == 1)
        {
            const LogKeyword &t2 = keyWordSet[pos1];
            const LogKeyword &t3 = keyWordSet[pos2];

            if (startPos < t2.mKeywordEnd)
            {
                startIndex = pos1;
            } else if (startPos >= t2.mKeywordEnd && startPos < t3.mKeywordEnd)
            {
                startPos = pos2;
            } else {
                startPos = pos2 + 1;
            }
            break;
        }

        size_t mid = (pos2 - pos1) / 2;
        const LogKeyword &midValue = keyWordSet[mid];

        if (startPos >= midValue.mKeywordStart && startPos < midValue.mKeywordEnd)
        {
            startIndex = mid;
            break;
        } else if (startPos < midValue.mKeywordStart)
        {
            pos2 = mid;
        } else {
            pos1 = mid;
        }
    }

    size_t i = startIndex;
    size_t endPos = startPos + length;
    size_t curPos = startPos;
    while (true) {
        if (i >= keyWordSet.size())
        {
            sc->SetState(STYLE_CONTENT);
            sc->ForwardBytes(endPos - curPos);
            break;
        }

        const LogKeyword &t4 = keyWordSet[i];
        if (curPos < t4.mKeywordStart)
        {
            sc->SetState(STYLE_CONTENT);
            sc->ForwardBytes(t4.mKeywordStart - curPos);
            curPos += (t4.mKeywordStart - curPos);
        } else if (curPos >= t4.mKeywordStart)
        {
            sc->SetState(t4.mStyle);
            sc->Forward(t4.mKeyword.size());
            curPos += t4.mKeyword.size();
            i++;
        }

        if (curPos >= endPos)
        {
            break;
        }
    }
}

void CLogViewBase::LogContentParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    ((CLogViewBase *)param)->OnLogStrStyle(ptr, startPos, length, s);
    return;
}

void CLogViewBase::OnViewUpdate() const {
    SyntaxTextView::OnViewUpdate();
}