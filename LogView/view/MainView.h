#pragma once
#include <Windows.h>
#include "../LogReceiver.h"
#include <LogLib/mstring.h>

enum LogViewMode {
    em_mode_config,         //��������ģʽ
    em_mode_debugMsg,       //������Ϣģʽ,����DbgView����
    em_mode_logFile,        //�ļ���־ģʽ
};

void PushLogContent(const LogInfoCache *cache);
void PushDbgContent(const std::mstring &content);
void ShowMainView();
void SwitchWorkMode(LogViewMode mode);
void UpdateStatusBar();

bool IsLogSniffRunning();
void NotifyLogSniff();