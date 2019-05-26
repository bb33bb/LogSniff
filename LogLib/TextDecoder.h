#pragma once

#include <string>

enum TextEncodeType {
    em_text_gbk = 0,        //gbk����
    em_text_utf8,           //utf8����
    em_text_no_unicode,     //δ��,��unicode,gbk����utf8
    em_text_unicode         //unicode����
};

class CTextDecoder {
public:
    static CTextDecoder *GetInst();
    TextEncodeType GetTextType(const std::string &text);
    std::string GetTextStr(const std::string &text, TextEncodeType &type);

private:
    CTextDecoder();
    virtual ~CTextDecoder();
};