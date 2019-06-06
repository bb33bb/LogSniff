#include <WinSock2.h>
#include "MainView.h"
#include "resource.h"
#include <LogLib/LogUtil.h>
#include <LogLib/winsize.h>
#include <LogLib/locker.h>
#include <LogLib/StrUtil.h>
#include "SyntaxHlpr/SyntaxView.h"
#include <CommCtrl.h>
#include <Shlwapi.h>
#include "LogServView.h"
#include "LogSyntaxView.h"
#include "ServTreeView.h"
#include "DbgView.h"
#include <LogLib/TextDecoder.h>

#pragma comment(lib, "comctl32.lib")

using namespace std;

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gs_hStatBar = NULL;
static HWND gs_hFilter = NULL;
static HWND gs_hCkRegular = NULL;
static HWND gsFindMode = NULL;
static CLogSyntaxView *gsLogView = NULL;
static CDbgView *gsDbgView = NULL;
static CServTreeDlg *gsServTreeView = NULL;
static CSyntaxCache *gsCurView = NULL;

static LogViewMode gsWorkMode = em_mode_debugMsg;
#define IDC_STATUS_BAR  (WM_USER + 1123)
#define TIMER_LOG_LOAD  (2010)
#define MSG_SET_FILTER  (WM_USER + 1160)

typedef LRESULT (CALLBACK *PWIN_PROC)(HWND, UINT, WPARAM, LPARAM);
static PWIN_PROC gsPfnFilterProc = NULL;

static HHOOK gsPfnKeyboardHook = NULL;
static DWORD gsLastEnterCount = 0;

static LRESULT CALLBACK _KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if ('\r' == wParam && gs_hFilter == GetFocus())
    {
        SendMessageA(gsMainWnd, MSG_SET_FILTER, 0, 0);
    }
    return CallNextHookEx(gsPfnKeyboardHook, code, wParam, lParam);
}

void SwitchWorkMode(LogViewMode mode) {
    gsWorkMode = mode;
    HWND logView = gsLogView->GetWindow();
    HWND dbgView = gsDbgView->GetWindow();
    if (em_mode_debugMsg == gsWorkMode)
    {
        ShowWindow(logView, SW_HIDE);
        ShowWindow(dbgView, SW_SHOW);
        SetWindowTextA(gsMainWnd, "LogView - 本地服务 - 调试信息");
        gsCurView = gsDbgView;
    } else if (em_mode_logFile == gsWorkMode)
    {
        ShowWindow(dbgView, SW_HIDE);
        ShowWindow(logView, SW_SHOW);
        SetWindowTextA(gsMainWnd, "LogView - 本地服务 - 文件日志");
        gsCurView = gsLogView;
    }
    SetWindowTextA(gs_hFilter, gsCurView->GetKeyword().c_str());
}

void PushLogContent(const LogInfoCache *cache) {
    mstring fileName = PathFindFileNameA(cache->mFilePath.c_str());
    fileName += " ";

    size_t pos1 = 0;
    size_t pos2 = 0;

    mstring logContent;
    const mstring &content = cache->mContent;
    while (true) {
        pos1 = content.find("\n", pos2);
        if (mstring::npos == pos1)
        {
            break;
        }

        if (pos1 > pos2)
        {
            logContent += fileName;
            logContent += content.substr(pos2, pos1 - pos2 + 1);
        }
        pos2 = pos1 + 1;
    }

    if (pos2 < content.size())
    {
        logContent += fileName;
        logContent += content.substr(pos2, content.size() - pos2);
        logContent += "\n";
    }
    gsLogView->PushToCache(logContent);
}

