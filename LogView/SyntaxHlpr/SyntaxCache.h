#pragma once
#include <Windows.h>
#include <list>
#include "SyntaxView.h"
#include "export.h"
#include <LogLib/mstring.h>
#include <LogLib/locker.h>
#include <map>
#include <vector>

struct RuleStatNode {
    size_t mStartPos;
    size_t mEndPos;
    int mStat;
    std::mstring mRule;
};

class CSyntaxCache : public SyntaxView, public RLocker {
public:
    CSyntaxCache();
    virtual ~CSyntaxCache();

    bool InitCache(const std::mstring &label, int interval);
    void PushToCache(const std::mstring &content);
    void SetFilter(const std::mstring &rule);

    void SetKeyWord(const std::mstring &keyWord, int stat);
    void DelKeyWord(const std::mstring &keyWord);
    void ClearKeyWord();
private:
    void InsertRuleNode(const RuleStatNode &node);
    void UpdateView();
    static void CALLBACK TimerCache(HWND hwnd,
        UINT msg,
        UINT_PTR id,
        DWORD time
        );
    static void __stdcall LogParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc,
        void *param
        );

    struct DataCache {
        std::mstring mLabel;
        std::mstring mContent;
        void *mParam;

        DataCache() {
            mParam = NULL;
        }
    };
private:
    int mInterval;
    std::mstring mLabel;
    std::mstring mCache;
    static std::map<HWND, CSyntaxCache *> msTimerCache;
    std::mstring mContent;
    std::mstring mShowData;
    std::mstring mRule;
    std::map<std::mstring, int> mKeyWordSet;

    int mStartPos;
    int mEndPos;
    std::vector<RuleStatNode> mRuleNodeSet;
};
