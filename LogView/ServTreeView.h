#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <LogLib/mstring.h>
#include "LogServMgr.h"

class CServTreeDlg : public LogServEvent {
    enum TreeNodeType {
        em_tree_root_node,
        em_tree_file_log,
        em_tree_dbg_msg,
        em_tree_dir,
        em_tree_file
    };

    struct TreeCtrlParam {
        TreeNodeType mNodeType;
        const LogServDesc *mServDesc;
        mstring mFilePath;

        TreeCtrlParam() {
            mNodeType = em_tree_root_node;
            mServDesc = NULL;
        }
    };

public:
    CServTreeDlg();
    virtual ~CServTreeDlg();

    BOOL CreateDlg(HWND hParent);
    HWND GetWindow();
    BOOL MoveWindow(int x, int y, int cx, int cy);

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnNotify(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    INT_PTR OnServAddedInternal(const LogServDesc *desc);
    static INT_PTR CALLBACK ServTreeDlgProc(HWND hlg, UINT msg, WPARAM wp, LPARAM lp);

    HTREEITEM InsertItem(HTREEITEM parent, const std::mstring &name, const TreeCtrlParam *param) const;
    BOOL SetItemStat(HTREEITEM treeItem, DWORD statMask) const;

private:
    virtual void OnLogServAdd(const LogServDesc *d);
    virtual void OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2);
    virtual void OnLogServAlter(const LogServDesc *d);

private:
    HWND mhWnd;
    HWND mParent;
    HWND mTreeCtrl;
    std::vector<const LogServDesc *> mServDesc;
};