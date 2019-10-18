#pragma once
#include <Windows.h>
#include "DialogBase.h"

class CTabCfgPage : public CDialogBase {
public:
    CTabCfgPage();
    virtual ~CTabCfgPage();
    void MoveView(int x, int y, int cx, int cy) const;

protected:
    void OnInitDialog(WPARAM wp, LPARAM lp);
    void OnCommand(WPARAM wp, LPARAM lp);
    void OnClose(WPARAM wp, LPARAM lp);
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND mEditPath;
    HWND mEditExt;
    HWND mPathList;
    HWND mEditStatus;
};