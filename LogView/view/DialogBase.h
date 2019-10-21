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
    //系统消息回调
    virtual INT_PTR MessageProc(UINT msg, WPARAM wp, LPARAM lp);
    //消息钩子回调,重载此函数可以处理子控件的消息
    virtual INT_PTR GetMsgHook(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
    //对话框回调函数
    static INT_PTR CALLBACK DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp);
    //消息钩子回调函数
    static LRESULT CALLBACK GetMsgProc(int code, WPARAM wp, LPARAM lp);

protected:
    static std::map<HWND, CDialogBase *> msPtrSet;
    HWND mHwnd;
    HWND mParent;
    static HHOOK msHHook;
};