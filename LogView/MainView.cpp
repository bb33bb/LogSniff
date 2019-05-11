#include "MainView.h"
#include "resource.h"
#include <LogLib/LogUtil.h>
#include <LogLib/winsize.h>
#include "SyntaxHlpr/SyntaxView.h"
#include <CommCtrl.h>
#include "LogServView.h"
#include "LogSyntaxView.h"

#pragma comment(lib, "comctl32.lib")

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gs_hStatBar = NULL;
static HWND gs_hFilter = NULL;
static HWND gs_hCkRegular = NULL;
static CLogSyntaxView *gsLogView = NULL;

#define IDC_STATUS_BAR  (WM_USER + 1123)

static VOID _CreateStatusBar(HWND hdlg)
{
    gs_hStatBar = CreateStatusWindowW(WS_CHILD | WS_VISIBLE, NULL, hdlg, IDC_STATUS_BAR);
    int wide[5] = {0};
    int length = 0;
    //声明
    wide[0] = 280;
    //封包统计
    wide[1] = wide[0] + 360;
    //选择范围
    wide[2]= wide[1] + 160;
    //选择的数值
    wide[3] = wide[2] + 360;
    //无用的
    wide[4] = wide[3] + 256;
    SendMessage(gs_hStatBar, SB_SETPARTS, sizeof(wide) / sizeof(int), (LPARAM)(LPINT)wide); 
}

static INT_PTR _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    gsMainWnd = hdlg;
    gs_hFilter = GetDlgItem(hdlg, IDC_COM_FILTER);
    gs_hCkRegular = GetDlgItem(hdlg, IDC_CK_REGULAR);
    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    _CreateStatusBar(hdlg);

    RECT clientRect = {0};
    GetClientRect(hdlg, &clientRect);
    RECT rt1 = {0};
    GetWindowRect(gs_hFilter, &rt1);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rt1, 2);
    RECT rt2 = {0};
    GetWindowRect(gs_hStatBar, &rt2);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&rt2, 2);

    gsLogView = new CLogSyntaxView();
    int clientWidth = clientRect.right - clientRect.left;
    int clientHigh = clientRect.bottom - clientRect.top;

    int top = rt1.bottom + 8;
    int bottom = rt2.top - 8;
    gsLogView->CreateLogView(hdlg, rt1.left, rt1.bottom + 8, clientWidth - (rt1.left * 2), bottom - top);

    HWND hLogView = gsLogView->GetWindow();
    CTL_PARAMS arry[] = {
        {0, gs_hFilter, 0, 0, 1, 0},
        {0, gs_hCkRegular, 1, 0, 0, 0},
        {IDC_MAIN_SELECT, NULL, 1, 0, 0, 0},
        {IDC_BTN_CONFIG, 0, 1, 0, 0, 0},
        {0, hLogView, 0, 0, 1, 1},
        {0, gs_hStatBar, 0, 1, 1, 0}
    };
    SetCtlsCoord(hdlg, arry, RTL_NUMBER_OF(arry));

    int cw = GetSystemMetrics(SM_CXSCREEN);
    int ch = GetSystemMetrics(SM_CYSCREEN);
    int cx = (cw / 4 * 3);
    int cy = (ch / 4 * 3);

    SetWindowPos(hdlg, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE);
    CentreWindow(hdlg, NULL);

    SetWindowTextA(hdlg, "LogView-日志文件查看分析工具");
    ShowLogServView(gsMainWnd);
    return 0;
}

static INT_PTR _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_MAIN_SELECT)
    {
        ShowLogServView(hdlg);
    }
    return 0;
}

static INT_PTR _OnTimer(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnKeyDown(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnClose(HWND hdlg) {
    EndDialog(hdlg, 0);
    return 0;
}

static INT_PTR CALLBACK _MainViewProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    int ret = 0;
    switch(msg)
    {
    case  WM_INITDIALOG:
        {
            _OnInitDialog(hdlg, wp, lp);
        }
        break;
    case  WM_COMMAND:
        {
            _OnCommand(hdlg, wp, lp);
        }
        break;
    case WM_TIMER:
        {
            _OnTimer(hdlg, wp, lp);
        }
        break;
    case WM_KEYDOWN:
        {
            _OnKeyDown(hdlg, wp, lp);
        }
        break;
    case WM_CLOSE:
        {
            _OnClose(hdlg);
        }
        break;
    }
    return 0;
}

void ShowMainView() {
    DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_LOGVIEW), NULL, _MainViewProc);
}