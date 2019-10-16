#include "LogViewBase.h"
#include <LogLib/LogUtil.h>
#include <Shlwapi.h>

using namespace std;

CLogViewBase::CLogViewBase() {
    mLastPos = 0, mTotalCount = 0;
    mShowCount = 0;
    mScriptEngine = new CScriptEngine();
}

void CLogViewBase::InitLogBase() {
    mLastPos = 0, mTotalCount = 0;
    mShowCount = 0;
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
    SetStyle(STYLE_LOG_WARN, RGB(0, 0, 0), RGB(0xee, 0xb4, 0x22));
    SetStyle(STYLE_LOG_ERROR, RGB(0, 0, 0), RGB(0xf0, 0x80, 0x80));
}

CLogViewBase::~CLogViewBase() {
}

void CLogViewBase::PushLog(const std::mstring &content) {
    if (content.empty())
    {
        return;
    }

    size_t pos1 = 0, pos2 = 0;
    while ((pos1 = content.find("\n", pos2)) != mstring::npos) {
        pos2 = pos1 + 1;
        mTotalCount++;
    }

    mContent += content;
    const mstring &showStr = mFilterResult.mContent;
    size_t lastShow = showStr.size();
    mScriptEngine->InputLog(mContent, mLastPos, mFilterResult);

    pos1 = 0, pos2 = lastShow;
    while ((pos1 = mFilterResult.mContent.find("\n", pos2)) != mstring::npos) {
        pos2 = pos1 + 1;
        mShowCount++;
    }

    mLastPos = mContent.size();
    if (showStr.size() > lastShow)
    {
        mstring test1 = showStr.substr(lastShow, showStr.size() - lastShow);
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

        //重新计算展示数量
        mShowCount = 0;
        size_t pos1 = 0, pos2 = 0;
        while ((pos1 = mFilterResult.mContent.find("\n", pos2)) != mstring::npos) {
            pos2 = pos1 + 1;
            mShowCount++;
        }
    }
    return;
}

void CLogViewBase::GetLineCount(int &total, int &show) {
    show = mShowCount;
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
    mShowCount = 0;

    ClearView();
}

void CLogViewBase::OnLogStrStyle(const char *ptr, unsigned int startPos, int length, StyleContextBase *sc) const {
    map<mstring, int> style = mScriptEngine->GetStyleSet();
    if (style.empty())
    {
        sc->SetState(STYLE_CONTENT);
        sc->ForwardBytes(length);
        return;
    }

    int lastPos = 0;
    for (int i = 0 ; i < length ;)
    {
        bool match = false;
        for (map<mstring, int>::const_iterator it = style.begin() ; it != style.end() ; it++)
        {
            if (0 == StrCmpNIA(it->first.c_str(), ptr + i, it->first.size()))
            {
                if (i > lastPos)
                {
                    sc->SetState(STYLE_CONTENT);
                    sc->ForwardBytes(i - lastPos);
                }

                sc->SetState(it->second);
                sc->ForwardBytes(it->first.size());
                i += it->first.size();
                lastPos = i;
                match = true;
                break;
            }
        }

        if (!match)
        {
            i++;
        }
    }

    if (lastPos < length)
    {
        sc->SetState(STYLE_CONTENT);
        sc->ForwardBytes(length - lastPos);
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