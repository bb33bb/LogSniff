#include "TabCfgPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"

CTabCfgPage::CTabCfgPage() {
}

CTabCfgPage::~CTabCfgPage() {
}

void CTabCfgPage::MoveView(int x, int y, int cx, int cy) const {
    MoveWindow(mHwnd, x, y, cx, cy, TRUE);
}

void CTabCfgPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mEditPath = GetDlgItem(hwnd, IDC_CFG_PATH);
    HWND mEditExt = GetDlgItem(hwnd, IDC_CFG_EXT);
    HWND mPathList = GetDlgItem(hwnd, IDC_CFG_PATH_LIST);
    HWND mEditStatus = GetDlgItem(hwnd, IDC_CFG_INFO);

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