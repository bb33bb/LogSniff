#include "LogViewBase.h"
#include <LogLib/LogUtil.h>

using namespace std;

CLogViewBase::CLogViewBase() {
    mLastPos = 0, mTotalCount = 0;
    mScriptEngine = new CScriptEngine();
}

void CLogViewBase::InitLogBase() {
    mLastPos = 0, mTotalCount = 0;
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

void CLogViewBase::OnViewUpdate() const {
    SyntaxTextView::OnViewUpdate();
}