#include <WinSock2.h>
#include "MainView.h"
#include "../resource.h"
#include <LogLib/LogUtil.h>
#include <LogLib/winsize.h>
#include <LogLib/locker.h>
#include <LogLib/StrUtil.h>
#include "../SyntaxHlpr/SyntaxTextView.h"
#include <CommCtrl.h>
#include <Shlwapi.h>
#include "LogServView.h"
#include "LogSyntaxView.h"
#include "ServTreeView.h"
#include <LogLib/TextDecoder.h>
#include "LogFrameBase.h"
#include "../GlobalDef.h"
#include "LocalFrame.h"
#include "MainView.h"
#include "AboutDlg.h"

#pragma comment(lib, "comctl32.lib")

using namespace std;

//快捷菜单
#define MENU_ID_TOPMOST         (WM_USER + 1200)
#define MENU_NAME_TOPMOST       ("窗口置顶 Ctrl+P")

#define MENU_ID_AUTO_SCROLL     (WM_USER + 1205)
#define MENU_NAME_AUTO_SCROLL   ("自动滚屏 Ctrl+B")

#define MENU_ID_PAUSE           (WM_USER + 1206)
#define MENU_NAME_PAUSE         ("暂停嗅探 Ctrl+U")

#define MENU_ID_CLEAR           (WM_USER + 1300)
#define MENU_NAME_CLEAR         ("清空页面 Ctrl+X")

#define MENU_ID_FIND            (WM_USER + 1301)
#define MENU_NAME_FIND          ("查找数据 Ctrl+F")

#define MENU_ID_SELECT_ALL      (WM_USER + 1302)
#define MENU_NAME_SELECT_ALL    ("全部选择 Ctrl+A")

#define MENU_ID_COPY            (WM_USER + 1303)
#define MENU_NAME_COPY          ("复制数据 Ctrl+C")

#define MENU_ID_EXPORT          (WM_USER + 1304)
#define MENU_NAME_EXPORT        ("存成文件 Ctrl+E")

#define MENU_ID_SET             (WM_USER + 1305)
#define MENU_NAME_SET           ("服务设置 Ctrl+S")

#define MENU_ID_ABOUT           (WM_USER + 1321)
#define MENU_NAME_ABOUT         ("关于LogSniff")

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gs_hStatBar = NULL;
static CServTreeDlg *gsServTreeView = NULL;
static CLogFrameBase *gsCurView = NULL;

//定时器更新状态，防止刷新过于频繁
static DWORD gsStatusCur = 0;
static DWORD gsStatusLast = 0;

//日志展示框架集合
static map<mstring, CLogFrameBase *> gsLogFrameSet;

#define IDC_STATUS_BAR          (WM_USER + 1123)
#define TIMER_LOG_LOAD          (2010)
#define TIMER_UPDATE_STATBAR    (2011)
#define MSG_ACTIVATE_VIEW   (WM_USER + 1163)

#define EVENT_SNIFFER_MUTEX     ("Global\\b036b8da-d9b7-4a66-9ba0-abcf24238")
HANDLE gsNotifyEvent = NULL;

typedef LRESULT (CALLBACK *PWIN_PROC)(HWND, UINT, WPARAM, LPARAM);
static PWIN_PROC gsPfnFilterProc = NULL;

static HHOOK gsPfnKeyboardHook = NULL;
static DWORD gsLastEnterCount = 0;
static HHOOK gsPfnMouseHook = NULL;
static DWORD gsCtrlLastCount = GetTickCount();

