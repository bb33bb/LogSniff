#include "DialogBase.h"

using namespace std;

HHOOK CDialogBase::msHHook = NULL;
map<HWND, CDialogBase *> CDialogBase::msPtrSet;

CDialogBase::CDialogBase() {
}

CDialogBase::~CDialogBase() {
}

int CDialogBase::DoModule(HWND parent, DWORD id) {
    mParent = parent;
    return DialogBoxParamA(NULL, MAKEINTRESOURCEA(id), parent, DialogProc, (LPARAM)this);
}

BOOL CDialogBase::CreateDlg(HWND parent, DWORD id) {
    mParent = parent;
    mHwnd = CreateDialogParamA(NULL, MAKEINTRESOURCEA(id), parent, DialogProc, (LPARAM)this);
    return (TRUE == IsWindow(mHwnd));
}

void CDialogBase::ShowDlg() const {
    ShowWindow(mHwnd, SW_SHOW);
}

void CDialogBase::HideDlg() const {
    ShowWindow(mHwnd, SW_HIDE);
}

HWND CDialogBase::GetHandle() const {
    return mHwnd;
}

INT_PTR CDialogBase::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    return 0;
}

//消息钩子回调,重载此函数可以处理子控件的消息
INT_PTR CDialogBase::GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CDialogBase::DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_INITDIALOG)
    {
        CDialogBase *ptr = (CDialogBase *)lp;
        msPtrSet[hdlg] = ptr;
        ptr->mHwnd = hdlg;

        if (NULL == msHHook)
        {
            extern HINSTANCE g_hInstance;
            msHHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hInstance, GetCurrentThreadId());
        }
    }

    map<HWND, CDialogBase*>::iterator it = msPtrSet.find(hdlg);
    if (it == msPtrSet.end())
    {
        return 0;
    }

    INT_PTR result = it->second->MessageProc(msg, wp, lp);
    if (msg == WM_DESTROY)
    {
        msPtrSet.erase(hdlg);
    }
    return result;
}

//消息钩子回调函数
LRESULT CDialogBase::GetMsgProc(int code, WPARAM wp, LPARAM lp) {
    for (map<HWND, CDialogBase *>::const_iterator it = msPtrSet.begin() ; it != msPtrSet.end() ; it++)
    {
        CDialogBase *tmp = it->second;
        MSG *pMsg = (MSG *)lp;

        if (pMsg)
        {
            tmp->GetMsgHook(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam);
        }
    }

    return CallNextHookEx(msHHook, code, wp, lp);
}