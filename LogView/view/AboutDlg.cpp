#include "AboutDlg.h"
#include "../../LogLib/LogUtil.h"
#include "../resource.h"

CAboutDlg::CAboutDlg() {
}

CAboutDlg::~CAboutDlg() {
}

INT_PTR CAboutDlg::OnInitDialog(WPARAM wp, LPARAM lp) {
    CentreWindow(mHwnd, NULL);

    extern HINSTANCE g_hInstance;
    HICON ico = LoadIconA(g_hInstance, MAKEINTRESOURCEA(IDI_MAIN));
    SendMessageA(GetDlgItem(mHwnd, IDC_ABOUT_PICTURE), STM_SETICON, (WPARAM)ico, 0);
    return 0;
}

INT_PTR CAboutDlg::OnCommand(WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (IDC_ABOUT_BTN_OK == id)
    {
        SendMessageA(mHwnd, WM_CLOSE, 0, 0);
    }
    return 0;
}

INT_PTR CAboutDlg::OnClose(WPARAM wp, LPARAM lp) {
    EndDialog(mHwnd, 0);
    return 0;
}

//系统消息回调
INT_PTR CAboutDlg::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    if (WM_INITDIALOG == msg)
    {
        return OnInitDialog(wp, lp);
    } else if (WM_COMMAND == msg)
    {
        return OnCommand(wp, lp);
    } else if (WM_CLOSE == msg)
    {
        return OnClose(wp, lp);
    }
    return 0;
}