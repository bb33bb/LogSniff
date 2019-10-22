#include "LocalFrame.h"
#include "../../LogLib/winsize.h"
#include "../DbgMsg.h"
#include "../resource.h"
#include <CommCtrl.h>

using namespace std;

CLocalLogFrame::CLocalLogFrame() :mServDesc(NULL) {
}

CLocalLogFrame::~CLocalLogFrame() {
}

void CLocalLogFrame::InitLogFrame(const LogServDesc *servDesc) {
    mServDesc = servDesc;
}

void CLocalLogFrame::UpdateConfig() {
}

void CLocalLogFrame::OnFileLog(const mstring &content) {
    mLogPage.AppendLog(LABEL_LOG_CONTENT, content);
}

void CLocalLogFrame::OnDbgLog(const mstring &content) {
    mDbgPage.AppendLog(LABEL_DBG_CONTENT, content);
}

void CLocalLogFrame::OnFileSearchLog(const mstring &filePath, const mstring &content) {
}

INT_PTR CLocalLogFrame::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mTabCtrl = GetDlgItem(hwnd, IDC_LOCAL_TAB);
    mCfgPage.SetServDesc(mServDesc);
    mCfgPage.CreateDlg(hwnd, IDD_TAB_CONFIG);
    mDbgPage.CreateDlg(hwnd, IDD_TAB_LOG);
    mLogPage.CreateDlg(hwnd, IDD_TAB_LOG);
    mSearchPage.CreateDlg(hwnd, IDD_TAB_LOG);

    //Init Tab Ctrl
    TCITEMW tc = {0};
    tc.mask = TCIF_TEXT;
    tc.pszText = L"配置选项";
    TabCtrl_InsertItem(mTabCtrl, 0, &tc);

    tc.mask = TCIF_TEXT;
    tc.pszText = L"调试输出";
    TabCtrl_InsertItem(mTabCtrl, 1, &tc);

    tc.mask = TCIF_TEXT;
    tc.pszText = L"文件日志";
    TabCtrl_InsertItem(mTabCtrl, 2, &tc);

    tc.mask = TCIF_TEXT;
    tc.pszText = L"文件检索";
    TabCtrl_InsertItem(mTabCtrl, 3, &tc);;

    RECT rtTab = {0};
    GetClientRect(hwnd, &rtTab);
    TabCtrl_AdjustRect(mTabCtrl, FALSE, &rtTab);
    MapWindowPoints(mTabCtrl, hwnd, (LPPOINT)&rtTab, 2);
    mCfgPage.MoveView(rtTab.left, rtTab.top, rtTab.right - rtTab.left, rtTab.bottom - rtTab.top);
    mDbgPage.MoveWindow(rtTab.left, rtTab.top, rtTab.right - rtTab.left, rtTab.bottom - rtTab.top);
    mLogPage.MoveWindow(rtTab.left, rtTab.top, rtTab.right - rtTab.left, rtTab.bottom - rtTab.top);
    mSearchPage.MoveWindow(rtTab.left, rtTab.top, rtTab.right - rtTab.left, rtTab.bottom - rtTab.top);
    mCfgPage.ShowDlg();
    mDbgPage.HideDlg();
    mLogPage.HideDlg();
    mSearchPage.HideDlg();

    CTL_PARAMS arry[] = {
        {NULL, mTabCtrl, 0, 0, 1, 1},
        {NULL, mCfgPage.GetHandle(), 0, 0, 1, 1},
        {NULL, mDbgPage.GetHandle(), 0, 0, 1, 1},
        {NULL, mLogPage.GetHandle(), 0, 0, 1, 1},
        {NULL, mSearchPage.GetHandle(), 0, 0, 1, 1}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));

    //初始化调试信息引擎
    CDbgCapturer::GetInst()->InitCapturer();
    return 0;
}

INT_PTR CLocalLogFrame::OnCommand(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CLocalLogFrame::OnNotify(WPARAM wp, LPARAM lp) {
    NMHDR *hdr = (NMHDR *)lp;

    if (hdr->hwndFrom == mTabCtrl)
    {
        switch(hdr->code)
        {
        case TCN_SELCHANGE:
            {
                switch (TabCtrl_GetCurSel(mTabCtrl)) {
                    case 0:
                        {
                            mCfgPage.ShowDlg();
                            mDbgPage.HideDlg();
                            mLogPage.HideDlg();
                            mSearchPage.HideDlg();
                        }
                        break;
                    case 1:
                        {
                            mCfgPage.HideDlg();
                            mDbgPage.ShowDlg();
                            mLogPage.HideDlg();
                            mSearchPage.HideDlg();
                        }
                        break;
                    case 2:
                        {
                            mCfgPage.HideDlg();
                            mDbgPage.HideDlg();
                            mLogPage.ShowDlg();
                            mSearchPage.HideDlg();
                        }
                        break;
                    case 3:
                        {
                            mCfgPage.HideDlg();
                            mDbgPage.HideDlg();
                            mLogPage.HideDlg();
                            mSearchPage.ShowDlg();
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

INT_PTR CLocalLogFrame::OnClose(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CLocalLogFrame::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG:
            OnInitDialog(wp, lp);
            break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            break;
        case WM_NOTIFY:
            OnNotify(wp, lp);
            break;
        case WM_CLOSE:
            OnClose(wp, lp);
            break;
        default:
            return 0;
    }
    return 0;
}