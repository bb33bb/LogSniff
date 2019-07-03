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
#include "LogScript/ScriptEngine.h"

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
    char dllPath[256];
    char installDir[256];
#ifdef _DEBUG
    GetModuleFileNameA(NULL, installDir, 256);
    lstrcpyA(dllPath, installDir);
    PathAppendA(dllPath, "..");
    gInstallDir = dllPath;
    PathAppendA(dllPath, "SyntaxView.dll");
#else
    GetWindowsDirectoryA(installDir, 256);

    PathAppendA(installDir, "LogSniff");

    lstrcpyA(dllPath, installDir);
    SHCreateDirectoryExA(NULL, dllPath, NULL);
    gInstallDir = installDir;
    PathAppendA(dllPath, "SyntaxView.dll");
#endif
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(dllPath))
    {
        ReleaseRes(dllPath, IDR_SYNTAX_DLL, "DLL");
    }
    LoadLibraryA(dllPath);
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    //CScriptEngine::GetInst()->Compile("aaaa || dddd");
    //LogFilterResult result = CScriptEngine::GetInst()->InputLog("fdjgdfgjkdlfkdfaaaaafjdgj4353dddd1144");
    //int ff = 1234;
    //return 0;
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