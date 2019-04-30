#include "LogServView.h"
#include <LogLib/LogUtil.h>
#include <LogLib/LogProtocol.h>
#include "resource.h"
#include "GroupSender.h"

using namespace std;

static HANDLE gsNotifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
static HANDLE gsScanThread = NULL;

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

static void _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    CentreWindow(hdlg, GetParent(hdlg));
    SetEvent(gsNotifyEvent);
}

static void _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
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