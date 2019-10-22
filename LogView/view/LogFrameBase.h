#pragma once
#include "../../LogLib/mstring.h"
#include "DialogBase.h"
#include "../GlobalDef.h"
#include "../MonitorBase.h"

//日志展示窗体基类
class CLogFrameBase : public CDialogBase {
public:
    virtual void InitLogFrame(const LogServDesc *servDesc) = 0;
    virtual void UpdateConfig() = 0;
    virtual void OnFileLog(const std::mstring &content) = 0;
    virtual void OnDbgLog(const std::mstring &content) = 0;
    virtual void OnFileSearchLog(const std::mstring &filePath, const std::mstring &content) = 0;
};