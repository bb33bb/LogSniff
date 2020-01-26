#pragma once
#include <Windows.h>
#include <map>
#include "DialogBase.h"
#include "LogSyntaxView.h"
#include "../../LogLib/locker.h"
#include "../GlobalDef.h"

class CTabLogPage : public CDialogBase {
public:
    CTabLogPage();
    virtual ~CTabLogPage();
    void SetType(EM_LOGVIEW_TYPE type);
    void AppendLog(const std::mstring &label, const std::mstring &content);
    void ClearLog();
    void SetAutoScroll(bool flag);
    void LoadCfg();

private:
    void OnEnterFilterStr(const std::mstring &str);
    void OnEnterFindStr(const std::mstring &str);
    void InsertStrList(HWND hComCtrl, HWND hComEdit, const std::list<std::mstring> &set1) const;
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnFilterReturn(WPARAM wp, LPARAM lp);
    INT_PTR OnFindReturn(WPARAM wp, LPARAM lp);
    //查找接口,下一个匹配项
    INT_PTR OnFindNext();
    //查找接口,前面一个匹配项
    INT_PTR OnFindFront();
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);
    virtual INT_PTR GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    CLogSyntaxView mSyntaxView;
    EM_LOGVIEW_TYPE mType;
    //Filter Ctrl
    HWND mFltCtrl;
    HWND mFltEdit;
    //Find Ctrl
    HWND mFindCtrl;
    HWND mFindEdit;
    //group ctrl
    HWND mGroupCtrl;
    //rule radio
    HWND mRadioRule;
    //regex radio
    HWND mRadioRegex;
    std::mstring mFilterStr;
};