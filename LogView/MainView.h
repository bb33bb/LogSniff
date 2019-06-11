#pragma once
#include <Windows.h>
#include "LogReceiver.h"
#include <LogLib/mstring.h>

enum LogViewMode {
    em_mode_debugMsg = 0,
    em_mode_logFile
};

void PushLogContent(const LogInfoCache *cache);
void PushDbgContent(const std::mstring &content);
void ShowMainView();
void SwitchWorkMode(LogViewMode mode);
void UpdateStatusBar();

bool IsLogSniffRunning();
void NotifyLogSniff();