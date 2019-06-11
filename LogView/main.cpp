#include <WinSock2.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <shlobj.h>
#include <string>
#include <LogLib/mstring.h>
#include "MainView.h"
#include "GroupSender.h"
#include "LogServView.h"
#include "LogReceiver.h"
#include "LocalSniff/LocalMonitor.h"
#include "LocalSniff/WinFileNoitfy.h"
#include "LogServMgr.h"
#include "DbgMsg.h"
#include "ServTreeView.h"
#include "resource.h"

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
mstring gStartTime;
mstring gCfgPath;
mstring gInstallDir;

static void _InitSniffer() {
    mstring dllPath;
    char installDir[256];
#ifdef _DEBUG
    GetModuleFileNameA(NULL, installDir, 256);
    dllPath = installDir;
    PathAppendA(dllPath, "..");
    gInstallDir = dllPath;
    dllPath.path_append("SyntaxView.dll");
#else
    GetWindowsDirectoryA(installDir, 256);

    PathAppendA(installDir, "LogSniff");

    dllPath = installDir;
    SHCreateDirectoryExA(NULL, dllPath.c_str(), NULL);
    gInstallDir = installDir;
    dllPath.path_append("SyntaxView.dll");
#endif
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dllPath.c_str()))
    {
        ReleaseRes(dllPath.c_str(), IDR_SYNTAX_DLL, "DLL");
    }
    LoadLibraryA(dllPath.c_str());
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    if (IsLogSniffRunning())
    {
        NotifyLogSniff();
        return 0;
    }

    g_hInstance = m;
    _InitSniffer();
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsadata;
    WSAStartup(wVersionRequested, &wsadata);

    SYSTEMTIME time = {0};
    GetLocalTime(&time);
    char timeStr[64];
    wnsprintfA(
        timeStr,
        64,
        "%04d-%02d-%02d %02d:%02d:%02d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond
        );

    char cfgPath[256];
    lstrcpyA(cfgPath, gInstallDir.c_str());
    PathAppendA(cfgPath, "DataBase");
    SHCreateDirectoryExA(NULL, cfgPath, NULL);
    gCfgPath = cfgPath;

    gStartTime = timeStr;
    CLogServMgr::GetInst()->InitMgr();
    ShowMainView();
    WSACleanup();
    return 0;
}