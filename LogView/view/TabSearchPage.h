#pragma once
#include <Windows.h>
#include <map>
#include "DialogBase.h"
#include "LogSyntaxView.h"
#include "SearchSyntaxView.h"
#include "../../LogLib/locker.h"
#include "../MonitorBase.h"

class CTabSearchPage : public CDialogBase {
    struct SearchInfo {
        std::mstring mFilePath;
        std::mstring mContent;
    };

public:
    CTabSearchPage();
    virtual ~CTabSearchPage();
    void ClearLog();
    void SetLogServDesc(const LogServDesc *desc);

private:
    void SearchSingleFile(const std::mstring &filePath, std::list<SearchInfo> &result) const;
    void SearchStrInFiles() const;
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnSearchReturn(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);
    virtual INT_PTR GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    CScriptEngine mScriptEngine;
    CSearchView mSyntaxView;
    HWND mFltCtrl;
    HWND mCkRegular;
    HWND mFltEdit;
    std::mstring mFilterStr;
    const LogServDesc *mServDesc;
};