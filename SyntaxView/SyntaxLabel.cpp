#include "SyntaxLabel.h"

map<HLabel, CSyntaxLabel *> CSyntaxLabel::msMgrSet;

CSyntaxLabel *CSyntaxLabel::GetLabelMgr(HLabel index) {
    map<HLabel, CSyntaxLabel *>::const_iterator it = msMgrSet.find(index);

    if (it == msMgrSet.end())
    {
        return NULL;
    }
    return it->second;
}

void CSyntaxLabel::RegisterLabel(HLabel index) {
    if (msMgrSet.end() == msMgrSet.find(index))
    {
        msMgrSet.insert(make_pair(index, new CSyntaxLabel()));
    }
}

void CSyntaxLabel::UnRegisterLabel(HLabel index) {
    msMgrSet.erase(index);
}

void CSyntaxLabel::RegisterParser(const string &label, pfnColouriseTextProc pfn) {
    mParserSet[label] = pfn;
}

void CSyntaxLabel::PushLabel(const void *ptr) {
    LabelCache *newNode = new LabelCache();
    LabelNode *pParam = (LabelNode *)ptr;
    newNode->mLabel = pParam->m_label;
    newNode->mContent = pParam->m_content;
    newNode->mStartPos = pParam->m_startPos;
    newNode->mEndPos = pParam->m_endPos;
    mNodeSet.push_back(newNode);
}

void CSyntaxLabel::ClearLabel() {
    mNodeSet.clear();
}

const LabelCache *CSyntaxLabel::GetLabelNode(int pos) const {
    //从上次分析的位置继续进行词法分析
    LabelCache *ptr = mNodeSet[mCurPos];
    if (pos >= (int)ptr->mStartPos && pos < (int)ptr->mEndPos)
    {
        return ptr;
    }
    return NULL;
}

void CSyntaxLabel::MoveNextPos() {
    mCurPos++;
}

void CSyntaxLabel::OnParserStr(size_t startPos, size_t size, StyleContextBase &sc) {
    size_t count2 = size;
    while (true) {
        const LabelCache *pLabelNode = GetLabelNode(startPos);
        if (!pLabelNode)
        {
            break;
        }

        size_t needSize = pLabelNode->mEndPos - startPos;
        size_t parserSize = (size_t)(count2 > needSize ? needSize : count2);

        map<string, pfnColouriseTextProc>::const_iterator it = mParserSet.find(pLabelNode->mLabel);
        if (it == mParserSet.end())
        {
            sc.SetState(1);
            sc.ForwardBytes(parserSize);
        } else {
            pfnColouriseTextProc pfnParser = it->second;
            const char *ptr1 = pLabelNode->mContent.c_str() + (startPos - pLabelNode->mStartPos);
            pfnParser(sc.GetStat(), startPos, ptr1, parserSize, &sc);

            if (count2 > needSize) 
            {
                MoveNextPos();
            }
        }

        count2 -= parserSize;
        startPos += parserSize;

        if ((int)count2 <= 0)
        {
            break;
        }
    }
    sc.Complete();
}

CSyntaxLabel::CSyntaxLabel() {
    mCurPos = 0;
}

CSyntaxLabel::~CSyntaxLabel() {
}