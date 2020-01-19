#include <Windows.h>
#include <fstream>
#include "GlobalDef.h"

using namespace std;
using namespace Json;

LogViewConfigMgr *LogViewConfigMgr::GetInst() {
    static LogViewConfigMgr *sPtr = NULL;

    if (NULL == sPtr) {
        sPtr = new LogViewConfigMgr();
    }
    return sPtr;
}

mstring LogViewConfigMgr::GetCfgJsonPath() const {
    extern mstring gCfgPath;
    mstring cfgPath(gCfgPath);
    cfgPath.path_append("CfgCache.json");
    return cfgPath;
}

void LogViewConfigMgr::SaveConfig() {
    Value jsonObj;
    Value globalObj;
    Value dbgViewObj;
    Value fileLogViewObj;
    Value fileSearchViewObj;
    list<mstring>::const_iterator it;

    globalObj["topmost"] = mGlobalCfg.mTopMost;
    globalObj["autoScroll"] = mGlobalCfg.mAutoScroll;
    globalObj["curView"] = mGlobalCfg.mCurView;
    jsonObj["globalCfg"] = globalObj;

    for (it = mDbgViewCfg.mFilterList.begin(); it != mDbgViewCfg.mFilterList.end(); it++) {
        dbgViewObj["fltList"].append(*it);
    }
    jsonObj["dbgViewCfg"] = dbgViewObj;

    for (it = mFileLogViewCfg.mFilterList.begin(); it != mFileLogViewCfg.mFilterList.end(); it++) {
        fileLogViewObj["fltList"].append(*it);
    }
    jsonObj["fileLogViewCfg"] = fileLogViewObj;

    for (it = mFileSearchViewCfg.mFilterList.begin(); it != mFileSearchViewCfg.mFilterList.end(); it++) {
        fileSearchViewObj["fltList"].append(*it);
    }
    jsonObj["fileSearchViewCfg"] = fileSearchViewObj;

    mstring cfgContent = StyledWriter().write(jsonObj);
    DeleteFileA(GetCfgJsonPath().c_str());
    ofstream fp(GetCfgJsonPath().c_str());

    fp << cfgContent;
}

void LogViewConfigMgr::SetGlobalCfg(const GlobalConfig &cfg) {
    mGlobalCfg = cfg;
}

GlobalConfig LogViewConfigMgr::GetGlobalCfg() const {
    return mGlobalCfg;
}

void LogViewConfigMgr::LoadConfig() {
    fstream fp(GetCfgJsonPath().c_str());

    if (!fp.is_open()) {
        return;
    }

    Value jsonObj;
    Reader().parse(fp, jsonObj);

    Value globalObj = jsonObj["globalCfg"];
    mGlobalCfg.mTopMost = globalObj["topmost"].asBool();
    mGlobalCfg.mAutoScroll = globalObj["autoScroll"].asBool();
    mGlobalCfg.mCurView = (EM_LOGVIEW_TYPE)globalObj["curView"].asInt();

    Value fileLogViewObj = jsonObj["fileLogViewCfg"];
    Value fltArray = fileLogViewObj["fltList"];
    size_t i = 0;
    for (i = 0; i < fltArray.size(); i++) {
        mFileLogViewCfg.mFilterList.push_back(fltArray[i].asString());
    }
}

void EnterDbgViewFilter(const std::mstring &filterStr);
void EnterFileLogViewFilter(const std::mstring &filterStr);
void EnterFileSearchViewFilter(const std::mstring &filterStr);