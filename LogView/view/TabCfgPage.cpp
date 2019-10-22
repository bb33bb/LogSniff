#include "TabCfgPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"
#include "../../LogLib/StrUtil.h"
#include <CommCtrl.h>

using namespace std;

CTabCfgPage::CTabCfgPage() {
}

CTabCfgPage::~CTabCfgPage() {
}

void CTabCfgPage::SetServDesc(const LogServDesc *servDesc) {
    mServDesc = servDesc;
}

void CTabCfgPage::MoveView(int x, int y, int cx, int cy) const {
    MoveWindow(mHwnd, x, y, cx, cy, TRUE);
}

void CTabCfgPage::InitListCtrl() const {
    ListView_SetExtendedListViewStyle(mPathList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    LVCOLUMNA col;
    memset(&col, 0x00, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 40;
    col.pszText = (LPSTR)"ÐòºÅ";
    SendMessageA(mPathList, LVM_INSERTCOLUMNA, 0, (LPARAM)&col);

    col.cx = 640;
    col.pszText = (LPSTR)"Â·¾¶";
    SendMessageA(mPathList, LVM_INSERTCOLUMNA, 1, (LPARAM)&col);
}

void CTabCfgPage::LoadPathSet() const {
    const list<mstring> &set1 = mServDesc->mPathSet;
    int index = 0;
    for (list<mstring>::const_iterator it = set1.begin() ; it != set1.end() ; it++, index++)
    {
        LVITEMA info = {0};
        info.mask = LVIF_TEXT ;
        info.iItem = index;

        info.iSubItem = 0;
        mstring str = FormatA("%d", index);
        info.pszText = (LPSTR)(str.c_str());
        SendMessageA(mPathList, LVM_INSERTITEMA, 0, (LPARAM)&info);

        info.iSubItem = 1;
        info.pszText = (LPSTR)it->c_str();
        SendMessageA(mPathList, LVM_SETITEMA, 0, (LPARAM)&info);
    }
}

void CTabCfgPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mEditPath = GetDlgItem(hwnd, IDC_CFG_PATH);
    mEditExt = GetDlgItem(hwnd, IDC_CFG_EXT);
    mPathList = GetDlgItem(hwnd, IDC_CFG_PATH_LIST);
    mEditStatus = GetDlgItem(hwnd, IDC_CFG_INFO);

    InitListCtrl();
    LoadPathSet();

    CTL_PARAMS arry[] = {
        {IDC_CFG_PATH, NULL, 0, 0, 1, 0},
        {IDC_CFG_EXT, NULL, 0, 0, 1, 0},
        {IDC_CFG_PATH_LIST, NULL, 0, 0, 1, 1},
        {IDC_CFG_INFO, NULL, 0, 1, 1, 0},
        {IDC_CFG_OK, NULL, 1, 1, 0, 0}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));
}

void CTabCfgPage::OnCommand(WPARAM wp, LPARAM lp) {
}

void CTabCfgPage::OnClose(WPARAM wp, LPARAM lp) {
}

INT_PTR CTabCfgPage::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG:
            OnInitDialog(wp, lp);
            break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            break;
        case WM_CLOSE:
            OnClose(wp, lp);
            break;
    }
    return 0;
}