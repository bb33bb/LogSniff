#pragma once
#include "SyntaxHlpr/SyntaxView.h"

class CLogSyntaxView : public SyntaxView {
public:
    CLogSyntaxView();
    virtual ~CLogSyntaxView();

    bool CreateLogView(HWND hParent, int x, int y, int cx, int cy);
private:
    void initLogView();

    static void __stdcall LogParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc
        );
}; 