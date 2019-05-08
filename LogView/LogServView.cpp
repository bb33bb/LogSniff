#include "LogServView.h"
#include <LogLib/LogUtil.h>
#include <LogLib/LogProtocol.h>
#include <LogLib/locker.h>
#include <LogLib/StrUtil.h>
#include <CommCtrl.h>
#include "resource.h"
#include "GroupSender.h"

using namespace std;

static HWND gsListCtrl = NULL;
static HANDLE gsNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
static HANDLE gsScanThread = NULL;
static vector<LpServDesc> gsLogServSet;
static RLocker gsLocker;

static void _OnLogServDesc(const LpServDesc &desc) {
    AutoLocker locker(&gsLocker);
    int i = 0;
    for (i = 0 ; i < (int)gsLogServSet.size() ; i++)
    {
        if (desc.mIpSet == desc.mIpSet)
        {
            gsLogServSet[i] = desc;
            PostMessageA(gsListCtrl, LVM_REDRAWITEMS, i, i + 1);
            return;
        }
    }

    gsLogServSet.push_back(desc);
    PostMessageA(gsListCtrl, LVM_SETITEMCOUNT, gsLogServSet.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    PostMessageA(gsListCtrl, LVM_REDRAWITEMS, i, i + 1);
}

static void _OnGetListCtrlDisplsy(NMLVDISPINFOW* plvdi)
{
    int itm = plvdi->item.iItem;
    int sub = plvdi->item.iSubItem;

    AutoLocker locker(&gsLocker);
    if (itm >= (int)gsLogServSet.size())
    {
        return;
    }

    static wstring sContent;
    sContent.clear();
    LpServDesc desc = gsLogServSet[itm];
    list<string>::const_iterator it;
    switch (sub) {
        //序号
        case 0:
            sContent = FormatW(L"%d", itm);
            break;
        //Ip地址
        case 1:
            sContent = FormatW(L"%hs", desc.mIpSet.begin()->c_str());
            break;
        //操作系统
        case 2:
            sContent = FormatW(L"%hs", desc.mSystem.c_str());
            break;
        //启动时间
        case 3:
            sContent = FormatW(L"%hs", desc.mStartTime.c_str());
            break;
        //日志路径
        case 4:
            {
                for (it = desc.mPathSet.begin() ; it != desc.mPathSet.end() ; it++)
                {
                    sContent += AtoW(it->c_str());
                    sContent += L";";
                }
            }
            break;
        //描述
        case 5:
            sContent = FormatW(L"%hs", desc.mUserDesc.c_str());
            break;
        default:
            break;
    }
    plvdi->item.pszText = (LPWSTR)sContent.c_str();
}

static void OnGroupRecv(const string &recvStr, string &respStr) {
    Value content;
    Reader().parse(recvStr, content);

    if (content.type() != objectValue || content["cmd"].type() != stringValue)
    {
        return;
    }

    string cmd = content["cmd"].asString();
    if (cmd == GROUT_MSG_DESC)
    {
        LpServDesc desc;
        CLogProtocol::GetInst()->DecodeDesc(recvStr, desc);
        _OnLogServDesc(desc);
    }
}

static DWORD WINAPI _ScanThread(LPVOID param) {
    static CGroupSender *sSender = NULL;
    static string sScanStr;

    if (NULL == sSender)
    {
        sSender = new CGroupSender();
        sSender->Init(OnGroupRecv);
        sSender->Bind(GROUP_ADDR, GROUP_PORT);
        sScanStr = "{\"cmd\":\"scan\"}";
    }

    while (true) {
        WaitForSingleObject(gsNotifyEvent, INFINITE);

        int count = 5;
        while (count-- > 0) {
            sSender->Send(sScanStr);

            Sleep(500);
        }
    }
    return 0;
}

static void _InitListCtrl() {
    ListView_SetExtendedListViewStyle(gsListCtrl, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    LVCOLUMNA col;
    memset(&col, 0x00, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 50;
    col.pszText = (LPSTR)"序号";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 0, (LPARAM)&col);

    col.cx = 80;
    col.pszText = (LPSTR)"Ip地址";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 1, (LPARAM)&col);

    col.cx = 160;
    col.pszText = (LPSTR)"操作系统";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 2, (LPARAM)&col);

    col.cx = 120;
    col.pszText = (LPSTR)"启动时间";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 3, (LPARAM)&col);

    col.cx = 220;
    col.pszText = (LPSTR)"日志路径";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 4, (LPARAM)&col);

    col.cx = 180;
    col.pszText = (LPSTR)"描述";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 5, (LPARAM)&col);
}

static void _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    gsListCtrl = GetDlgItem(hdlg, IDC_SERV_LIST);

    extern HINSTANCE g_hInstance;
    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    _InitListCtrl();
    CentreWindow(hdlg, GetParent(hdlg));
    SetEvent(gsNotifyEvent);
}

static void _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_SERV_REFUSH)
    {
        SetEvent(gsNotifyEvent);
    }
}

static void _OnNotify(HWND hdlg, WPARAM wp, LPARAM lp)
{
    LPNMHDR msg = (LPNMHDR)lp;
    if (!msg || gsListCtrl != msg->hwndFrom)
    {
        return;
    }

    NMLVDISPINFOW *plvdi;
    switch (((LPNMHDR) lp)->code)
    {
    case LVN_GETDISPINFOW:
        {
            plvdi = (NMLVDISPINFOW *)lp;
            _OnGetListCtrlDisplsy(plvdi);
        }
        break;
    default:
        break;
    }
}

static void _OnClose(HWND hdlg) {
    EndDialog(hdlg, 0);
}

static INT_PTR CALLBACK _LogServViewProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
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

void ShowLogServView(HWND hParent) {
    if (!gsScanThread)
    {
        gsScanThread = CreateThread(NULL, 0, _ScanThread, NULL, 0, NULL);
    }
    DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_LOGSERV), hParent, _LogServViewProc);
}