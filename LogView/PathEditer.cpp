#include <WinSock2.h>
#include <Windows.h>
#include "LogReceiver.h"
#include "PathEditer.h"
#include "resource.h"

static INT_PTR _OnInitDialog(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnDropFiles(HWND hdlg, WPARAM wp, LPARAM lp) {
    char file[MAX_PATH] = {0x00};
    DragQueryFileA(HDROP(wp), 0, file, MAX_PATH);
    if (0x00 != file[0]) {
    }
    DragFinish(HDROP(wp));

    return 0;
}

static INT_PTR _OnCommand(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnNotify(HWND hdlg, WPARAM wp, LPARAM lp) {
    return 0;
}

static INT_PTR _OnClose(HWND hdlg) {
    EndDialog(hdlg, 0);
    return 0;
}

static INT_PTR CALLBACK _PathEditerProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
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

void ShowPathEditor(HWND hParent) {
    DialogBoxA(NULL, MAKEINTRESOURCEA(IDD_MON_PATH), hParent, _PathEditerProc);
}