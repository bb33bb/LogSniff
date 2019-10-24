#include "TabSearchPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"
#include "../../LogLib/LogUtil.h"
#include "../../LogLib/TextDecoder.h"
#include <assert.h>
#include <fstream>
#include "MainView.h"

using namespace std;

#define MSG_SEARCH_RETURN       (WM_USER + 2011)

CTabSearchPage::CTabSearchPage() {
}

CTabSearchPage::~CTabSearchPage() {
}

void CTabSearchPage::SetLogServDesc(const LogServDesc *desc) {
    mServDesc = desc;
}

void CTabSearchPage::ClearLog() {
    mSyntaxView.ClearCache();
    mSyntaxView.ClearSearchView();
}

INT_PTR CTabSearchPage::OnInitDialog(WPARAM wp, LPARAM lp) {
    HWND hwnd = GetHandle();
    mFltCtrl = GetDlgItem(hwnd, IDC_COM_SEARCH);
    mCkRegular = GetDlgItem(hwnd, IDC_CK_SEARCH_REGULAR);

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
    int width = (rtClient.right - rtClient.left) - 2 * x;
    int high = (rtClient.bottom - rtClient.top) - y - 5;
    mSyntaxView.CreateSearchView(hwnd, x, y, width, high);

    CTL_PARAMS arry[] = {
        {IDC_COM_SEARCH, NULL, 0, 0, 1, 0},
        {IDC_CK_SEARCH_REGULAR, NULL, 1, 0, 0, 0},
        {0, mSyntaxView.GetWindow(), 0, 0, 1, 1}
    };
    SetCtlsCoord(hwnd, arry, RTL_NUMBER_OF(arry));
    return 0;
}

void CTabSearchPage::SearchSingleFile(const mstring &filePath, list<SearchInfo> &result) const {
    int bomLen = 0;
    TextEncodeType encodeType = CTextDecoder::GetInst()->GetFileType(filePath, bomLen);

    fstream fp(filePath.c_str(), ios::in);
    if (!fp.is_open())
    {
        return;
    }

    if (bomLen > 0)
    {
        fp.seekg(bomLen);
    }

    mstring lineStr;
    mstring decodeStr;
    while (getline(fp, lineStr)) {
        //没有文件bom头,只能根据文件内容或者编码类型
        if (em_text_unknown == encodeType)
        {
            encodeType = CTextDecoder::GetInst()->GetTextType(lineStr);
        }

        decodeStr = CTextDecoder::GetInst()->GetTextStr(lineStr, encodeType);
        //if (mScriptEngine.InputLog())
        {
        }
    }
    return;
}

void CTabSearchPage::SearchStrInFiles() const {
    const list<mstring> &set1 = mServDesc->mPathSet;
    list<mstring> fileSet;

    for (list<mstring>::const_iterator it = set1.begin() ; it != set1.end() ; it++)
    {
        struct LocalProc {
            static bool FileHandler(bool isDir, LPCSTR filePath, void *param) {
                list<mstring> *set1 = (list<mstring> *)param;

                if (MonitorBase::IsLogFile(filePath))
                {
                    set1->push_back(filePath);
                }
                return true;
            }
        };

        EnumFiles(*it, TRUE, LocalProc::FileHandler, &fileSet);
    }

    list<SearchInfo> result;
    for (list<mstring>::const_iterator ij = fileSet.begin() ; ij != fileSet.end() ; ij++)
    {
        SearchSingleFile(*ij, result);
    }
}

INT_PTR CTabSearchPage::OnSearchReturn(WPARAM wp, LPARAM lp) {
    mFilterStr = GetWindowStrA(mFltEdit);

    mScriptEngine.Compile(mFilterStr);
    return 0;
}

INT_PTR CTabSearchPage::OnClose(WPARAM wp, LPARAM lp) {
    return 0;
}

INT_PTR CTabSearchPage::MessageProc(UINT msg, WPARAM wp, LPARAM lp) {
    if (WM_INITDIALOG == msg)
    {
        return OnInitDialog(wp, lp);
    }
    else if (MSG_SEARCH_RETURN == msg)
    {
        return OnSearchReturn(wp, lp);
    }
    else if (WM_CLOSE == msg)
    {
        return OnClose(wp, lp);
    }
    return 0;
}

INT_PTR CTabSearchPage::GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (hwnd == mFltEdit && WM_KEYDOWN == msg)
    {
        if (wp == VK_RETURN)
        {
            SendMessageW(mHwnd, MSG_SEARCH_RETURN, 0, 0);
        }
    } else if (WM_RBUTTONDOWN == msg && (hwnd == mSyntaxView.GetWindow()))
    {
        OnPopupMenu();
    }
    return 0;
}