#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include "MainView.h"
#include "GroupSender.h"
#include "LogServView.h"
#include "LogReceiver.h"
#include "LocalSniff/WinFileNoitfy.h"

using namespace std;

#if defined _M_IX86
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#  pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

HINSTANCE g_hInstance = NULL;

static void _TestFileNotify(const char *filePath, unsigned int mask) {
    int dd = 1234;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    g_hInstance = m;
    LoadLibraryA("SyntaxView.dll");

    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsadata;
    WSAStartup(wVersionRequested, &wsadata);

    //CLogReceiver::GetInst()->ConnectServ("10.10.16.191");
    //MessageBoxA(0, 0, 0, 0);
    //ShowMainView();
    //CWinFileNotify::GetInst()->InitNotify();
    //CWinFileNotify::GetInst()->Register("D:\\git\\LogSniff\\Debug\\test", -1, _TestFileNotify, true);

    MessageBoxA(0, 0, 0, 0);
    WSACleanup();
    return 0;
}