#pragma once
#include <Windows.h>
#include <list>
#include "SyntaxView.h"
#include "export.h"
#include <LogLib/mstring.h>
#include <LogLib/locker.h>
#include <map>

class CSyntaxCache : public SyntaxView, public RLocker {
public:
    CSyntaxCache();
    virtual ~CSyntaxCache();

    bool InitCache(const std::mstring &label, int interval);
    void PushToCache(const std::mstring &content);
private:
    static void CALLBACK TimerCache(HWND hwnd,
        UINT msg,
        UINT_PTR id,
        DWORD time
        );

    struct DataCache {
        std::mstring mLabel;
        std::mstring mContent;
        void *mParam;

        DataCache() {
            mParam = NULL;
        }
    };
private:
    int mInterval;
    std::mstring mLabel;
    std::mstring mCache;
    static std::map<HWND, CSyntaxCache *> msTimerCache;
};
