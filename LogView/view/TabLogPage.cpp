#include "TabLogPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"

CTabLogPage::CTabLogPage() {
}

CTabLogPage::~CTabLogPage() {
}

void CTabLogPage::MoveWindow(int x, int y, int cx, int cy) const {
    ::MoveWindow(mHwnd, x, y, cx, cy, TRUE);
}

INT_PTR CTabLogPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mFltEdit = GetDlgItem(hwnd, IDC_COM_FILTER);
    mCkRegular = GetDlgItem(hwnd, IDC_CK_REGULAR);

    RECT rtClient = {0};
    GetClientRect(hwnd, &rtClient);
    RECT rtFlt = {0};
    GetWindowRect(mFltEdit, &rtFlt);
    MapWindowPoints(NULL, hwnd, (LPPOINT)&rtFlt, 2);

    int x = rtFlt.left;
    int y = rtFlt.bottom + 5;
    int width = (rtClient.right - rtClient.left) - 2 * x;
    int high = (rtClient.bottom - rtClient.top) - y - 5;
    mSyntaxView.CreateLogView(hwnd, x, y, width, high);

    CTL_PARAMS arry[] = {
        {IDC_COM_FILTER, NULL, 0, 0, 1, 0},
        {IDC_CK_REGULAR, NULL, 1, 0, 0, 0},
        {0, mSyntaxView.GetWindow(), 0, 0, 1, 1}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));
    return 0;
}

INT_PTR CTabLogPage::OnClose(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CTabLogPage::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    if (WM_INITDIALOG == msg)
    {
        return OnInitDialog(wp, lp);
    } else if (WM_CLOSE == msg)
    {
        return OnClose(wp, lp);
    }
    return 0;
}