void PushDbgContent(const std::mstring &content) {
    gsDbgView->PushToCache(content);
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

//页面布局
static void _OnMainViewLayout() {
    RECT rt12 = {0};
    GetWindowRect(gs_hFilter, &rt12);
    SetWindowPos(gs_hFilter, 0, rt12.right, rt12.top, rt12.right - rt12.left, 10, SWP_NOMOVE | SWP_NOZORDER);

    RECT clientRect = {0};
    GetClientRect(gsMainWnd, &clientRect);
    RECT rt1 = {0};
    GetWindowRect(gs_hFilter, &rt1);
    MapWindowPoints(NULL, gsMainWnd, (LPPOINT)&rt1, 2);
    RECT rt2 = {0};
    GetWindowRect(gs_hStatBar, &rt2);
    MapWindowPoints(NULL, gsMainWnd, (LPPOINT)&rt2, 2);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHigh = clientRect.bottom - clientRect.top;
    int statusHigh = rt2.bottom - rt2.top;

    int top = rt1.bottom + 8;
    int bottom = rt2.top - 8;

    //上下左右间距
    const int spaceWidth = 3;
    gsServTreeView->MoveWindow(spaceWidth, spaceWidth, 220, clientHigh - statusHigh - spaceWidth * 2);

    RECT rtTreeView = {0};
    GetWindowRect(gsServTreeView->GetWindow(), &rtTreeView);

    int treeWidth = rtTreeView.right - rtTreeView.left;
    int treeHigh = rtTreeView.bottom - rtTreeView.top;
    int logWidth = clientWidth - (spaceWidth * 2 + treeWidth) - spaceWidth;

    int filterX = spaceWidth * 2 + treeWidth;
    int filterY = 0;
    int filterHigh = 28;
    int filterWidth = logWidth - 160;
    MoveWindow(gs_hFilter, filterX, filterY, filterWidth, filterHigh, TRUE);

    int modeX = filterX + filterWidth + 10;
    int modeY = filterY;
    int modeHigh = filterHigh;
    int modeWidth = 80;
    MoveWindow(gsFindMode, modeX, modeY, modeWidth, modeHigh, TRUE);

    HWND hLogView = gsLogView->GetWindow();
    HWND hDbgView = gsDbgView->GetWindow();
    MoveWindow(hLogView, spaceWidth * 2 + treeWidth, spaceWidth * 2 + filterHigh, logWidth, treeHigh - filterHigh - spaceWidth * 2, TRUE);
    MoveWindow(hDbgView, spaceWidth * 2 + treeWidth, spaceWidth * 2 + filterHigh, logWidth, treeHigh - filterHigh - spaceWidth * 2, TRUE);
    InvalidateRect(gsMainWnd, NULL, TRUE);
}

static DWORD WINAPI _TestSelect(LPVOID param) {
    char filePath[256];
    GetModuleFileNameA(NULL, filePath, 256);
    PathAppendA(filePath, "..\\test.log");

    int bomLen = 0;
    TextEncodeType type = CTextDecoder::GetInst()->GetFileType(filePath, bomLen);

    FILE *fp = fopen(filePath, "rb");
    fseek(fp, bomLen, SEEK_CUR);

    char buffer[4096];
    int count = 0;
    mstring dd;
    while ((count = fread(buffer, 1, 4096, fp)) > 0) {
        dd.append(buffer, count);
    }
    fclose(fp);

    gsLogView->SendMsg(SCI_SETIDENTIFIERS, STAT_KEYWORD, (LPARAM)"location");
    gsLogView->PushToCache(UtoA(dd));
    gsLogView->SendMsg(SCI_STYLESETFORE, SCE_C_WORD, RGB(255, 0, 0));

    Sleep(1000);
    return 0;
}

static INT_PTR _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    gsMainWnd = hdlg;
    gsServTreeView = new CServTreeDlg();
    gsServTreeView->CreateDlg(hdlg);
    gs_hFilter = GetDlgItem(hdlg, IDC_EDT_FILTER);
    gsFindMode = GetDlgItem(hdlg, IDC_COM_MODE);

    SendMessageA(gsFindMode, CB_INSERTSTRING, 0, (LPARAM)"过滤模式");
    SendMessageA(gsFindMode, CB_INSERTSTRING, 1, (LPARAM)"查找模式");
    SendMessageA(gsFindMode, CB_SETCURSEL, 0, 0);

    _CreateStatusBar(hdlg);
    gsLogView = new CLogSyntaxView();
    gsLogView->CreateLogView(gsMainWnd, 0, 0, 1, 1);
    gsDbgView = new CDbgView();
    gsDbgView->CreateDbgView(gsMainWnd, 0, 0, 1, 1);

    gs_hCkRegular = GetDlgItem(gsMainWnd, IDC_CK_REGULAR);
    SendMessageW(gsMainWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(gsMainWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    _OnMainViewLayout();
    SwitchWorkMode(em_mode_logFile);

    HWND hLogView = gsLogView->GetWindow();
    HWND hDbgView = gsDbgView->GetWindow();
    CTL_PARAMS arry[] = {
        {0, gs_hFilter, 0, 0, 1, 0},
        {0, gs_hCkRegular, 1, 0, 0, 0},
        {IDC_MAIN_SELECT, NULL, 1, 0, 0, 0},
        {0, hLogView, 0, 0, 1, 1},
        {0, hDbgView, 0, 0, 1, 1},
        {0, gs_hStatBar, 0, 1, 1, 0},
        {0, gsServTreeView->GetWindow(), 0, 0, 0, 1},
        {IDC_COM_MODE, 0, 1, 0, 0, 0}
    };
    SetCtlsCoord(hdlg, arry, RTL_NUMBER_OF(arry));

    int cw = GetSystemMetrics(SM_CXSCREEN);
    int ch = GetSystemMetrics(SM_CYSCREEN);
    int cx = (cw / 4 * 3);
    int cy = (ch / 4 * 3);

    SetWindowPos(hdlg, HWND_TOP, 0, 0, cx, cy, SWP_NOMOVE);
    CentreWindow(hdlg, NULL);

    //SetWindowTextA(hdlg, "LogView-日志文件查看分析工具");
    SetTimer(gsMainWnd, TIMER_LOG_LOAD, 100, NULL);
    //_TestFile();
    //gsLogView->SetHightStr("Thread");
    //gsPfnFilterProc = (PWIN_PROC)SetWindowLongPtr(gs_hFilter, GWLP_WNDPROC, (LONG_PTR)_FilterProc);
    gsPfnKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, _KeyboardProc, g_hInstance, GetCurrentThreadId());
    //CloseHandle(CreateThread(NULL, 0, _TestSelect, NULL, 0, NULL));
    return 0;
}

static INT_PTR _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_MAIN_SELECT)
    {
        ShowLogServView(hdlg);
    } else if (id == IDC_COM_MODE)
    {
        int curSel = SendMessageA(gsFindMode, CB_GETCURSEL, 0, 0);
        gsCurView->SwitchWorkMode(curSel);
    }
    return 0;
}

