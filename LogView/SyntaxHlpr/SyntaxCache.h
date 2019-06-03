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

    std::mstring GetViewStr(int startPos, int length) const;
    void OnViewUpdate() const;
    void SetSelStr(const std::mstring &str);
    void UpdateView() const;
private:
    bool IsKeyWordInCache(const std::mstring &keyWord) const;
    void InsertRuleNode(const RuleStatNode &node);
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
    std::mstring mRuleStr;
    std::mstring mSelStr;
};
