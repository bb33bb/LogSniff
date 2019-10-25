#include "SearchSyntaxView.h"

using namespace std;

CSearchView::CSearchView() {
}

CSearchView::~CSearchView() {
}

void CSearchView::InitStyle() {
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

void CSearchView::PushSearchResult(const mstring &filePath, const mstring &content) {
    PushToCache(LABEL_SEARCH_FILE, filePath);
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

void CSearchView::SearchLogParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *s,
    void *param
    )
{
    s->SetState(STYLE_SEARCH_LOG);
    s->ForwardBytes(length);
}
