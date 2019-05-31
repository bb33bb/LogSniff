#include <WinSock2.h>
#include "ServTreeView.h"
#include "resource.h"
#include <CommCtrl.h>
#include <LogLib/winsize.h>
#include "MainView.h"

using namespace std;

#define MSG_LOGSERV_ADD     (WM_USER + 6011)

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
    CLogServMgr::GetInst()->Register(this);
    return TRUE;
}

HWND CServTreeDlg::GetWindow() {
    return mhWnd;
}

BOOL CServTreeDlg::MoveWindow(int x, int y, int cx, int cy) {
    ::MoveWindow(mhWnd, x, y, cx, cy, TRUE);
    return TRUE;
}

HTREEITEM CServTreeDlg::InsertItem(HTREEITEM parent, const mstring &name, const TreeCtrlParam *param) const {
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
    mServDesc.push_back(d);

    SendMessageA(mhWnd, MSG_LOGSERV_ADD, (WPARAM)d, 0);
}

void CServTreeDlg::OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2) {
}

void CServTreeDlg::OnLogServAlter(const LogServDesc *d) {
}

INT_PTR CServTreeDlg::OnInitDialog(WPARAM wp, LPARAM lp) {
    mTreeCtrl = GetDlgItem(mhWnd, IDC_SERV_TREE);

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

INT_PTR CServTreeDlg::OnNotify(WPARAM wp, LPARAM lp) {
    NMHDR *headr = (NMHDR *)lp;

    if (headr->code == NM_DBLCLK)
    {
        HTREEITEM sel = TreeView_GetSelection(mTreeCtrl);

        TVITEM itm = {0};
        itm.mask = TVIF_PARAM;
        itm.hItem = sel;
        TreeView_GetItem(mTreeCtrl, &itm);

        const TreeCtrlParam *desc = (const TreeCtrlParam *)itm.lParam;
        int d = 123;

        if (desc->mNodeType == em_tree_file_log)
        {
            SwitchWorkMode(em_mode_logFile);
        } else if (desc->mNodeType == em_tree_dbg_msg)
        {
            SwitchWorkMode(em_mode_debugMsg);
        }
        //SendMessage(mTreeCtrl, TVM_EXPAND, TVE_EXPAND, (LPARAM)newItem);
        //SendMessage(mTreeCtrl, TVM_SELECTITEM, TVGN_CARET, (LPARAM)item2);
    }
    return 0;
}

INT_PTR CServTreeDlg::OnServAddedInternal(const LogServDesc *desc) {
    if (desc->mLogServType == em_log_serv_local)
    {
        TreeCtrlParam *root = new TreeCtrlParam();
        root->mNodeType = em_tree_root_node;
        root->mServDesc = desc;
        HTREEITEM t1 = InsertItem(NULL, "本地服务", root);

        TreeCtrlParam *param1 = new TreeCtrlParam();
        param1->mNodeType = em_tree_file_log;
        param1->mServDesc = desc;
        HTREEITEM t2 = InsertItem(t1, "文件日志", param1);

        TreeCtrlParam *param2 = new TreeCtrlParam();
        param2->mNodeType = em_tree_dbg_msg;
        param2->mServDesc = desc;
        HTREEITEM t3 = InsertItem(t1, "调试信息", param2);

        TreeCtrlParam *param3 = new TreeCtrlParam();
        param3->mNodeType = em_tree_dir;
        param3->mServDesc = desc;
        HTREEITEM t4 = InsertItem(t1, "日志文件", param3);
    }
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

    if (!sPtr)
    {
        return 0;
    }

    switch (msg)
    {
    case WM_INITDIALOG:
        sPtr->OnInitDialog(wp, lp);
        break;
    case WM_COMMAND:
        sPtr->OnCommand(wp, lp);
        break;
    case WM_NOTIFY:
        sPtr->OnNotify(wp, lp);
        break;
    case MSG_LOGSERV_ADD:
        sPtr->OnServAddedInternal((const LogServDesc *)wp);
        break;
    case WM_CLOSE:
        sPtr->OnClose(wp, lp);
        break;
    }
    return 0;
}