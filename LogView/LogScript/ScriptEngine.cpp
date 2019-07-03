#include "ScriptEngine.h"
#include <vector>
#include <Shlwapi.h>
#include <LogLib/StrUtil.h>

using namespace std;

CScriptEngine::CScriptEngine() {
    mColourSet.push_back(RGB(0, 0xff, 0));
    mColourSet.push_back(RGB(0xd0, 0xd0, 0xff));
    mColourSet.push_back(RGB(0xd0, 0xff, 0xd0));
    mColourSet.push_back(RGB(0xcd, 0xcd, 0x00));
    mColourSet.push_back(RGB(0xca, 0xff, 0x70));
    mColourSet.push_back(RGB(0xb0, 0xe2, 0xff));
    mColourSet.push_back(RGB(0xff, 0xd7, 0x00));

    mSplitStrSet.push_back("&&");
    mSplitStrSet.push_back("||");
    mSplitStrSet.push_back("(");
    mSplitStrSet.push_back(")");
}

CScriptEngine::~CScriptEngine() {
}

void CScriptEngine::ScriptCleanUp(mstring &script) const {
    mstring tmp;
    size_t lastPos = 0;
    bool spaceFlag = false;

    for (size_t i = 0 ; i < script.size() ;)
    {
        size_t j = 0;
        for (j = 0 ; j < mSplitStrSet.size() ; j++)
        {
            if (0 == script.comparei(mSplitStrSet[j], i))
            {
                break;
            }
        }

        if (j == mSplitStrSet.size())
        {
            tmp += script[i];
            i++;
        } else {
            tmp.trimright();
            tmp += mSplitStrSet[j];
            i += mSplitStrSet[j].size();

            while (i < script.size()) {
                char c = script[i];
                if (c == ' ' || c == '\t')
                {
                    i++;
                } else {
                    break;
                }
            }
        }
    }
    script = tmp;
}

