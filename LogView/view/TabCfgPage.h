#pragma once
#include <Windows.h>
#include "DialogBase.h"
#include "../LogServMgr.h"

class CTabCfgPage : public CDialogBase {
public:
    CTabCfgPage();
    virtual ~CTabCfgPage();
    void SetServDesc(const LogServDesc *servDesc);
    void MoveView(int x, int y, int cx, int cy) const;

protected:
    void LoadPathSet() const;
    void InitListCtrl() const;
    void OnInitDialog(WPARAM wp, LPARAM lp);
    void OnCommand(WPARAM wp, LPARAM lp);
    void OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND mEditPath;
    HWND mEditExt;
    HWND mPathList;
    HWND mEditStatus;
    const LogServDesc *mServDesc;
};