#include "ScriptEngine.h"
#include <vector>
#include <Shlwapi.h>
#include <LogLib/StrUtil.h>

using namespace std;

CScriptEngine *CScriptEngine::GetInst() {
    static CScriptEngine *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CScriptEngine();
    }
    return sPtr;
}

CScriptEngine::CScriptEngine() {
}

CScriptEngine::~CScriptEngine() {
}

void CScriptEngine::ScriptCleanUp(mstring &script) const {
    script.repsub(" &&", "&&");
    script.repsub("&& ", "&&");

    script.repsub(" ||", "||");
    script.repsub("|| ", "||");

    script.repsub(" (", "(");
    script.repsub("( ", "(");
    script.repsub(" )", ")");
    script.repsub(") ", ")");
}

//Compile Rule Without Bracket
vector<CScriptEngine::FilterRule> CScriptEngine::SimpleCompile(const mstring &script) const {
    vector<FilterRule> result;

    vector<mstring> strSet;
    size_t lastPos = 0;
    size_t i = 0;
    for (size_t i = 0 ; i < script.size() ;) {
        if (i < script.size() - 1)
        {
            if (0 == StrCmpNA(script.c_str() + i, "&&", 2) || 0 == StrCmpNA(script.c_str() + i, "||", 2))
            {
                if (i > (lastPos + 1))
                {
                    mstring sub = script.substr(lastPos, i - lastPos);
                    strSet.push_back(sub);
                    strSet.push_back(script.substr(i, 2));
                    i = i + 2;
                    lastPos = i;
                    continue;
                }
            }
        }
        i++;
    }

    if (lastPos < (script.size() - 1))
    {
        strSet.push_back(script.substr(lastPos, script.size() - lastPos));
    }

    //Syntax Check
    for (i = 0 ; i < strSet.size() ; i++) {
        if (i % 2 != 0)
        {
            if (strSet[i] != "&&" && strSet[i] == "||")
            {
                throw (ScriptException("”Ô∑®¥ÌŒÛ"));
                return vector<FilterRule>();
            }
        }

        if (i % 2 == 0)
        {
            if (strSet[i] == "&&" || strSet[i] == "||")
            {
                throw (ScriptException("”Ô∑®¥ÌŒÛ"));
                return vector<FilterRule>();
            }
        }
    }

    //Push Rule Node To Result
    int mode = 0;   //0:&&, 1:||
    FilterRule tmp;
    for(i = 0 ; i < strSet.size() ; i++) {
        const string &cur = strSet[i];
        if (i % 2 == 0)
        {
            bool varFlag = false;
            vector<FilterRule> varContent;

            map<mstring, vector<FilterRule>>::const_iterator it;
            if (mVarSet.end() != (it = mVarSet.find(cur)))
            {
                varFlag = true;
                varContent = it->second;
            }

            if (0 == mode)
            {
                // &&
                if (result.empty())
                {
                    if (varFlag)
                    {
                        result = varContent;
                    } else {
                        tmp.mKeywordSet.push_back(cur);
                        result.push_back(tmp);
                    }
                } else {
                    if (varFlag)
                    {
                        result = CalAndResult(result, varContent);
                    } else {
                        result[result.size() - 1].mKeywordSet.push_back(cur);
                    }
                }
            } else if (1 == mode)
            {
                // ||
                if (varFlag)
                {
                    result = CalOrResult(result, varContent);
                } else {
                    tmp.mKeywordSet.clear();
                    tmp.mKeywordSet.push_back(cur);
                    result.push_back(tmp);
                }
            }
        } else {
            if (strSet[i] == "&&")
            {
                mode = 0;
            } else {
                mode = 1;
            }
        }
    }
    return result;
}

// (a || b) && (c || d)->(a && c) || (a && d) || (b && c) || (b && d)
// (a || c) || (a || d)
vector<CScriptEngine::FilterRule> CScriptEngine::CalAndResult(const vector<CScriptEngine::FilterRule> &a, const vector<CScriptEngine::FilterRule> &b) const {
    vector<FilterRule> result;

    size_t i = 0, j = 0;
    for (i = 0 ; i < a.size() ; i++)
    {
        const FilterRule &t1 = a[i];
        for (j = 0 ; j < b.size() ; j++)
        {
            const FilterRule &t2 = b[j];
            FilterRule tmp = t1;
            tmp.mKeywordSet.insert(tmp.mKeywordSet.end(), t2.mKeywordSet.begin(), t2.mKeywordSet.end());
            result.push_back(tmp);
        }
    }
    return result;
}

vector<CScriptEngine::FilterRule> CScriptEngine::CalOrResult(const vector<CScriptEngine::FilterRule> &a, const vector<CScriptEngine::FilterRule> &b) const {
    vector<FilterRule> result;
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

bool CScriptEngine::Compile(const mstring &str) {
    mstring script = str;
    ScriptCleanUp(script);

    //Check Syntax For Bracket
    mVarSet.clear();
    list<size_t> st1;
    size_t varIndex = 0;
    size_t i = 0;
    for (i = 0 ; i < script.size() ;)
    {
        char c = script[i];
        if (c == '(')
        {
            st1.push_back(i);
        }

        if (c == ')')
        {
            if (st1.empty())
            {
                throw (ScriptException("”Ô∑®¥ÌŒÛ,¿®∫≈≈‰∂‘ ß∞‹"));
                return false;
            }

            size_t pos1 = st1.back();
            size_t pos2 = i;
            st1.pop_back();

            mstring simple = script.substr(pos1 + 1, pos2 - pos1 - 1);
            vector<FilterRule> result = SimpleCompile(simple);

            mstring varName = FormatA("var_<%04d>", varIndex++);
            mVarSet[varName] = result;
            script.replace(pos1, pos2 - pos1 + 1, varName);
            i = pos1 + varName.size();
            continue;
        }
        i++;
    }

    if (!st1.empty())
    {
        throw (ScriptException("”Ô∑®¥ÌŒÛ,¿®∫≈≈‰∂‘ ß∞‹"));
        return false;
    }
    mRuleSet = SimpleCompile(script);
    return true;
}

LogFilterResult InputLog(const mstring &content) {
    LogFilterResult result;

    return result;
}