#pragma once
#include <Windows.h>
#include <map>
#include "DialogBase.h"
#include "LogSyntaxView.h"
#include "../../LogLib/locker.h"

class CTabLogPage : public CDialogBase {
public:
    CTabLogPage();
    virtual ~CTabLogPage();
    void AppendLog(const std::mstring &label, const std::mstring &content);
    void ClearLog();

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnFilterReturn(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);
    virtual INT_PTR GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    CLogSyntaxView mSyntaxView;
    HWND mFltCtrl;
    HWND mFltEdit;
    //group ctrl
    HWND mGroupCtrl;
    //rule radio
    HWND mRadioRule;
    //regex radio
    HWND mRadioRegex;
    std::mstring mFilterStr;
};