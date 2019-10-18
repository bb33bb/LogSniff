#pragma once
#include <Windows.h>
#include "../LogReceiver.h"
#include <LogLib/mstring.h>

enum LogViewMode {
    em_mode_config,         //参数配置模式
    em_mode_debugMsg,       //调试信息模式,类似DbgView工具
    em_mode_logFile,        //文件日志模式
};

void PushLogContent(const LogInfoCache *cache);
void PushDbgContent(const std::mstring &content);
void ShowMainView();
void SwitchWorkMode(LogViewMode mode);
void UpdateStatusBar();

bool IsLogSniffRunning();
void NotifyLogSniff();