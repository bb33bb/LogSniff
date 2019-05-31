#pragma once
#include "SyntaxHlpr/SyntaxCache.h"

class CLogSyntaxView : public CSyntaxCache {
public:
    CLogSyntaxView();
    virtual ~CLogSyntaxView();

    bool CreateLogView(HWND hParent, int x, int y, int cx, int cy);
    bool SetHightStr(const std::string &str);
private:
    void initLogView();

    static void __stdcall LogParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc,
        void *param
        );

private:
    std::string mKeywordStr;
}; 