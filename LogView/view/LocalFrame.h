#pragma once
#include <Windows.h>
#include "LogFrameBase.h"
#include "DbgView.h"
#include "LogSyntaxView.h"
#include "TabCfgPage.h"
#include "TabLogPage.h"

class CLocalLogFrame : public CLogFrameBase {
public:
    virtual void SetFilter(const std::mstring &str);
    virtual void ClearView();
    virtual void UpdateConfig();
    virtual void OnFileLog(const std::mstring &content);
    virtual void OnDbgLog(const std::mstring &content);
    virtual void OnFileSearchLog(const std::mstring &filePath, const std::mstring &content);

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnNotify(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND mTabCtrl;
    CTabCfgPage mCfgPage;
    CTabLogPage mDbgPage;
    CTabLogPage mLogPage;
    CTabLogPage mSearchPage;
};