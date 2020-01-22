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
    Value jsonObj(objectValue);
    Value globalObj(objectValue);
    Value dbgViewObj(objectValue);
    Value fileLogViewObj(objectValue);
    Value fileSearchViewObj(objectValue);
    Value arrayObj(arrayValue);
    list<mstring>::const_iterator it;

    globalObj["topmost"] = mGlobalCfg.mTopMost;
    globalObj["autoScroll"] = mGlobalCfg.mAutoScroll;
    globalObj["curView"] = mGlobalCfg.mCurView;
    jsonObj["globalCfg"] = globalObj;

    arrayObj.clear();
    for (it = mDbgViewCfg.mFilterList.begin(); it != mDbgViewCfg.mFilterList.end(); it++) {
        arrayObj.append(*it);
    }
    dbgViewObj["fltList"] = arrayObj;
    jsonObj["dbgViewCfg"] = dbgViewObj;

    arrayObj.clear();
    for (it = mFileLogViewCfg.mFilterList.begin(); it != mFileLogViewCfg.mFilterList.end(); it++) {
        arrayObj.append(*it);
    }
    fileLogViewObj["fltList"] = arrayObj;
    jsonObj["fileLogViewCfg"] = fileLogViewObj;

    arrayObj.clear();
    for (it = mFileSearchViewCfg.mFilterList.begin(); it != mFileSearchViewCfg.mFilterList.end(); it++) {
        arrayObj.append(*it);
    }
    fileSearchViewObj["fltList"] = arrayObj;
    jsonObj["fileSearchViewCfg"] = fileSearchViewObj;

    mstring cfgContent = StyledWriter().write(jsonObj);
    DeleteFileA(GetCfgJsonPath().c_str());

    ofstream fp(GetCfgJsonPath().c_str());
    fp << cfgContent;
    fp.close();
}

void LogViewConfigMgr::SetGlobalCfg(const GlobalConfig &cfg) {
    mGlobalCfg = cfg;
    SaveConfig();
}

GlobalConfig LogViewConfigMgr::GetGlobalCfg() const {
    return mGlobalCfg;
}

void LogViewConfigMgr::SetDbgViewCfg(const DbgLogViewConfig &cfg) {
    mDbgViewCfg = cfg;
}

DbgLogViewConfig LogViewConfigMgr::GetDbgViewCfg() const {
    return mDbgViewCfg;
}

FileLogViewConfig LogViewConfigMgr::GetFileLogViewCfg() const {
    return mFileLogViewCfg;
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

    size_t i = 0;
    Value fltArray;
    Value dbgLogViewObj = jsonObj["dbgViewCfg"];
    fltArray = dbgLogViewObj["fltList"];
    for (i = 0; i < fltArray.size(); i++) {
        mDbgViewCfg.mFilterList.push_back(fltArray[i].asString());
    }

    Value fileLogViewObj = jsonObj["fileLogViewCfg"];
    fltArray = fileLogViewObj["fltList"];
    for (i = 0; i < fltArray.size(); i++) {
        mFileLogViewCfg.mFilterList.push_back(fltArray[i].asString());
    }

    Value fileSearchObj = jsonObj["fileSearchViewCfg"];
    fltArray = fileSearchObj["fltList"];
    for (i = 0; i < fltArray.size(); i++) {
        mFileSearchViewCfg.mFilterList.push_back(fltArray[i].asString());
    }
}

//插入搜索串,规则如下
//如果集合中存在,将其顺序调整为第一个,如果不存在,直接将其追加到头部
void LogViewConfigMgr::InsertStrToList(const mstring &str, list<mstring> *set1) const {
    for (list<mstring>::const_iterator it = set1->begin(); it != set1->end(); it++) {
        if (*it == str) {
            set1->erase(it);
            break;
        }
    }
    set1->push_front(str);
}

//缓存最新使用的搜索串
void LogViewConfigMgr::EnterFilterStr(EM_LOGVIEW_TYPE eType, const std::mstring &str) {
    list<mstring> *set1 = NULL;
    switch (eType) {
        case EM_VIEW_DBGLOG:
            set1 = &mDbgViewCfg.mFilterList;
            break;
        case EM_VIEW_FILELOG:
            set1 = &mFileLogViewCfg.mFilterList;
            break;
        case EM_VIEW_FILESEARCH:
            set1 = &mFileSearchViewCfg.mFilterList;
            break;
        default:
            return;
    }

    if (set1) {
        InsertStrToList(str, set1);
        SaveConfig();
    }
}

void LogViewConfigMgr::EnterSearchStr(EM_LOGVIEW_TYPE eType, const std::mstring &str) {
}