#pragma once
#include <Windows.h>
#include <list>
#include "SyntaxView.h"
#include "export.h"
#include <LogLib/mstring.h>
#include <LogLib/locker.h>
#include <map>
#include <vector>

class CSyntaxCache : public SyntaxView, public RLocker {
public:
    CSyntaxCache();
    virtual ~CSyntaxCache();

    bool InitCache(const std::mstring &label, int interval);
    void PushToCache(const std::mstring &content);

    void SwitchWorkMode(int mode);
    bool SetKeyword(const std::mstring &keyWord);
    std::mstring GetKeyword();

    bool JmpNextPos(const std::mstring &str);
    bool JmpFrontPos(const std::mstring &str);
    bool JmpFirstPos(const std::mstring &str);
    bool JmpLastPos(const std::mstring &str);

    std::mstring GetViewStr(int startPos, int length) const;
    void OnViewUpdate() const;
    void UpdateView() const;

    //������л���
    void ClearCache();
private:
    std::mstring GetFilterStr(const std::mstring &content, const std::mstring &key) const;
    void OnFilter();
    static void CALLBACK TimerCache(HWND hwnd,
        UINT msg,
        UINT_PTR id,
        DWORD time
        );
    static void __stdcall LogParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc,
        void *param
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
    std::mstring mContent;
    std::mstring mShowData;
    std::mstring mKeyword;
    int mWorkMode;  //����ģʽ 0 ����ģʽ 1 ����ģʽ
};