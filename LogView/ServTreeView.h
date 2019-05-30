#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <LogLib/mstring.h>
#include "LogServMgr.h"

class CServTreeDlg : public LogServEvent {
public:
    CServTreeDlg();
    virtual ~CServTreeDlg();

    BOOL CreateDlg(HWND hParent);
    HWND GetWindow();
    BOOL MoveWindow(int x, int y, int cx, int cy);

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    static INT_PTR CALLBACK ServTreeDlgProc(HWND hlg, UINT msg, WPARAM wp, LPARAM lp);

    HTREEITEM InsertItem(HTREEITEM parent, const std::mstring &name, void *param) const;
    BOOL SetItemStat(HTREEITEM treeItem, DWORD statMask) const;

private:
    virtual void OnLogServAdd(const LogServDesc *d);
    virtual void OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2);
    virtual void OnLogServAlter(const LogServDesc *d);

private:
    HWND mhWnd;
    HWND mParent;
    HWND mTreeCtrl;
};