static LRESULT CALLBACK _KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    DWORD curCount = GetTickCount();
    if (VK_CONTROL == wParam) {
        gsCtrlLastCount = curCount;
        return CallNextHookEx(gsPfnKeyboardHook, code, wParam, lParam);
    }

    if ((curCount - gsCtrlLastCount < 100) || (GetKeyState(VK_CONTROL) & (1 << 16)) && (lParam & (1 << 30)))
    {
        //窗口置顶
        if ('P' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_TOPMOST, 0);
        }
        //自动滚屏
        else if ('B' == wParam) {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_AUTO_SCROLL, 0);
        }
        //暂停嗅探
        else if ('U' == wParam) {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_PAUSE, 0);
        }
        //清空页面
        else if ('X' == wParam) {
            dp("clear view");
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_CLEAR, 0);
        }
        //查找数据
        else if ('F' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_FIND, 0);
        }
        //全选
        else if ('A' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_SELECT_ALL, 0);
        }
        //复制
        else if ('C' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_COPY, 0);
        }
        //导出
        else if ('E' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_EXPORT, 0);
        }
        //服务设置
        else if ('S' == wParam)
        {
            PostMessageA(gsMainWnd, WM_COMMAND, MENU_ID_SET, 0);
        }
    }
    return CallNextHookEx(gsPfnKeyboardHook, code, wParam, lParam);
}

void OnPopupMenu() {
    POINT pt = {0};
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    if (gShowConfig.mTopMost)
    {
        AppendMenuA(menu, MF_ENABLED | MF_CHECKED, MENU_ID_TOPMOST, MENU_NAME_TOPMOST);
    } else {
        AppendMenuA(menu, MF_ENABLED, MENU_ID_TOPMOST, MENU_NAME_TOPMOST);
    }

    if (gShowConfig.mAutoScroll)
    {
        AppendMenuA(menu, MF_ENABLED | MF_CHECKED, MENU_ID_AUTO_SCROLL, MENU_NAME_AUTO_SCROLL);
    } else {
        AppendMenuA(menu, MF_ENABLED, MENU_ID_AUTO_SCROLL, MENU_NAME_AUTO_SCROLL);
    }

    if (gShowConfig.mPause)
    {
        AppendMenuA(menu, MF_ENABLED | MF_CHECKED, MENU_ID_PAUSE, MENU_NAME_PAUSE);
    } else {
        AppendMenuA(menu, MF_ENABLED, MENU_ID_PAUSE, MENU_NAME_PAUSE);
    }

    AppendMenuA(menu, MF_ENABLED, MENU_ID_CLEAR, MENU_NAME_CLEAR);
    AppendMenuA(menu, MF_ENABLED, MENU_ID_FIND, MENU_NAME_FIND);
    AppendMenuA(menu, MF_ENABLED, MENU_ID_SELECT_ALL, MENU_NAME_SELECT_ALL);

    AppendMenuA(menu, MF_MENUBARBREAK, 0, 0);

    AppendMenuA(menu, MF_ENABLED, MENU_ID_COPY, MENU_NAME_COPY);
    AppendMenuA(menu, MF_ENABLED, MENU_ID_EXPORT, MENU_NAME_EXPORT);

    AppendMenuA(menu, MF_MENUBARBREAK, 0, 0);
    AppendMenuA(menu, MF_ENABLED, MENU_ID_SET, MENU_NAME_SET);

    AppendMenuA(menu, MF_MENUBARBREAK, 0, 0);
    AppendMenuA(menu, MF_ENABLED, MENU_ID_ABOUT, MENU_NAME_ABOUT);
    TrackPopupMenu(menu, TPM_CENTERALIGN, pt.x, pt.y, 0, gsMainWnd, 0);
    DestroyMenu(menu);
}

static void _SetStatusText(int index, const mstring &text) {
    SendMessageA(gs_hStatBar, SB_SETTEXTA, index, (LPARAM)text.c_str());
}

void UpdateStatusBar() {
    gsStatusCur++;
}

static void _UpdateStatusBarReally() {
    char self[512];
    GetModuleFileNameA(NULL, self, 512);
    mstring ver;
    GetFileVersion(self, ver);
    mstring content0 = FormatA("LogSniff  版本:%hs", ver.c_str());
    _SetStatusText(0, content0.c_str());

    mstring content1;
    const LogServDesc *desc = CLogServMgr::GetInst()->GetCurServ();
    content1 = "服务类型:";
    if (desc->mLogServType == em_log_serv_local)
    {
        content1 += "  本地服务";
    } else {
        content1 += "  远端服务";
    }

    _SetStatusText(1, content1.c_str());
    _SetStatusText(3, "联系作者  QQ: 412776488 邮箱: lougdhr@126.com");
}

