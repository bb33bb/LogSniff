#include "DbgView.h"
#include "DbgMsg.h"

using namespace std;

CDbgView::CDbgView() {}

CDbgView::~CDbgView() {}

bool CDbgView::CreateDbgView(HWND hParent, int x, int y, int cx, int cy) {
    if (CreateView(hParent, x, y, cx, cy))
    {
        InitLogBase();
        initDbgView();
    }

    InitCache(500);
    CDbgCapturer::GetInst()->InitCapturer();
    return true;
}

void CDbgView::initDbgView() {
    ShowMargin(true);
    SetLineNum(true);

    SendMsg(SCI_SETREADONLY, 1, 0);

    SetCaretColour(RGB(0, 0, 0));
    SetCaretSize(1);

    SetFont("����");

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

    //ѡ�����򱳾�ɫ��͸����
    SendMsg(SCI_SETSELBACK, true, RGB(0, 0, 0));
    SendMsg(SCI_SETSELALPHA, 50, 0);

    //Disable popup menu
    SendMsg(SCI_USEPOPUP, 0, 0);
}