#pragma once
#include "DialogBase.h"

class CAboutDlg : public CDialogBase {
public:
    CAboutDlg();
    virtual ~CAboutDlg();

private:
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);
    //ϵͳ��Ϣ�ص�
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);

private:
    HWND mEditVersion;
    HWND mEditDesc;
};