#include "SearchSyntaxView.h"
#include <Shlwapi.h>
#include <math.h>
#include "../../LogLib/LogUtil.h"
#include "../../LogLib/StrUtil.h"

using namespace std;

CSearchView::CSearchView() {
}

CSearchView::~CSearchView() {
}

void CSearchView::InitStyle() {
    //日志文件信息样式
    SetStyle(STYLE_SEARCH_FILE, RGB(0, 0, 255), RGB(255, 255, 255));

    SetStyle(STYLE_LOG_KEYWORD_BASE + 0, RGB(0, 0, 0), RGB(0, 0xff, 0));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 1, RGB(0, 0, 0), RGB(0xd0, 0xd0, 0xff));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 2, RGB(0, 0, 0), RGB(0xd0, 0xff, 0xd0));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 3, RGB(0, 0, 0), RGB(0xcd, 0xcd, 0x00));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 4, RGB(0, 0, 0), RGB(0xca, 0xff, 0x70));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 5, RGB(0, 0, 0), RGB(0xb0, 0xe2, 0xff));
    SetStyle(STYLE_LOG_KEYWORD_BASE + 6, RGB(0, 0, 0), RGB(0xff, 0xd7, 0x00));

    int i = 0, j = 0;
    for (i = STYLE_LOG_KEYWORD_BASE + 7 ; i < 255 ; i++)
    {
        static DWORD sMagic = 0x12f;
        srand(GetTickCount() + sMagic++);
        BYTE r = rand() % 128 + 128;
        BYTE g = rand() % 128 + 128;
        BYTE b = rand() % 128 + 128;
        SetStyle(i, RGB(0, 0, 0), RGB(r, g, b));
    }
    SetStyle(STYLE_LOG_WARN, RGB(0, 0, 0), RGB(0xee, 0xb4, 0x22));
    SetStyle(STYLE_LOG_ERROR, RGB(0, 0, 0), RGB(0xf0, 0x80, 0x80));
}

void CSearchView::InitViewStyle() {
    ShowMargin(true);
    SetLineNum(true);

    SendMsg(SCI_SETREADONLY, 1, 0);

    SetCaretColour(RGB(0, 0, 0));
    SetCaretSize(1);

    SetFont("宋体");

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);

    SetDefStyle(RGB(0, 0, 0), RGB(255, 255, 255));
    ShowCaretLine(true, RGB(0x00, 0x00, 0xcd), 30);

    SetStyle(STYLE_CONTENT, RGB(0, 0, 0), RGB(255, 255, 255));

    SendMsg(SCI_SETSCROLLWIDTHTRACKING, 1, 1);

    SendMsg(SCI_SETMARGINCURSORN, 0, SC_CURSORARROW);
    SendMsg(SCI_SETMARGINCURSORN, 1, SC_CURSORARROW);

    SendMsg(SCI_SETFOLDMARGINCOLOUR, 1, RGB(255, 0, 0));
    SendMsg(SCI_SETFOLDMARGINHICOLOUR, 1, RGB(255, 0, 0));
    SetStyle(STYLE_LINENUMBER, RGB(128, 128, 128), RGB(228, 228, 228));

    //选择区域背景色和透明度
    SendMsg(SCI_SETSELBACK, true, RGB(0, 0, 0));
    SendMsg(SCI_SETSELALPHA, 50, 0);

    //Disable popup menu
    SendMsg(SCI_USEPOPUP, 0, 0);
}

void CSearchView::InitSearchView() {
    mTotalCount = 0, mShowCount = 0;

    InitStyle();
    InitViewStyle();
    InitCache(100);
    RegisterParser(LABEL_SEARCH_FILE, SearchPathParser, this);
    RegisterParser(LABEL_SEARCH_LOG, SearchLogParser, this);
}

bool CSearchView::CreateSearchView(HWND hParent, int x, int y, int cx, int cy) {
    CreateView(hParent, x, y, cx, cy);

    InitSearchView();
    return true;
}

void CSearchView::SetStyleSet(map<mstring, int> &set1) {
    mStyleSet = set1;
}

