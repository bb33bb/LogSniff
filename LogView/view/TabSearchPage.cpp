#include "TabSearchPage.h"
#include "../resource.h"
#include "../../LogLib/winsize.h"
#include "../../LogLib/LogUtil.h"
#include "../../LogLib/TextDecoder.h"
#include "../../LogLib/StrUtil.h"
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

    if (em_text_unknown == encodeType)
    {
        encodeType = em_text_utf8;
    }

    FILE *fp = fopen(filePath.c_str(), "rb");

    if (NULL == fp)
    {
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (bomLen > 0)
    {
        fseek(fp, bomLen, SEEK_SET);
    }

    char buff[4096 * 2];
    const size_t maxRead = (4096 * 2 - 4);
    mstring lineStr;
    mstring decodeStr;

    SearchInfo content;
    content.mFilePath = filePath;

    long readCount = 0;
    long totalCount = 0;
    bool readEnd = false;

    //每次读取的字节数
    int readOnceCount = 1;

    if (encodeType == em_text_unicode_le)
    {
        readOnceCount = sizeof(wchar_t);
    }

    int lineCount = 0;
    while (true) {
        readCount = 0;
        while (true) {
            if (readOnceCount != fread(buff + readCount, 1, readOnceCount, fp))
            {
                readEnd = true;
                break;
            }

            if (0 == memcmp(buff + readCount, "\n", readOnceCount))
            {
                readCount += readOnceCount;
                totalCount += readOnceCount;
                lineCount++;
                break;
            }
            readCount += readOnceCount;
            totalCount += readOnceCount;

            if (readCount >= maxRead)
            {
                break;
            }
        }

        buff[readCount] = 0x00, buff[readCount + 1] = 0x00;
        if (em_text_unicode_le == encodeType)
        {
            //Unicode补齐最后一位
            decodeStr = CTextDecoder::GetInst()->GetTextStr(string(buff, readCount + 2), encodeType);
        } else {
            decodeStr = CTextDecoder::GetInst()->GetTextStr(buff, encodeType);
        }

        LogFilterResult result;
        if (mScriptEngine.InputLog(decodeStr, 0, result))
        {
            content.mContent += FormatA("line:%d ", lineCount);
            content.mContent += result.mContent;
        }

        if (readEnd)
        {
            break;
        }
    }

    fclose(fp);
    if (!content.mContent.empty())
    {
        result.push_back(content);
    }
    return;
}

void CTabSearchPage::SearchStrInFiles() {
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

    for (list<SearchInfo>::const_iterator it2 = result.begin() ; it2 != result.end() ; it2++)
    {
        mSyntaxView.PushSearchResult(it2->mFilePath, it2->mContent + "\n");
    }
}

INT_PTR CTabSearchPage::OnSearchReturn(WPARAM wp, LPARAM lp) {
    mSyntaxView.ClearSearchView();
    mFilterStr = GetWindowStrA(mFltEdit);

    if (mScriptEngine.Compile(mFilterStr))
    {
        mSyntaxView.SetStyleSet(mScriptEngine.GetStyleSet());
    }

    SearchStrInFiles();
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