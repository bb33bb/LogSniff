#include "TextDecoder.h"

using namespace std;

CTextDecoder *CTextDecoder::GetInst() {
    static CTextDecoder *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CTextDecoder();
    }
    return sPtr;
}

CTextDecoder::CTextDecoder() {
}

CTextDecoder::~CTextDecoder() {
}

TextEncodeType CTextDecoder::GetTextType(const string &text) {
    TextEncodeType type = em_text_no_unicode;
    return type;
}

string CTextDecoder::GetTextStr(const string &text, TextEncodeType &type) {
    string result;
    return result;
}
