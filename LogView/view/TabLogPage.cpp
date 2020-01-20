#include "TabLogPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"
#include "../../LogLib/LogUtil.h"
#include "../GlobalDef.h"
#include <assert.h>
#include <CommCtrl.h>
#include "MainView.h"

using namespace std;

#define MSG_FILTER_RETURN       (WM_USER + 2011)

CTabLogPage::CTabLogPage() {
}

CTabLogPage::~CTabLogPage() {
}

void CTabLogPage::SetType(EM_LOGVIEW_TYPE type) {
    mType = type;
}

void CTabLogPage::AppendLog(const mstring &label, const mstring &content) {
    if (label == LABEL_DBG_CONTENT)
    {
        mSyntaxView.PushLog(content);
    } else if (label == LABEL_LOG_CONTENT)
    {
        mSyntaxView.PushLog(content);
    }
}

void CTabLogPage::ClearLog() {
    mSyntaxView.ClearCache();
    mSyntaxView.ClearLogView();
}

void CTabLogPage::SetAutoScroll(bool flag) {
    mSyntaxView.SetAutoScroll(flag);
}

INT_PTR CTabLogPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mFltCtrl = GetDlgItem(hwnd, IDC_COM_FILTER);
    mGroupCtrl = GetDlgItem(hwnd, IDC_ST_GROUP);
    mRadioRule = GetDlgItem(hwnd, IDC_RADIO_RULE);
    mRadioRegex = GetDlgItem(hwnd, IDC_RADIO_REGULAR);

    SendMessageA(mRadioRule, BM_SETCHECK, BST_CHECKED, 0);

    COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
    GetComboBoxInfo(mFltCtrl, &info);
    mFltEdit = info.hwndItem;

    RECT rtClient = {0};
    GetClientRect(hwnd, &rtClient);
    RECT rtFlt = {0};
    GetWindowRect(mFltCtrl, &rtFlt);
    MapWindowPoints(NULL, hwnd, (LPPOINT)&rtFlt, 2);

    int x = rtFlt.left;
    int y = rtFlt.bottom + 5;
    int width = (rtClient.right - rtClient.left) - 10;
    int high = (rtClient.bottom - rtClient.top) - y - 5;
    mSyntaxView.CreateLogView(hwnd, 5, y, width, high);
    mSyntaxView.InitLogBase();

    CTL_PARAMS arry[] = {
        {IDC_TEXT_FILTER, NULL, 0, 0, 0, 0},
        {IDC_COM_FILTER, NULL, 0, 0, 0.5f, 0},
        {IDC_TEXT_FIND, NULL, 0.5f, 0, 0, 0},
        {IDC_COM_FIND, NULL, 0.5f, 0, 0.5f, 0},
        {IDC_BTN_NEXT, NULL, 1, 0, 0, 0},
        {IDC_BTN_FRONT, NULL, 1, 0, 0, 0},
        {0, mGroupCtrl, 1, 0, 0, 0},
        {0, mRadioRule, 1, 0, 0, 0},
        {IDC_BTN_HELP, 0, 1, 0, 0, 0},
        {0, mRadioRegex, 1, 0, 0, 0},
        {0, mSyntaxView.GetWindow(), 0, 0, 1, 1}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));
    return 0;
}

INT_PTR CTabLogPage::OnFilterReturn(WPARAM wp, LPARAM lp) {
    mstring str(GetWindowStrA(mFltEdit));
    str.trim();

    if (str.empty()) {
        return 0;
    }
    mSyntaxView.SetFilter(str);
    LogViewConfigMgr::GetInst()->EnterFilterStr(mType, str);
    dp("str:%hs", str.c_str());
    int count = SendMessageA(mFltCtrl, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; i++) {
        SendMessageA(mFltCtrl, CB_DELETESTRING, 0, 0);
    }

    list<mstring> set1;
    if (mType == EM_VIEW_DBGLOG) {
        set1 = LogViewConfigMgr::GetInst()->GetDbgViewCfg().mFilterList;
    } else if (mType == EM_VIEW_FILELOG) {
        set1 = LogViewConfigMgr::GetInst()->GetFileLogViewCfg().mFilterList;
    }

    int pos = 0;
    for (list<mstring>::const_iterator it = set1.begin(); it != set1.end(); it++) {
        SendMessageA(mFltCtrl, CB_INSERTSTRING, pos, (LPARAM)it->c_str());
        pos++;
    }

    mstring strFirst = *(set1.begin());
    SendMessageA(mFltEdit, WM_SETTEXT, 0, (LPARAM)strFirst.c_str());
    SendMessageA(mFltEdit, EM_SETSEL, strFirst.size(), strFirst.size());
    return 0;
}

INT_PTR CTabLogPage::OnClose(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CTabLogPage::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    if (WM_INITDIALOG == msg)
    {
        return OnInitDialog(wp, lp);
    }
    else if (MSG_FILTER_RETURN == msg)
    {
        return OnFilterReturn(wp, lp);
    }
    else if (WM_CLOSE == msg)
    {
        return OnClose(wp, lp);
    }
    return 0;
}

INT_PTR CTabLogPage::GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (hwnd == mFltEdit && WM_KEYDOWN == msg)
    {
        if (wp == VK_RETURN)
        {
            PostMessageA(mHwnd, MSG_FILTER_RETURN, 0, 0);
        }
    } else if (WM_RBUTTONDOWN == msg && (hwnd == mSyntaxView.GetWindow()))
    {
        OnPopupMenu();
    }
    return 0;
}