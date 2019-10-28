#pragma once
#include <LogLib/mstring.h>
#include <map>
#include "../SyntaxHlpr/SyntaxCache.h"
#include "../LogScript/ScriptEngine.h"

//File Search Syntax View
class CSearchView : public CSyntaxCache {
public:
    CSearchView();
    virtual ~CSearchView();
    bool CreateSearchView(HWND hParent, int x, int y, int cx, int cy);
    void SetStyleSet(std::map<std::mstring, int> &set1);

    void PushSearchResult(const std::mstring &filePath, const std::mstring &content);
    void GetLineCount(int &total, int &show);;
    void ClearSearchView();
private:
    void InitStyle();
    void InitViewStyle();
    void InitSearchView();
    //1分钟前, 1小时20分钟前, 1天前, 1周前, 1月前
    //time:Local FileTime
    std::mstring GetTimeDesc(const FILETIME &time) const;
    std::mstring GetLogFileInfo(const std::mstring &filePath) const;
    static void __stdcall SearchPathParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *s,
        void *param
        );

    void OnSearchLogStyle(const char *ptr, unsigned int startPos, int length, StyleContextBase *sc) const;
    static void __stdcall SearchLogParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *s,
        void *param
        );

private:
    std::map<std::mstring, int> mStyleSet;
    size_t mTotalCount;
    size_t mShowCount;
};