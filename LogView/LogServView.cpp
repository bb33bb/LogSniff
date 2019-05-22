#include <WinSock2.h>
#include "LogServView.h"
#include <LogLib/LogUtil.h>
#include <LogLib/LogProtocol.h>
#include <LogLib/locker.h>
#include <LogLib/StrUtil.h>
#include <LogLib/SqliteOperator.h>
#include <CommCtrl.h>
#include "MonitorBase.h"
#include "resource.h"
#include "GroupSender.h"
#include "LogReceiver.h"

using namespace std;

#define MSG_SERVWND_ACTIVATED (WM_USER + 1011)

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gsStaus = NULL;
static HWND gsListCtrl = NULL;
static HWND gsDescEdit = NULL;
static HANDLE gsNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
static HANDLE gsScanThread = NULL;
static vector<LogServDesc> gsLogServSet;
static RLocker gsLocker;
static int gsCurSel = -1;
static mstring gsCfgDbPath;

static void _OnLogServDesc(const LpServDesc &desc) {
    AutoLocker locker(&gsLocker);
    int i = 0;
    for (i = 0 ; i < (int)gsLogServSet.size() ; i++)
    {
        LogServDesc &cache = gsLogServSet[i];
        if (em_log_serv_remote == cache.mLogServType && desc.mIpSet == desc.mIpSet)
        {
            cache.mRemoteServDesc = desc;
            PostMessageW(gsListCtrl, LVM_REDRAWITEMS, i, i + 1);
            return;
        }
    }

    LogServDesc newDesc;
    newDesc.mLogServType = em_log_serv_remote;
    newDesc.mRemoteServDesc = desc;
    gsLogServSet.push_back(newDesc);
    PostMessageW(gsListCtrl, LVM_SETITEMCOUNT, gsLogServSet.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    PostMessageW(gsListCtrl, LVM_REDRAWITEMS, i, i + 1);
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
    const LogServDesc &desc = gsLogServSet[itm];
    bool localServ = (desc.mLogServType == em_log_serv_local);
    list<string>::const_iterator it;
    switch (sub) {
        //序号
        case 0:
            sContent = FormatW(L"%d", itm);
            break;
        //Ip地址
        case 1:
            if (localServ)
            {
                sContent = L"本地服务";
            }
            else {
                sContent = FormatW(L"%hs", desc.mRemoteServDesc.mIpSet.begin()->c_str());
            }
            break;
        //启动时间
        case 2:
            if (localServ)
            {
                sContent = AtoW(desc.mLocalServDesc.mStartTime);
            } else {
                sContent = AtoW(desc.mRemoteServDesc.mStartTime);
            }
            break;
        //连接状态
        case 3:
            if (desc.mConnectStat == em_log_serv_connected)
            {
                sContent = L"已连接";
            } else {
                sContent = L"未连接";
            }
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

        int count = 1;
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

    col.cx = 120;
    col.pszText = (LPSTR)"启动时间";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 2, (LPARAM)&col);

    col.cx = 70;
    col.pszText = (LPSTR)"连接状态";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 3, (LPARAM)&col);
}

static void _InitCache() {
    SqliteOperator opt(gsCfgDbPath);
    srand(GetTickCount());
    string unique = FormatA(
        "%02x%02x%02x%02x",
        rand() % 0xff,
        rand() % 0xff,
        rand() % 0xff,
        rand() % 0xff
        );

    LocalServDesc localServ;
    localServ.mUnique = unique;
    mstring sql = FormatA(
        "INSERT INTO tSessionDesc (servUnique, type, system, ipSet, pathSet, time)VALUES('%hs', %d, '%hs', '本地服务', '', '%hs')",
        unique.c_str(),
        em_log_serv_local,
        localServ.mSystem.c_str(),
        localServ.mStartTime.c_str()
        );
    opt.Insert(sql);

    LogServDesc tmp;
    tmp.mLogServType = em_log_serv_local;
    tmp.mConnectStat = em_log_serv_disconnected;
    tmp.mLocalServDesc = localServ;
    gsLogServSet.push_back(tmp);
}

static void _LoadCacheFromDb() {
    SqliteOperator opt(gsCfgDbPath);
    SqliteResult &result = opt.Select("select * from tSessionDesc");

    for (SqliteIterator it = result.begin() ; it != result.end() ; ++it)
    {
        LogServDesc tmp;
        tmp.mLogServType = (LogServType)atoi(it.GetValue("type").c_str());

        string pathSet;
        list<string> strSet;
        if (em_log_serv_local == tmp.mLogServType)
        {
            LocalServDesc tmp1;
            tmp1.mUnique = it.GetValue("servUnique");
            tmp1.mSystem = it.GetValue("system");
            pathSet = it.GetValue("pathSet");
            list<mstring> ss = SplitStrA(pathSet, ";");

            for (list<mstring>::const_iterator it = ss.begin() ; it != ss.end() ; it++) 
            {
                tmp1.mPathSet.push_back(*it);
            }
            tmp.mLocalServDesc = tmp1;
        } else if (em_log_serv_remote == tmp.mLogServType)
        {
            RemoteServDesc tmp2;
            tmp2.mUnique = it.GetValue("servUnique");
            tmp2.mSystem = it.GetValue("system");
            pathSet = it.GetValue("pathSet");
            list<mstring> ss = SplitStrA(pathSet, ";");

            for (list<mstring>::const_iterator it = ss.begin() ; it != ss.end() ; it++)
            {
                tmp2.mPathSet.push_back(*it);
            }
            tmp.mRemoteServDesc = tmp2;
        }
        gsLogServSet.push_back(tmp);
    }
}

