#include "LogSyntaxView.h"

CLogSyntaxView::CLogSyntaxView() {

}

CLogSyntaxView::~CLogSyntaxView() {
}

bool CLogSyntaxView::CreateLogView(HWND hParent, int x, int y, int cx, int cy) {
    if (CreateView(hParent, x, y, cx, cy))
    {
        initLogView();
    }
    return true;
}

void CLogSyntaxView::initLogView() {
    RegisterParser(LABEL_DEFAULT, LogParser);

    ShowMargin(false);
    SetCaretColour(RGB(255, 255, 255));

    SetFont("Lucida Console");
    SetCaretSize(1);

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);

    SetDefStyle(RGB(255, 0, 0), RGB(40, 40, 40));

    ShowCaretLine(true, RGB(60, 56, 54));
    SendMsg(SCI_SETSELBACK, 1, RGB(255, 255, 255));
    SendMsg(SCI_SETSELALPHA, 70, 0);

    SetStyle(STAT_CONTENT, RGB(202, 255, 112), RGB(40, 40, 40));
}

void CLogSyntaxView::LogParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *sc
    )
{
    sc->SetState(STAT_CONTENT);
    sc->ForwardBytes(length);
    return;
}