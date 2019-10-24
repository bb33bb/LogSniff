#include "TabCfgPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"
#include "../../LogLib/StrUtil.h"
#include "../LogReceiver.h"
#include <CommCtrl.h>

using namespace std;

CTabCfgPage::CTabCfgPage() {
}

CTabCfgPage::~CTabCfgPage() {
}

void CTabCfgPage::SetServDesc(const LogServDesc *servDesc) {
    mServDesc = servDesc;
}

void CTabCfgPage::InitListCtrl() const {
    ListView_SetExtendedListViewStyle(mPathList, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
    LVCOLUMNA col;
    memset(&col, 0x00, sizeof(col));
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 40;
    col.pszText = (LPSTR)"序号";
    SendMessageA(mPathList, LVM_INSERTCOLUMNA, 0, (LPARAM)&col);

    col.cx = 640;
    col.pszText = (LPSTR)"路径";
    SendMessageA(mPathList, LVM_INSERTCOLUMNA, 1, (LPARAM)&col);
}

void CTabCfgPage::LoadPathSet() const {
    const list<mstring> &set1 = mServDesc->mPathSet;
    int index = 0;

    SendMessageA(mPathList, LVM_DELETEALLITEMS, 0, 0);
    for (list<mstring>::const_iterator it = set1.begin() ; it != set1.end() ; it++, index++)
    {
        LVITEMA info = {0};
        info.mask = LVIF_TEXT ;
        info.iItem = index;

        info.iSubItem = 0;
        mstring str = FormatA("%d", index);
        info.pszText = (LPSTR)(str.c_str());
        SendMessageA(mPathList, LVM_INSERTITEMA, 0, (LPARAM)&info);

        info.iSubItem = 1;
        info.pszText = (LPSTR)it->c_str();
        SendMessageA(mPathList, LVM_SETITEMA, 0, (LPARAM)&info);
    }
}

void CTabCfgPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mEditPath = GetDlgItem(hwnd, IDC_CFG_PATH);
    mEditExt = GetDlgItem(hwnd, IDC_CFG_EXT);
    mPathList = GetDlgItem(hwnd, IDC_CFG_PATH_LIST);
    mEditStatus = GetDlgItem(hwnd, IDC_CFG_INFO);

    //临时写死,将来支持可配置
    SetWindowTextA(mEditExt, "*.txt;*.log");

    InitListCtrl();
    LoadPathSet(); 

    CTL_PARAMS arry[] = {
        {IDC_CFG_PATH, NULL, 0, 0, 1, 0},
        {IDC_CFG_EXT, NULL, 0, 0, 1, 0},
        {IDC_CFG_PATH_LIST, NULL, 0, 0, 1, 1},
        {IDC_CFG_INFO, NULL, 0, 1, 1, 0},
        {IDC_CFG_DEL, NULL, 1, 1, 0, 0},
        {IDC_CFG_OK, NULL, 1, 1, 0, 0}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));
}

bool CTabCfgPage::IsPathInCache(const mstring &path) const {
    mstring tmp(path);
    tmp.makelower(), tmp.repsub("/", "\\");

    const list<mstring> &set1 = mServDesc->mPathSet;
    for (list<mstring>::const_iterator it = set1.begin() ; it != set1.end() ; it++)
    {
        mstring tmp2 = *it;
        tmp2.makelower(), tmp2.repsub("/", "\\");

        if (mstring::npos != tmp.find(tmp2))
        {
            return true;
        }
    }
    return false;
}

void CTabCfgPage::OnCommand(WPARAM wp, LPARAM lp) {
    WORD id = LOWORD(wp);

    if (IDC_CFG_OK == id)
    {
        mstring str = GetWindowStrA(mEditPath);

        str.trim();
        if (str.empty())
        {
            return;
        }

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(str.c_str()))
        {
            MessageBoxA(mEditStatus, "不存在的日志路径", "警告", MB_ICONERROR | MB_OK);
            return;
        }

        if (IsPathInCache(str))
        {
            MessageBoxA(mEditStatus, "日志路径已存在", "警告", MB_ICONERROR | MB_OK);
            return; 
        }
        CLogReceiver::GetInst()->AddPathMonitor(str);
        LoadPathSet();
    } else if (IDC_CFG_DEL == id)
    {
        struct SelectNode {
            int mItem;
            mstring mPathStr;
        };

        int lastItem = - 1;
        list<SelectNode> set1;
        char buff[512];

        LVITEMA item = {0};
        item.pszText = buff;
        item.cchTextMax = sizeof(buff);
        item.iSubItem = 1;

        while (true) {
            int sel = SendMessageA(mPathList, LVM_GETNEXTITEM, lastItem, LVNI_SELECTED);

            if (sel < 0)
            {
                break;
            }

            SelectNode node;
            buff[0] = 0, item.cchTextMax = sizeof(buff);
            SendMessageA(mPathList, LVM_GETITEMTEXTA, sel, (LPARAM)&item);

            node.mItem = sel;
            node.mPathStr = buff;
            set1.push_back(node);
            lastItem = sel;
        }

        if (set1.empty())
        {
            MessageBoxA(GetHandle(), "请至少选择一条要要删除的路径", "警告", MB_ICONERROR | MB_OK);
            return;
        } else {
            if (IDOK != MessageBoxA(GetHandle(), "确定删除选中的路径?", "警告", MB_ICONQUESTION | MB_OK))
            {
                return;
            }

            for (list<SelectNode>::const_reverse_iterator it = set1.rbegin() ; it != set1.rend() ; it++)
            {
                CLogReceiver::GetInst()->DelPathMonitor(it->mPathStr);
            }
            LoadPathSet();
        }
    }
}

void CTabCfgPage::OnClose(WPARAM wp, LPARAM lp) {
}

INT_PTR CTabCfgPage::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_INITDIALOG:
            OnInitDialog(wp, lp);
            break;
        case WM_COMMAND:
            OnCommand(wp, lp);
            break;
        case WM_CLOSE:
            OnClose(wp, lp);
            break;
    }
    return 0;
}