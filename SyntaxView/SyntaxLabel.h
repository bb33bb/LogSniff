#pragma once
#include <string>
#include <map>
#include <vector>
#include "export.h"

using namespace std;

typedef const void* HLabel;

struct LabelCache {
    string mLabel;
    string mContent;

    size_t mStartPos;
    size_t mEndPos;
};

class CSyntaxLabel {
public:
    static CSyntaxLabel *GetLabelMgr(HLabel indee);
    static void RegisterLabel(HLabel index);
    static void UnRegisterLabel(HLabel  index);

    void RegisterParser(const string &label, pfnColouriseTextProc pfn);
    void PushLabel(const void *ptr);
    void ClearLabel();
    const LabelCache *GetLabelNode(int pos) const;
    void MoveNextPos();
    void OnParserStr(size_t startPos, size_t size, StyleContextBase &sc);

private:
    CSyntaxLabel();
    virtual ~CSyntaxLabel();

private:
    size_t mCurPos;
    vector<LabelCache *> mNodeSet;
    map<string, pfnColouriseTextProc> mParserSet;

private:
    static map<HLabel, CSyntaxLabel *> msMgrSet;
};