//1分钟前, 1小时20分钟前, 1天前, 1周前, 1月前
//time:Local FileTime
mstring CSearchView::GetTimeDesc(const FILETIME &time) const {
    SYSTEMTIME t1 = {0};
    GetLocalTime(&t1);

    FILETIME f1 = {0};
    SystemTimeToFileTime(&t1, &f1);

    ULONGLONG d1 = (((ULONGLONG)(time.dwHighDateTime) << 32) | (ULONGLONG)time.dwLowDateTime);
    ULONGLONG d2 = (((ULONGLONG)(f1.dwHighDateTime) << 32) | (ULONGLONG)f1.dwLowDateTime);
    ULONGLONG d3 = (d2 - d1) / 10000 / 1000;

    mstring result;
    //分钟之内
    if (d3 < 60)
    {
        return FormatA("%llu秒前", d3);
    }
    //一小时之内
    else if (d3 >= 60 && d3 < (60 * 60))
    {
        return FormatA("%llu分钟前", d3 / 60);
    }
    //一天之内
    else if (d3 >= (60 * 60) && d3 < (60 * 60 * 24))
    {
        return FormatA("%llu小时%llu分钟前", d3 / 60 / 60, d3 % (60 * 60));
    }
    //一天之外
    else
    {
        ULONGLONG dd = d3 / (60 * 60 * 24);
        return FormatA("%llu天前", d3 / (60 * 60 * 24));
    }
}

mstring CSearchView::GetLogFileInfo(const mstring &filePath) const {
    mstring result;
    result += FormatA("日志文件:%hs\n", filePath.c_str());

    HANDLE hFile = CreateFileA(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );
    HandleAutoClose abc(hFile);

    if (NULL == hFile || INVALID_HANDLE_VALUE == hFile)
    {
        return result;
    }

    FILETIME t1 = {0}, t2 = {0}, t3 = {0};
    GetFileTime(hFile, &t1, &t2, &t3);

    FILETIME l1 = {0}, l2 = {0}, l3 = {0};
    FileTimeToLocalFileTime(&t1, &l1);
    FileTimeToLocalFileTime(&t2, &l2);
    FileTimeToLocalFileTime(&t3, &l3);

    SYSTEMTIME time = {0};
    FileTimeToSystemTime(&l1, &time);
    mstring str = FormatA(
        "%04d-%02d-%02d %02d:%02d:%02d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond
        );
    result += FormatA("创建时间:%hs %hs\n", str.c_str(), GetTimeDesc(l1).c_str());

    FileTimeToSystemTime(&l3, &time);
    str = FormatA(
        "%04d-%02d-%02d %02d:%02d:%02d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond
        );
    result += FormatA("最后写入:%hs %hs\n", str.c_str(), GetTimeDesc(l3).c_str());

    DWORD high = 0;
    DWORD low = GetFileSize(hFile, &high);

    char buff[128];
    sprintf(buff, "文件大小:%.2f KB\n", (float)low / (float)1024);
    result += buff;
    return result;
}

void CSearchView::PushSearchResult(const mstring &filePath, const mstring &content) {
    PushToCache(LABEL_SEARCH_FILE, GetLogFileInfo(filePath));
    PushToCache(LABEL_SEARCH_FILE, "\n");
    PushToCache(LABEL_SEARCH_LOG, content);
}

void CSearchView::GetLineCount(int &total, int &show) {
    total = mTotalCount;
    show = mShowCount;
}

void CSearchView::ClearSearchView() {
    ClearCache();
    ClearView();
}

void CSearchView::SearchPathParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_SEARCH_FILE);
    s->ForwardBytes(length);
}

void CSearchView::OnSearchLogStyle(const char *ptr, unsigned int startPos, int length, StyleContextBase *sc) const {
    if (mStyleSet.empty())
    {
        sc->SetState(STYLE_SEARCH_LOG);
        sc->ForwardBytes(length);
        return;
    }

    int lastPos = 0;
    for (int i = 0 ; i < length ;)
    {
        bool match = false;
        for (map<mstring, int>::const_iterator it = mStyleSet.begin() ; it != mStyleSet.end() ; it++)
        {
            if (0 == StrCmpNIA(it->first.c_str(), ptr + i, it->first.size()))
            {
                if (i > lastPos)
                {
                    sc->SetState(STYLE_SEARCH_LOG);
                    sc->ForwardBytes(i - lastPos);
                }

                sc->SetState(it->second);
                sc->ForwardBytes(it->first.size());
                i += it->first.size();
                lastPos = i;
                match = true;
                break;
            }
        }

        if (!match)
        {
            i++;
        }
    }

    if (lastPos < length)
    {
        sc->SetState(STYLE_SEARCH_LOG);
        sc->ForwardBytes(length - lastPos);
    }
}

void CSearchView::SearchLogParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    CSearchView *pThis = (CSearchView *)param;
    pThis->OnSearchLogStyle(ptr, startPos, length, s);
}
