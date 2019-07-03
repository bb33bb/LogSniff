#include <WinSock2.h>
#include "LogSyntaxView.h"
#include <LogLib/LogUtil.h>
#include "LogReceiver.h"

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

    InitCache(500);
    CLogReceiver::GetInst()->InitReceiver();
    return true;
}

bool CLogSyntaxView::SetHightStr(const std::string &str) {
    mKeywordStr = str;
    SendMsg(SCI_COLOURISE, 0, -1);
    return true;
}

void CLogSyntaxView::initLogView() {
    ShowMargin(true);
    SetLineNum(true);

    SendMsg(SCI_SETREADONLY, 1, 0);

    SetCaretColour(RGB(0, 0, 0));
    SetCaretSize(1);

    SetFont("ËÎÌו");

    SendMsg(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
    ShowVsScrollBar(true);
    ShowHsScrollBar(true);

    SetDefStyle(RGB(0, 0, 0), RGB(255, 255, 255));
    ShowCaretLine(true, RGB(232, 232, 255));

    SetStyle(STYLE_CONTENT, RGB(0, 0, 0), RGB(255, 255, 255));
    SetStyle(STYLE_KEYWORD, RGB(255, 0, 0), RGB(0, 0, 255));

    SendMsg(SCI_SETSCROLLWIDTHTRACKING, 1, 1);

    SendMsg(SCI_SETMARGINCURSORN, 0, SC_CURSORARROW);
    SendMsg(SCI_SETMARGINCURSORN, 1, SC_CURSORARROW);

    SendMsg(SCI_SETFOLDMARGINCOLOUR, 1, RGB(255, 0, 0));
    SendMsg(SCI_SETFOLDMARGINHICOLOUR, 1, RGB(255, 0, 0));
    SetStyle(STYLE_LINENUMBER, RGB(128, 128, 128), RGB(228, 228, 228));

    //Disable popup menu
    SendMsg(SCI_USEPOPUP, 0, 0);
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
                    sc->SetState(STYLE_CONTENT);
                    sc->ForwardBytes(str.size() - lastPos);
                }
                break;
            }

            if (curPos > lastPos)
            {
                sc->SetState(STYLE_CONTENT);

                sc->ForwardBytes(curPos - lastPos);
            }
            sc->SetState(STYLE_KEYWORD);
            sc->ForwardBytes(keyWord.size());
            lastPos = (curPos + keyWord.size());
        }
    } else {
        sc->SetState(STYLE_CONTENT);
        sc->ForwardBytes(length);
    }
    return;
}