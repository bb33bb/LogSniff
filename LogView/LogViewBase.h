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
    void OnLogStrStyle(const char *ptr, unsigned int startPos, int length, StyleContextBase *sc) const;
    static void __stdcall LogContentParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *s,
        void *param
        );
    virtual void OnViewUpdate() const;

private:
    std::mstring mContent;
    size_t mLastPos;
    LogFilterResult mFilterResult;
    CScriptEngine *mScriptEngine;
    std::mstring mRuleStr;
    size_t mTotalCount;

    std::vector<DWORD> mColourSet;
};