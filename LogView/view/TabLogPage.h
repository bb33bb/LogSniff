#pragma once
#include <Windows.h>
#include "DialogBase.h"
#include "LogSyntaxView.h"

class CTabLogPage : public CDialogBase {
public:
    CTabLogPage();
    virtual ~CTabLogPage();
    void MoveWindow(int x, int y, int cx, int cy) const;

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    CLogSyntaxView mSyntaxView;
    HWND mFltEdit;
    HWND mCkRegular;
};