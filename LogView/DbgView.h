#include "SyntaxHlpr/SyntaxCache.h"

class CDbgView : public CSyntaxCache {
public:
    CDbgView();
    virtual ~CDbgView();

    bool CreateDbgView(HWND hParent, int x, int y, int cx, int cy);
    bool SetHightStr(const std::string &str);
private:
    void initDbgView();

    static void __stdcall DbgParser(
        int initStyle,
        unsigned int startPos,
        const char *ptr,
        int length,
        StyleContextBase *sc,
        void *param
        );
private:
    std::string mKeywordStr;
}; 