#include "LogSyntaxView.h"
#include <LogLib/LogUtil.h>

using namespace std;

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

bool CLogSyntaxView::SetHightStr(const std::string &str) {
    mKeywordStr = str;
    SendMsg(SCI_COLOURISE, 0, -1);
    return true;
}

void CLogSyntaxView::initLogView() {
    RegisterParser(LABEL_LOG_CONTENT, LogParser, this);

    ShowMargin(true);
    SetCaretColour(RGB(255, 255, 255));

    SetFont("Lucida Console");
    SetCaretSize(1);

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);

    SetDefStyle(RGB(0, 0, 0), RGB(255, 255, 255));
    ShowCaretLine(true, RGB(232, 232, 255));
    //SendMsg(SCI_SETSELALPHA, 70, 0);

    SetStyle(STAT_CONTENT, RGB(0, 0, 0), RGB(255, 255, 255));
    SetStyle(STAT_KEYWORD, RGB(255, 0, 0), RGB(0, 0, 255));
    SendMsg(SCI_SETSCROLLWIDTHTRACKING, 1, 1);
    SendMsg(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    int w = SendMsg(SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"_99999");
    SendMsg(SCI_SETMARGINWIDTHN, 0, w);
    SendMsg(SCI_SETMARGINWIDTHN, 1, 0);
    SendMsg(SCI_SETMARGINCURSORN, 0, SC_CURSORARROW);
    SendMsg(SCI_SETMARGINCURSORN, 1, SC_CURSORARROW);

    SendMsg(SCI_SETFOLDMARGINCOLOUR, 1, RGB(255, 0, 0));
    SendMsg(SCI_SETFOLDMARGINHICOLOUR, 1, RGB(255, 0, 0));
    SetStyle(STYLE_LINENUMBER, RGB(128, 128, 128), RGB(228, 228, 228));
}

void CLogSyntaxView::LogParser(
    int initStyle,
    unsigned int startPos,
    const char *ptr,
    int length,
    StyleContextBase *sc,
    void *param
    )
{
    CLogSyntaxView *pThis = (CLogSyntaxView *)param;
    //dp("start:%d, length:%d, str:%hs", startPos, length, ptr);

    if (!pThis->mKeywordStr.empty())
    {
        string str(ptr, length);
        string keyWord = pThis->mKeywordStr;
        size_t curPos = 0;
        size_t lastPos = 0;

        while (true) {
            curPos = str.find(keyWord, lastPos);

            if (string::npos == curPos)
            {
                if (str.size() > lastPos)
                {
                    sc->SetState(STAT_CONTENT);
                    sc->ForwardBytes(str.size() - lastPos);
                }
                break;
            }

            if (curPos > lastPos)
            {
                sc->SetState(STAT_CONTENT);

                sc->ForwardBytes(curPos - lastPos);
            }
            sc->SetState(STAT_KEYWORD);
            sc->ForwardBytes(keyWord.size());
            lastPos = (curPos + keyWord.size());
        }
    } else {
        sc->SetState(STAT_CONTENT);
        sc->ForwardBytes(length);
    }
    return;
}