void PushLogContent(const LogInfoCache *cache) {
    if (gShowConfig.mPause)
    {
        return;
    }

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

    gsCurView->OnFileLog(logContent);
}

void PushDbgContent(const std::mstring &content) {
    if (gShowConfig.mPause)
    {
        return;
    }

    gsCurView->OnDbgLog(content);
}

static VOID _CreateStatusBar(HWND hdlg)
{
    gs_hStatBar = CreateStatusWindowW(WS_CHILD | WS_VISIBLE, NULL, hdlg, IDC_STATUS_BAR);
    int wide[5] = {0};
    int length = 0;
    wide[0] = 160;
    wide[1] = wide[0] + 360;
    wide[2]= wide[1] + 220;
    wide[3] = wide[2] + 480;
    wide[4] = wide[3] + 256;
    SendMessage(gs_hStatBar, SB_SETPARTS, sizeof(wide) / sizeof(int), (LPARAM)(LPINT)wide); 
}

//页面布局
static void _OnMainViewLayout() {
    RECT clientRect = {0};
    GetClientRect(gsMainWnd, &clientRect);
    RECT rt1 = {0};
    RECT rt2 = {0};
    GetWindowRect(gs_hStatBar, &rt2);
    MapWindowPoints(NULL, gsMainWnd, (LPPOINT)&rt2, 2);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHigh = clientRect.bottom - clientRect.top;
    int statusHigh = rt2.bottom - rt2.top;

    //上下左右间距
    const int spaceWidth = 3;
    gsServTreeView->MoveWindow(spaceWidth, spaceWidth, 160, clientHigh - statusHigh - spaceWidth * 2);

    RECT rtTreeView = {0};
    GetWindowRect(gsServTreeView->GetWindow(), &rtTreeView);
    MapWindowPoints(NULL, gsServTreeView->GetWindow(), (LPPOINT)&rtTreeView, 2);

    int treeWidth = rtTreeView.right - rtTreeView.left;
    int treeHigh = rtTreeView.bottom - rtTreeView.top;
    int logWidth = clientWidth - (spaceWidth * 2 + treeWidth) - spaceWidth;

    HWND hLogFrame = gsCurView->GetHandle();
    MoveWindow(hLogFrame, spaceWidth * 2 + treeWidth, rtTreeView.top, logWidth, treeHigh + 4, TRUE);
    /*
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

    HWND hLogFrame = gsCurView->GetHandle();
    MoveWindow(hLogFrame, spaceWidth * 2 + treeWidth, spaceWidth * 2 + filterHigh, logWidth, treeHigh - filterHigh - spaceWidth * 2, TRUE);
    */
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

    //gsLogView->PushLog(UtoA(dd));
    //gsLogView->SendMsg(SCI_STYLESETFORE, SCE_C_WORD, RGB(255, 0, 0));

    Sleep(1000);
    return 0;
}

static DWORD WINAPI _NoitfyThread(LPVOID param) {
    while (true) {
        WaitForSingleObject(gsNotifyEvent, INFINITE);

        SendMessage(gsMainWnd, MSG_ACTIVATE_VIEW, 0, 0);
    }
    return 0;
}

//加载所有的日志服务
static void _LoadLogServ() {
    size_t s = CLogServMgr::GetInst()->GetServCount();
    for (size_t i = 0 ; i < s ; i++)
    {
        const LogServDesc *desc = CLogServMgr::GetInst()->GetServDesc(i);

        CLogFrameBase *ptr = NULL;
        if (desc->mLogServType == em_log_serv_local) {
            ptr = new CLocalLogFrame();
            gsCurView = ptr;
        } else {
        }

        ptr->InitLogFrame(desc);
        gsLogFrameSet[desc->mUnique] = ptr;
    }
}

