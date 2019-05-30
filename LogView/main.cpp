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

static void _TestFileNotify(const char *filePath, unsigned int mask) {
    int dd = 1234;
}

static void _GetFileTime(const mstring &filePath) {
    HANDLE h = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    FILETIME t1 = {0}, t2 = {0}, t3 = {0};
    GetFileTime(h, &t1, &t2, &t3);
    CloseHandle(h);
}

static bool _TestFileEnumProc(bool isDir, const char *filePath, void *param) {
    if (isDir)
    {
        return true;
    }

    if (MonitorBase::IsLogFile(filePath))
    {
        list<mstring> *ptr = (list<mstring> *)param;
        ptr->push_back(filePath);
    }
    return true;
}

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    CDbgCapturer::GetInst()->InitCapturer();
    MessageBoxA(0, 0, 0, 0);
    return 0;

    g_hInstance = m;
    LoadLibraryA("SyntaxView.dll");

    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsadata;
    WSAStartup(wVersionRequested, &wsadata);

    //CLogReceiver::GetInst()->ConnectServ("10.10.16.191");
    //MessageBoxA(0, 0, 0, 0);
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
    GetModuleFileNameA(NULL, cfgPath, 256);
    PathAppendA(cfgPath, "..\\DataBase");
    SHCreateDirectoryExA(NULL, cfgPath, NULL);
    gCfgPath = cfgPath;

    gStartTime = timeStr;
    CLogServMgr::GetInst()->InitMgr();
    CLogReceiver::GetInst()->InitReceiver();
    ShowMainView();
    //CWinFileNotify::GetInst()->InitNotify();
    //CWinFileNotify::GetInst()->Register("D:\\git\\LogSniff\\Debug\\test", -1, _TestFileNotify, true);

    //MonitorCfg cfg;
    //cfg.mType = em_monitor_local;
    //CLogReceiver::GetInst()->Start(cfg);
    //CLogReceiver::GetInst()->AddPath("D:\\git\\LogSniff\\Debug\\test");
    //MessageBoxA(0, 0, 0, 0);
    WSACleanup();
    return 0;
}