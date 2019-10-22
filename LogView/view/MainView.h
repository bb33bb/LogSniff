#pragma once
#include <Windows.h>
#include "../LogReceiver.h"
#include <LogLib/mstring.h>

void PushLogContent(const LogInfoCache *cache);
void PushDbgContent(const std::mstring &content);
void ShowMainView();
void UpdateStatusBar();

bool IsLogSniffRunning();
void NotifyLogSniff();