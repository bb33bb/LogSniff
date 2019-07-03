#pragma once
#include <LogLib/mstring.h>
#include "SyntaxHlpr/SyntaxCache.h"
#include "LogScript/ScriptEngine.h"

class CLogViewBase : public CSyntaxCache {
public:
    CLogViewBase();
    virtual ~CLogViewBase();
    void InitLogBase();
    void PushLog(const std::mstring &content);
    void SetFilter(const std::mstring &newFilter);

    void GetLineCount(int &show, int &total);
    std::mstring GetRuleStr();
    void ClearLogView();
private:
    virtual void OnViewUpdate() const;

private:
    std::mstring mContent;
    size_t mLastPos;
    LogFilterResult mFilterResult;
    CScriptEngine *mScriptEngine;
    std::mstring mRuleStr;
    size_t mTotalCount;
};