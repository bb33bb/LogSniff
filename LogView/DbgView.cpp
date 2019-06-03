#include "DbgView.h"
#include "DbgMsg.h"

using namespace std;

CDbgView::CDbgView() {}

CDbgView::~CDbgView() {}

bool CDbgView::CreateDbgView(HWND hParent, int x, int y, int cx, int cy) {
    if (CreateView(hParent, x, y, cx, cy))
    {
        initDbgView();
    }

    InitCache(LABEL_DBG_CONTENT, 500);
    CDbgCapturer::GetInst()->InitCapturer();
    return true;
}

void CDbgView::initDbgView() {
    ShowMargin(true);
    SetLineNum(true);
    SetCaretColour(RGB(255, 255, 255));

    SetFont("ËÎÌו");
    SetCaretSize(1);

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);

    SetDefStyle(RGB(0, 0, 0), RGB(255, 255, 255));
    ShowCaretLine(true, RGB(232, 232, 255));

    SendMsg(SCI_SETSCROLLWIDTHTRACKING, 1, 1);
}