#include "ServTreeView.h"
#include "resource.h"
#include <CommCtrl.h>
#include <LogLib/winsize.h>

using namespace std;

CServTreeDlg::CServTreeDlg() {
    mhWnd = NULL;
    mParent = NULL;
    mTreeCtrl = NULL;
}

CServTreeDlg::~CServTreeDlg() {
}

BOOL CServTreeDlg::CreateDlg(HWND hParent) {
    mParent = hParent;
    mhWnd = CreateDialogParamA(NULL, MAKEINTRESOURCEA(IDD_SERV_TREE), hParent, ServTreeDlgProc, (LPARAM)this);
    return TRUE;
}

HWND CServTreeDlg::GetWindow() {
    return mhWnd;
}

BOOL CServTreeDlg::MoveWindow(int x, int y, int cx, int cy) {
    ::MoveWindow(mhWnd, x, y, cx, cy, TRUE);
    return TRUE;
}

HTREEITEM CServTreeDlg::InsertItem(HTREEITEM parent, const mstring &name, void *param) const {
    TVINSERTSTRUCTA st = {0};
    st.hParent = parent;
    st.hInsertAfter = TVI_LAST;
    st.item.mask = TVIF_TEXT | TVIF_PARAM;
    st.item.hItem = NULL;
    st.item.pszText = (LPSTR)name.c_str();
    st.item.cchTextMax = name.size();
    st.item.lParam = (LPARAM)param;
    return (HTREEITEM)SendMessageA(
        mTreeCtrl,
        TVM_INSERTITEMA,
        0,
        (LPARAM)(LPTV_INSERTSTRUCTA)(&st)
        );
}

BOOL CServTreeDlg::SetItemStat(HTREEITEM treeItem, DWORD statMask) const {
    TVITEMEXA item = {0};
    item.mask = TVIF_STATE | TVIF_HANDLE;
    item.hItem = treeItem;
    item.state = statMask;
    item.stateMask = statMask;
    return SendMessageA(
        (HWND)mTreeCtrl,
        TVM_SETITEMA,
        0,
        (LPARAM)(LPTV_INSERTSTRUCTA)(&item)
        );
}

void CServTreeDlg::OnLogServAdd(const LogServDesc *d) {
}

void CServTreeDlg::OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2) {
}

void CServTreeDlg::OnLogServAlter(const LogServDesc *d) {
}

INT_PTR CServTreeDlg::OnInitDialog(WPARAM wp, LPARAM lp) {
    mTreeCtrl = GetDlgItem(mhWnd, IDC_SERV_TREE);

    HTREEITEM root = TreeView_GetRoot(mTreeCtrl);
    HTREEITEM newItem = InsertItem(NULL, "本地服务", NULL);
    HTREEITEM item2 = NULL;

    InsertItem(newItem, "实时日志", NULL);
    item2 = InsertItem(newItem, "调试信息", NULL);
    InsertItem(newItem, "日志文件", NULL);

    UINT tt = TreeView_GetItemState(mTreeCtrl, newItem, TVIS_EXPANDED);
    SendMessage(mTreeCtrl, TVM_EXPAND, TVE_EXPAND, (LPARAM)newItem);
    SendMessage(mTreeCtrl, TVM_SELECTITEM, TVGN_CARET, (LPARAM)item2);

    CTL_PARAMS arry[] =
    {
        {NULL, mTreeCtrl, 0, 0, 1, 1},
        {IDC_BTN_REFUSH, NULL, 1, 1, 0, 0}
    };
    SetCtlsCoord(mhWnd, arry, sizeof(arry) / sizeof(CTL_PARAMS));
    return 0;
}

INT_PTR CServTreeDlg::OnCommand(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CServTreeDlg::OnClose(WPARAM wp, LPARAM lp) {
    EndDialog(mhWnd, 0);
    return 0;
}

INT_PTR CServTreeDlg::ServTreeDlgProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
    static CServTreeDlg *sPtr = NULL;

    if (msg == WM_INITDIALOG)
    {
        sPtr = (CServTreeDlg *)lp;
        sPtr->mhWnd = hdlg;
    }

    switch (msg)
    {
    case WM_INITDIALOG:
        sPtr->OnInitDialog(wp, lp);
        break;
    case WM_COMMAND:
        sPtr->OnCommand(wp, lp);
        break;
    case WM_CLOSE:
        sPtr->OnClose(wp, lp);
        break;
    }
    return 0;
}