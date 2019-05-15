#include <WinSock2.h>
#include "MainView.h"
#include "resource.h"
#include <LogLib/LogUtil.h>
#include <LogLib/winsize.h>
#include <LogLib/locker.h>
#include <LogLib/StrUtil.h>
#include "SyntaxHlpr/SyntaxView.h"
#include <CommCtrl.h>
#include "LogServView.h"
#include "LogSyntaxView.h"

#pragma comment(lib, "comctl32.lib")

using namespace std;

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gs_hStatBar = NULL;
static HWND gs_hFilter = NULL;
static HWND gs_hCkRegular = NULL;
static CLogSyntaxView *gsLogView = NULL;
//log content cache
static RLocker *gsLogLocker = NULL;
static string gsLogContentCache;

#define IDC_STATUS_BAR  (WM_USER + 1123)
#define TIMER_LOG_LOAD  (2010)

void PushLogContent(const LogInfoCache *cache) {
    AutoLocker locker(gsLogLocker);
    gsLogContentCache += cache->mContent;
    gsLogContentCache += "\n";
}

static VOID _CreateStatusBar(HWND hdlg)
{
    gs_hStatBar = CreateStatusWindowW(WS_CHILD | WS_VISIBLE, NULL, hdlg, IDC_STATUS_BAR);
    int wide[5] = {0};
    int length = 0;
    wide[0] = 280;
    wide[1] = wide[0] + 360;
    wide[2]= wide[1] + 160;
    wide[3] = wide[2] + 360;
    wide[4] = wide[3] + 256;
    SendMessage(gs_hStatBar, SB_SETPARTS, sizeof(wide) / sizeof(int), (LPARAM)(LPINT)wide); 
}

static void _TestFile() {
    const int bufSize = 1024 * 1024 * 16;
    static char *buffer = new char[bufSize];

    FILE *fp = fopen("E:\\gdemm\\2018-03-20 030539.log", "rb+");
    int c = fread(buffer, 1, bufSize, fp);
    buffer[c] = 0;
    gsLogView->AppendText(LABEL_LOG_CONTENT, UtoA(buffer));
    fclose(fp);
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

    gsLogLocker = new RLocker();
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

    SetWindowPos(hdlg, HWND_TOP, 0, 0, 300, 400, SWP_NOMOVE);
    CentreWindow(hdlg, NULL);

    SetWindowTextA(hdlg, "LogView-日志文件查看分析工具");
    SetTimer(gsMainWnd, TIMER_LOG_LOAD, 100, NULL);

    _TestFile();
    gsLogView->SetHightStr("Thread");
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
    if (TIMER_LOG_LOAD == wp)
    {
        AutoLocker locker(gsLogLocker);

        //双重缓存,避免频繁窗口消息导致窗体卡顿
        if (gsLogContentCache.size() > 0)
        {
            gsLogView->AppendText(LABEL_LOG_CONTENT, gsLogContentCache);
            gsLogContentCache.clear();
        }
    }
    return 0;
}

static INT_PTR _OnKeyDown(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnNotify(HWND hdlg, WPARAM wp, LPARAM lp) {
    NotifyHeader *header = (NotifyHeader *)lp;
    SCNotification *notify = (SCNotification *)lp;

    switch (header->code) {
        case SCN_UPDATEUI:
            {
                if (notify->updated & SC_UPDATE_SELECTION)
                {
                    size_t pos1 = gsLogView->SendMsg(SCI_GETSELECTIONSTART, 0, 0);
                    size_t pos2 = gsLogView->SendMsg(SCI_GETSELECTIONEND, 0, 0);
                    dp("select %d-%d", pos1, pos2);
                }
            }
        default:
            break;
    }
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
    case WM_NOTIFY:
        {
            _OnNotify(hdlg, wp, lp);
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