#include <WinSock2.h>
#include "LogSyntaxView.h"
#include <LogLib/LogUtil.h>
#include "../LogReceiver.h"

using namespace std;

CLogSyntaxView::CLogSyntaxView() {
}

CLogSyntaxView::~CLogSyntaxView() {
}

bool CLogSyntaxView::CreateLogView(HWND hParent, int x, int y, int cx, int cy) {
    if (CreateView(hParent, x, y, cx, cy))
    {
        InitLogBase();
        initLogView();
    }

    InitCache(500);
    CLogReceiver::GetInst()->InitReceiver();
    return true;
}

void CLogSyntaxView::initLogView() {
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

    //查找关键字
    SetStyle(STYLE_FIND_KEYWORD, RGB(0, 0, 0), RGB(0, 255, 0));
    //选择关键字
    SetStyle(STYLE_SELECT_KEYWORD, RGB(0, 0, 0), RGB(0, 255, 0));

    //选择区域背景色和透明度
    SendMsg(SCI_SETSELBACK, true, RGB(0, 0, 0));
    SendMsg(SCI_SETSELALPHA, 50, 0);

    //Disable popup menu
    SendMsg(SCI_USEPOPUP, 0, 0);
}