static INT_PTR _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    gsMainWnd = hdlg;

    _LoadLogServ();
    gsCurView->CreateDlg(gsMainWnd, IDD_LOCAL_FRAME);
    gsNotifyEvent = CreateEventA(NULL, FALSE, FALSE, EVENT_SNIFFER_MUTEX);
    CloseHandle(CreateThread(NULL, 0, _NoitfyThread, NULL, 0, NULL));

    gsServTreeView = new CServTreeDlg();
    gsServTreeView->CreateDlg(hdlg);

    _CreateStatusBar(hdlg);
    SendMessageW(gsMainWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(gsMainWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    _OnMainViewLayout();

    HWND hLogFrame = gsCurView->GetHandle();
    CTL_PARAMS arry[] = {
        {IDC_MAIN_SELECT, NULL, 1, 0, 0, 0},
        {0, hLogFrame, 0, 0, 1, 1},
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

    gsPfnKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, _KeyboardProc, g_hInstance, GetCurrentThreadId());

    class CUpdateStatus {
    public:
        static VOID CALLBACK TimerProc(HWND hwnd, UINT msg, UINT_PTR id, DWORD time) {
            if (gsStatusLast != gsStatusCur)
            {
                gsStatusLast = gsStatusCur;
            }
            _UpdateStatusBarReally();
        }
    };
    SetTimer(gsMainWnd, TIMER_UPDATE_STATBAR, 100, CUpdateStatus::TimerProc);
    UpdateStatusBar();
    return 0;
}

static INT_PTR _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_MAIN_SELECT)
    {
        ShowLogServView(hdlg);
    } else if (id == MENU_ID_TOPMOST)
    {
        gShowConfig.mTopMost = !gShowConfig.mTopMost;

        if (gShowConfig.mTopMost)
        {
            SetWindowPos(gsMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        } else {
            SetWindowPos(gsMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        }
    } else if (id == MENU_ID_AUTO_SCROLL)
    {
        gShowConfig.mAutoScroll = !gShowConfig.mAutoScroll;
        gsCurView->UpdateConfig();
    } else if (id == MENU_ID_PAUSE)
    {
        gShowConfig.mPause = !gShowConfig.mPause;
    } else if (id == MENU_ID_CLEAR)
    {
        gsCurView->ClearView();
        UpdateStatusBar();
    } else if (id == MENU_ID_FIND)
    {
    } else if (id == MENU_ID_SELECT_ALL)
    {
    } else if (id == MENU_ID_COPY)
    {
    } else if (id == MENU_ID_EXPORT)
    {
    } else if (id == MENU_ID_SET)
    {
    } else if (id == MENU_ID_ABOUT)
    {
        CAboutDlg dlg;
        dlg.DoModule(gsMainWnd, IDD_ABOUT);
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
                }
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

static INT_PTR _OnClose(HWND hdlg) {
    EndDialog(hdlg, 0);
    return 0;
}

static void _ActiveWindow() {
    if(IsIconic(gsMainWnd))
    {
        SendMessageA(gsMainWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
    }

    if (!IsZoomed(gsMainWnd))
    {
        CentreWindow(gsMainWnd, NULL);
        RECT rect;
        GetWindowRect(gsMainWnd, &rect);
        if (rect.left < 0 || rect.top < 0)
        {
            SetWindowPos(gsMainWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    if (!gShowConfig.mTopMost)
    {
        SetForegroundWindow(gsMainWnd);
        SetWindowPos(gsMainWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(gsMainWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

bool IsLogSniffRunning() {
    HANDLE h = OpenEventA(EVENT_MODIFY_STATE, FALSE, EVENT_SNIFFER_MUTEX);

    if (h)
    {
        CloseHandle(h);
        return true;
    }
    return false;
}

void NotifyLogSniff() {
    HANDLE h = OpenEventA(EVENT_MODIFY_STATE, FALSE, EVENT_SNIFFER_MUTEX);

    if (h)
    {
        SetEvent(h);
        CloseHandle(h);
    }
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
    case MSG_ACTIVATE_VIEW:
        {
            _ActiveWindow();
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