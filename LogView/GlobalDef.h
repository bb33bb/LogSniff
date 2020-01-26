#pragma once
#include <Windows.h>
#include <list>
#include "../LogLib/mstring.h"
#include "../LogLib/json/json.h"

enum EM_LOGVIEW_TYPE {
    EM_VIEW_CONFIG,
    EM_VIEW_DBGLOG,
    EM_VIEW_FILELOG,
    EM_VIEW_FILESEARCH
};

struct GlobalConfig {
    bool mTopMost;
    bool mPause;
    bool mAutoScroll;
    EM_LOGVIEW_TYPE mCurView;

    GlobalConfig() {
        mTopMost = false, mPause = false;
        mAutoScroll = false, mCurView = EM_VIEW_DBGLOG;
    }
};

struct DbgLogViewConfig {
    std::list<std::mstring> mFilterList;
    std::list<std::mstring> mFindList;
};

struct FileLogViewConfig {
    std::list<std::mstring> mFilterList;
    std::list<std::mstring> mFindList;
};

struct FileSearchViewConfig {
    std::list<std::mstring> mFilterList;
    std::list<std::mstring> mFindList;
};

class LogViewConfigMgr {
public:
    static LogViewConfigMgr *GetInst();
    void SaveConfig();
    void LoadConfig();

    void SetGlobalCfg(const GlobalConfig &cfg);
    GlobalConfig GetGlobalCfg() const;

    void SetDbgViewCfg(const DbgLogViewConfig &cfg);
    DbgLogViewConfig GetDbgViewCfg() const;

    void SetFileLogViewCfg(const FileLogViewConfig &cfg);
    FileLogViewConfig GetFileLogViewCfg() const;

    void SetFileSearchViewCfg(const FileSearchViewConfig &cfg);
    FileSearchViewConfig GetFileSearchViewCfg() const;

    //缓存最新使用的搜索串
    void EnterFilterStr(EM_LOGVIEW_TYPE eType, const std::mstring &str);
    void EnterSearchStr(EM_LOGVIEW_TYPE eType, const std::mstring &str);
private:
    LogViewConfigMgr() {}
    virtual ~LogViewConfigMgr() {}
    void InsertStrToList(const std::mstring &str, std::list<std::mstring> *set1) const;
    std::mstring GetCfgJsonPath() const;

private:
    GlobalConfig mGlobalCfg;
    DbgLogViewConfig mDbgViewCfg;
    FileLogViewConfig mFileLogViewCfg;
    FileSearchViewConfig mFileSearchViewCfg;

    Json::Value mJsonObj;
};