static INT_PTR _OnTimer(HWND hdlg, WPARAM wp, LPARAM lp) {
    if (TIMER_LOG_LOAD == wp)
    {
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
                }
                gsCurView->OnViewUpdate();
            }
            break;
        default:
            break;
    }
    return 0;
}

static INT_PTR _OnDropFiles(HWND hdlg, WPARAM wp, LPARAM lp) {
    char file[MAX_PATH] = {0x00};
    DragQueryFileA(HDROP(wp), 0, file, MAX_PATH);
    DragFinish(HDROP(wp));
    if (0x00 != file[0]) {
        mstring str(file);
        str.makelower();

        DWORD dw = GetFileAttributesA(str.c_str());
        if (INVALID_FILE_ATTRIBUTES == dw)
        {
            return 0;
        }

        if (dw & FILE_ATTRIBUTE_DIRECTORY)
        {
            CLogServMgr::GetInst()->AddPath(str);
        } else if (MonitorBase::IsLogFile(str))
        {
        }
    }
    return 0;
}

static INT_PTR _OnSetFilter() {
    char keyWord[256] = {0};

    GetWindowTextA(gs_hFilter, keyWord, 256);
    gsCurView->SetKeyword(keyWord);
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
    case MSG_SET_FILTER:
        {
            _OnSetFilter();
        }
        break;
    case WM_NOTIFY:
        {
            _OnNotify(hdlg, wp, lp);
        }
        break;
    case WM_DROPFILES:
        {
            _OnDropFiles(hdlg, wp, lp);
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