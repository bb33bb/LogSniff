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
#include "LogServMgr.h"

using namespace std;

#define MSG_SERVWND_ACTIVATED (WM_USER + 1011)

extern HINSTANCE g_hInstance;
static HWND gsMainWnd = NULL;
static HWND gsStaus = NULL;
static HWND gsListCtrl = NULL;
static HWND gsDescEdit = NULL;
static HANDLE gsScanThread = NULL;
static RLocker gsLocker;

static void _OnGetListCtrlDisplsy(NMLVDISPINFOW* plvdi)
{
    int itm = plvdi->item.iItem;
    int sub = plvdi->item.iSubItem;

    AutoLocker locker(&gsLocker);
    if (itm >= (int)CLogServMgr::GetInst()->GetServCount())
    {
        return;
    }

    static wstring sContent;
    sContent.clear();
    const LogServDesc *desc = CLogServMgr::GetInst()->GetServDesc(itm);
    bool localServ = (desc->mLogServType == em_log_serv_local);
    list<string>::const_iterator it;
    switch (sub) {
        //���
        case 0:
            sContent = FormatW(L"%d", itm);
            break;
        //Ip��ַ
        case 1:
            if (localServ)
            {
                sContent = L"���ط���";
            }
            else {
                sContent = FormatW(L"%hs", desc->mRemoteServDesc.mIpSet.begin()->c_str());
            }
            break;
        //����ʱ��
        case 2:
            if (localServ)
            {
                sContent = AtoW(desc->mLocalServDesc.mStartTime);
            } else {
                sContent = AtoW(desc->mRemoteServDesc.mStartTime);
            }
            break;
        //����״̬
        case 3:
            if (desc->mConnectStat == em_log_serv_connected)
            {
                sContent = L"������";
            } else {
                sContent = L"δ����";
            }
            break;
        default:
            break;
    }
    plvdi->item.pszText = (LPWSTR)sContent.c_str();
}

static void _InitListCtrl() {
    ListView_SetExtendedListViewStyle(gsListCtrl, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    LVCOLUMNA col;
    memset(&col, 0x00, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 50;
    col.pszText = (LPSTR)"���";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 0, (LPARAM)&col);

    col.cx = 80;
    col.pszText = (LPSTR)"Ip��ַ";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 1, (LPARAM)&col);

    col.cx = 120;
    col.pszText = (LPSTR)"����ʱ��";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 2, (LPARAM)&col);

    col.cx = 70;
    col.pszText = (LPSTR)"����״̬";
    SendMessageA(gsListCtrl, LVM_INSERTCOLUMNA, 3, (LPARAM)&col);
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

    SendMessageW(gsListCtrl, LVM_SETITEMCOUNT, CLogServMgr::GetInst()->GetServCount(), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
    int c = ListView_GetItemCount(gsListCtrl);
    ListView_SetItemState(gsListCtrl, 0, LVIS_SELECTED, LVIS_SELECTED);
    CLogServMgr::GetInst()->Refush();
}

static void _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (id == IDC_SERV_LOCAL)
    {
    }

    if (id == IDC_SERV_REFUSH)
    {
        CLogServMgr::GetInst()->Refush();
    }

    if (id == IDC_SERV_SELECT)
    {
        int sel = SendMessageA(gsListCtrl, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

        if (sel < 0)
        {
            SetWindowTextA(gsStaus, "��ѡ��һ��Ŀ��");
            return;
        }

        AutoLocker locker(&gsLocker);
        const LogServDesc *desc = CLogServMgr::GetInst()->GetServDesc(sel);

        if (CLogReceiver::GetInst()->Run(desc))
        {
            SendMessageA(gsMainWnd, WM_CLOSE, 0, 0);
        }
    }
}

static mstring _GetLogServDesc(const LogServDesc &serv) {
    mstring result;

    result = "����ϵͳ\r\n";
    if (em_log_serv_local == serv.mLogServType)
    {
        const LocalServDesc &local = serv.mLocalServDesc;
        result += local.mSystem;
        result += "\r\n\r\n";

        result += "��־·��\r\n";
        for (list<mstring>::const_iterator it = local.mPathSet.begin() ; it != local.mPathSet.end() ; it++)
        {
            result += *it;
            result += "\r\n";
        }

        if (local.mPathSet.empty())
        {
            result += "δ����\r\n";
        }
    } else if (em_log_serv_remote == serv.mLogServType)
    {
        const RemoteServDesc &remote = serv.mRemoteServDesc;
        result += remote.mSystem;
        result += "\r\n\r\n";

        result += "��־·��\r\n";
        for (list<string>::const_iterator it = remote.mPathSet.begin() ; it != remote.mPathSet.end() ; it++)
        {
            result += *it;
            result += "\r\n";
        }

        if (remote.mPathSet.empty())
        {
            result += "δ����\r\n";
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
            if (viewer->iItem >= 0 && viewer->iItem < (int)CLogServMgr::GetInst()->GetServCount())
            {
                LogServDesc desc = *CLogServMgr::GetInst()->GetServDesc(viewer->iItem);
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