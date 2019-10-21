#pragma once
#include <Windows.h>
#include <map>

class CDialogBase {
public:
    CDialogBase();
    virtual ~CDialogBase();

    int DoModule(HWND parent, DWORD id);
    BOOL CreateDlg(HWND parent, DWORD id);
    void ShowDlg() const;
    void HideDlg() const;
    HWND GetHandle() const;
    HWND GetParent() const;
protected:
    //ϵͳ��Ϣ�ص�
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);
    //��Ϣ���ӻص�,���ش˺������Դ����ӿؼ�����Ϣ
    virtual INT_PTR GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    //�Ի���ص�����
    static INT_PTR CALLBACK DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
    //��Ϣ���ӻص�����
    static LRESULT CALLBACK GetMsgProc(int code, WPARAM wp, LPARAM lp);

protected:
    static std::map<HWND, CDialogBase *> msPtrSet;
    HWND mHwnd;
    HWND mParent;
    static HHOOK msHHook;
};