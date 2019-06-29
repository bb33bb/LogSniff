#pragma once
#include <Windows.h>
#include <vector>
#include <list>
#include <map>
#include <LogLib/mstring.h>

class ScriptException {
public:
    ScriptException(const std::mstring &str) {
        mErrDesc = str;
    }

    std::mstring what() {
        return mErrDesc;
    }
private:
    std::mstring mErrDesc;;
};

struct LogKeyword {
    std::mstring mKeyword;
    size_t mKeywordStart;
    size_t mKeywordEnd;
    DWORD mColour;
};

struct LogFilterResult {
    std::mstring mContent;
    std::list<LogKeyword> mKeywordSet;
    bool mValid;

    LogFilterResult() {
        mValid = false;
    }
};

class CScriptEngine {
    struct FilterRule {
        //KeyWord, and logic
        std::list<std::mstring> mKeywordSet;
    };

public:
    static CScriptEngine *GetInst();
    bool Compile(const std::mstring &script);
    LogFilterResult InputLog(const std::mstring &content);

private:
    CScriptEngine();
    virtual ~CScriptEngine();
    std::vector<FilterRule> SimpleCompile(const std::mstring &script) const;
    std::vector<FilterRule> CalAndResult(const std::vector<FilterRule> &a, const std::vector<FilterRule> &b) const;
    std::vector<FilterRule> CalOrResult(const std::vector<FilterRule> &a, const std::vector<FilterRule> &b) const;
    void ScriptCleanUp(std::mstring &script) const;

private:
    std::vector<FilterRule> mRuleSet;
    std::map<std::mstring, std::vector<FilterRule>> mVarSet;
};