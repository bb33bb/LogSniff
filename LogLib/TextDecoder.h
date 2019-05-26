#pragma once

#include <string>

enum TextEncodeType {
    em_text_gbk = 0,        //gbk编码
    em_text_utf8,           //utf8编码
    em_text_no_unicode,     //未决,非unicode,gbk或者utf8
    em_text_unicode         //unicode编码
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