static void _InitConfig() {
    extern mstring gCfgPath;

    if (gsCfgDbPath.empty())
    {
        gsCfgDbPath = gCfgPath;
        gsCfgDbPath += "\\config.db";
    }

    SqliteOperator opt(gsCfgDbPath);
    opt.Exec("create table if not exists tSessionDesc (id INTEGER PRIMARY KEY, servUnique CHAR(32), type INTEGER, system TEXT, ipSet TEXT, pathSet TEXT, time CHAR(32))");
    SqliteResult &result = opt.Select("select count(*) from tSessionDesc");

    int count = atoi(result.begin().GetValue("count(*)").c_str());
    opt.Close();
    if (0 == count) {
        _InitCache();
    } else {
       _LoadCacheFromDb();
    }
}

static void _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    gsMainWnd = hdlg;
    gsStaus = GetDlgItem(hdlg, IDC_SERV_STATUS);
    gsListCtrl = GetDlgItem(hdlg, IDC_SERV_LIST);
    gsDescEdit = GetDlgItem(hdlg, IDC_EDT_DESC);

    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));
    SendMessageW(hdlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_MAIN)));

    _InitListCtrl();
    CentreWindow(hdlg, GetParent(hdlg));

    if (gsCfgDbPath.empty())
    {
        _InitConfig();
    }

    if (!gsLogServSet.empty())
    {
        SendMessageW(gsListCtrl, LVM_SETITEMCOUNT, gsLogServSet.size(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    }

    int c = ListView_GetItemCount(gsListCtrl);
    ListView_SetItemState(gsListCtrl, 0, LVIS_SELECTED, LVIS_SELECTED);

    if (!gsScanThread) 
    {
        gsScanThread = CreateThread(NULL, 0, _ScanThread, NULL, 0, NULL);
    }
    SetEvent(gsNotifyEvent);
}

static void _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_SERV_LOCAL)
    {
    }

    if (id == IDC_SERV_REFUSH)
    {
        SetEvent(gsNotifyEvent);
    }

    if (id == IDC_SERV_SELECT)
    {
        int sel = SendMessageA(gsListCtrl, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

        if (sel < 0)
        {
            SetWindowTextA(gsStaus, "先选择一个目标");
            return;
        }

        AutoLocker locker(&gsLocker);
        const LogServDesc &desc = gsLogServSet[sel];

        /*
        if (CLogReceiver::GetInst()->Start(*desc.mIpSet.begin()))
        {
            SendMessageA(gsMainWnd, WM_CLOSE, 0, 0);
        }
        */
    }
}

static mstring _GetLogServDesc(const LogServDesc &serv) {
    mstring result;

    result = "操作系统\r\n";
    if (em_log_serv_local == serv.mLogServType)
    {
        const LocalServDesc &local = serv.mLocalServDesc;
        result += local.mSystem;
        result += "\r\n\r\n";

        result += "日志路径\r\n";
        for (list<mstring>::const_iterator it = local.mPathSet.begin() ; it != local.mPathSet.end() ; it++)
        {
            result += *it;
            result += "\r\n";
        }

        if (local.mPathSet.empty())
        {
            result += "未设置\r\n";
        }
    } else if (em_log_serv_remote == serv.mLogServType)
    {
        const RemoteServDesc &remote = serv.mRemoteServDesc;
        result += remote.mSystem;
        result += "\r\n\r\n";

        result += "日志路径\r\n";
        for (list<string>::const_iterator it = remote.mPathSet.begin() ; it != remote.mPathSet.end() ; it++)
        {
            result += *it;
            result += "\r\n";
        }

        if (remote.mPathSet.empty())
        {
            result += "未设置\r\n";
        }
    }
    return result;
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
    case  LVN_ITEMCHANGED:
        {
            AutoLocker locker(&gsLocker);
            NMLISTVIEW *viewer = (NMLISTVIEW *)lp;
            if (viewer->iItem >= 0 && viewer->iItem < (int)gsLogServSet.size())
            {
                LogServDesc desc = gsLogServSet[viewer->iItem];
                mstring str = _GetLogServDesc(desc);
                SetWindowTextA(gsDescEdit, str.c_str());
            }
        }
        break;
    case  LVN_ITEMCHANGING:
        {
        }
        break;
    default:
        break;
    }
}

static void _OnClose(HWND hdlg) {
    EndDialog(hdlg, 0);
    gsMainWnd = NULL;
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
    case MSG_SERVWND_ACTIVATED:
        {
            CentreWindow(gsMainWnd, GetParent(gsMainWnd));
            SetForegroundWindow(gsMainWnd);
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
    if (IsWindow(gsMainWnd))
    {
        SendMessageA(gsMainWnd, MSG_SERVWND_ACTIVATED, 0, 0);
        return;
    }
    CreateDialogW(g_hInstance, MAKEINTRESOURCEW(IDD_LOGSERV), hParent, _LogServViewProc);
}