//Compile Rule Without Bracket
vector<CScriptEngine::FilterRule> CScriptEngine::SimpleCompile(const mstring &script) const {
    vector<FilterRule> result;

    vector<mstring> strSet;
    size_t lastPos = 0;
    size_t i = 0;
    for (i = 0 ; i < script.size() ;) {
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

    if (lastPos <= (script.size() - 1))
    {
        strSet.push_back(script.substr(lastPos, script.size() - lastPos));
    }

    //Syntax Check
    for (i = 0 ; i < strSet.size() ; i++) {
        if (i % 2 != 0)
        {
            if (strSet[i] != "&&" && strSet[i] != "||")
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
                        tmp.mInclude.insert(cur);
                        result.push_back(tmp);
                    }
                } else {
                    if (varFlag)
                    {
                        result = CalAndResult(result, varContent);
                    } else {
                        result[result.size() - 1].mInclude.insert(cur);
                    }
                }
            } else if (1 == mode)
            {
                // ||
                if (varFlag)
                {
                    result = CalOrResult(result, varContent);
                } else {
                    tmp.mInclude.clear();
                    tmp.mInclude.insert(cur);
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
            tmp.mInclude.insert(t2.mInclude.begin(), t2.mInclude.end());
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

void CScriptEngine::SetRuleColour() {
    mRuleRgb.clear();

    int index = 0;
    size_t i = 0, j = 0, k = 0;
    for (i = 0 ; i < mRuleSet.size() ; i++)
    {
        const set<mstring> &tmp = mRuleSet[i].mInclude;
        for (set<mstring>::const_iterator j = tmp.begin() ; j != tmp.end() ; j++)
        {
            const mstring &keyWord = *j;

            if (mRuleRgb.end() == mRuleRgb.find(keyWord))
            {
                if (k < mColourSet.size())
                {
                    mRuleRgb[keyWord] = mColourSet[k];
                } else {
                    static DWORD sMagic = 0x12f;
                    srand(GetTickCount() + sMagic++);
                    BYTE r = rand() % 128 + 128;
                    BYTE g = rand() % 128 + 128;
                    BYTE b = rand() % 128 + 128;

                    mRuleRgb[mstring(keyWord).makelower()] = RGB(r, g, b);
                }
                k++;
            }
        }
    }
}

bool CScriptEngine::Compile(const mstring &str) {
    mstring script = str;
    ScriptCleanUp(script);

    if (str.empty())
    {
        ClearCache();
        return true;
    }

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

    //”Ô∑®◊≈…´
    SetRuleColour();
    //Create Search Index
    for (map<mstring, DWORD>::const_iterator it = mRuleRgb.begin() ; it != mRuleRgb.end() ; it++)
    {
        char c = it->first.c_str()[0];
        if (c >= 'A' && c <= 'Z')
        {
            c |= 32;
        }

        mSearchIndex[c].insert(mstring(it->first).makelower());

    }
    return true;
}

bool CScriptEngine::OnRuleFilter(const mstring &lineStr) const {
    for (size_t j = 0 ; j < mRuleSet.size() ; j++)
    {
        const FilterRule &rule = mRuleSet[j];
        set<mstring>::const_iterator k;
        for (k = rule.mInclude.begin() ; k != rule.mInclude.end() ; k++)
        {
            if (mstring::npos == lineStr.find(*k))
            {
                break;
            }
        }

        if (k == rule.mInclude.end())
        {
            return true;
        }
    }
    return false;
}

void CScriptEngine::OnStrColour(const mstring &filterStr, LogFilterResult &result) const {
    mstring low = filterStr;
    low.makelower();
    size_t initPos = result.mContent.size();

    for (size_t i = 0 ; i < low.size() ;)
    {
        bool match = false;
        for (map<mstring, DWORD>::const_iterator it = mRuleRgb.begin() ; it != mRuleRgb.end() ; it++)
        {
            if (0 == low.comparei(it->first, i))
            {
                LogKeyword node;
                node.mKeyword = filterStr.substr(i, it->first.size());
                node.mColour = it->second;
                node.mKeywordStart = initPos + i;
                node.mKeywordEnd = node.mKeywordStart + it->first.size();
                result.mKeywordSet.push_back(node);
                i += node.mKeyword.size();
                match = true;
                break;
            }
        }

        if (!match)
        {
            i++;
        }
    }
}

void CScriptEngine::ClearCache() {
    mRuleSet.clear();
    mRuleRgb.clear();
    mVarSet.clear();
    mSearchIndex.clear();
}

bool CScriptEngine::InputLog(const mstring &content, size_t initPos, LogFilterResult &result) {
    mstring filterStr;

    if (mRuleSet.empty()) {
        filterStr = content.substr(initPos, content.size() - initPos);
    } else {
        for (size_t i = initPos ; i < content.size() ;)
        {
            char c = content[i];
            if (c >= 'A' && c <= 'Z')
            {
                c |= 32;
            }

            bool passLine = false;
            map<char, set<mstring>>::const_iterator it;
            if (mSearchIndex.end() != (it = mSearchIndex.find(c)))
            {
                for (set<mstring>::const_iterator ij = it->second.begin() ; ij != it->second.end() ; ij++)
                {
                    if (0 == content.comparei(*ij, i))
                    {
                        //Rule Filter
                        size_t startPos = content.rfind('\n', i);
                        if (mstring::npos == startPos)
                        {
                            startPos = 0;
                        } else {
                            startPos += 1;
                        }

                        size_t endPos = content.find('\n', i + 1);
                        if (mstring::npos == endPos)
                        {
                            endPos = content.size();
                        }

                        mstring lineStr = content.substr(startPos, endPos - startPos);
                        if (OnRuleFilter(lineStr))
                        {
                            filterStr += lineStr;

                            if (lineStr.c_str()[lineStr.size() - 1] != '\n')
                            {
                                filterStr += '\n';
                            }
                        }

                        passLine = true;
                        i = endPos + 1;
                        break;
                    }
                }

                if (passLine)
                {
                    continue;
                }
            }
            i++;
        }

        OnStrColour(filterStr, result);
    }
    result.mContent += filterStr;
    return !filterStr.empty();
}