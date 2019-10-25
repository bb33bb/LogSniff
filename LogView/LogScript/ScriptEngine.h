#pragma once
#include <Windows.h>
#include <vector>
#include <list>
#include <map>
#include <set>
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
    int mStyle;
    DWORD mColour;

    LogKeyword() {
        mKeywordStart = 0;
        mKeywordEnd = 0;
        mStyle = 0, mColour = 0;
    }
};

struct LogFilterResult {
    std::mstring mContent;
    std::vector<LogKeyword> mKeywordSet;
    bool mValid;

    LogFilterResult() {
        mValid = false;
    }

    void Clear() {
        mContent.clear();
        mKeywordSet.clear();
        mValid = false;
    }
};

class CScriptEngine {
    struct FilterRule {
        //KeyWord, and logic
        std::set<std::mstring> mInclude;
        std::set<std::mstring> mExclude;

        void Clear() {
            mInclude.clear();
            mExclude.clear();
        }
    };

public:
    CScriptEngine();
    virtual ~CScriptEngine();
    bool Compile(const std::mstring &script);
    std::map<std::mstring, int> GetStyleSet();
    bool InputLog(const std::mstring &content, size_t initPos, LogFilterResult &result) const;

private:
    std::vector<FilterRule> SimpleCompile(const std::mstring &script) const;
    std::vector<FilterRule> CalAndResult(const std::vector<FilterRule> &a, const std::vector<FilterRule> &b) const;
    std::vector<FilterRule> CalOrResult(const std::vector<FilterRule> &a, const std::vector<FilterRule> &b) const;
    void ScriptCleanUp(std::mstring &script) const;
    void SetRuleStyle();
    bool OnRuleFilter(const char *content, size_t length) const;
    void OnStrColour(const std::mstring &filterStr, LogFilterResult &result) const;
    void ClearCache();

private:
    std::vector<FilterRule> mRuleSet;
    std::map<std::mstring, int> mRuleStyle;
    std::vector<std::mstring> mSplitStrSet;
    std::map<std::mstring, std::vector<FilterRule>> mVarSet;
};