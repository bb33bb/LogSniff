#pragma once
#include "LogViewBase.h"

class CLogSyntaxView : public CLogViewBase {
public:
    CLogSyntaxView();
    virtual ~CLogSyntaxView();

    bool CreateLogView(HWND hParent, int x, int y, int cx, int cy);
private:
    void initLogView();
private:
    std::string mKeywordStr;
}; 