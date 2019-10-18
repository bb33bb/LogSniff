#pragma once
#include "LogViewBase.h"

class CDbgView : public CLogViewBase {
public:
    CDbgView();
    virtual ~CDbgView();

    bool CreateDbgView(HWND hParent, int x, int y, int cx, int cy);
    bool SetHightStr(const std::string &str);
private:
    void initDbgView();
private:
    std::string mKeywordStr;
}; 