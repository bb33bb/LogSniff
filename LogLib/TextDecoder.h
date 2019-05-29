#pragma once

#include <string>

enum TextEncodeType {
    em_text_gbk = 0,        //gbk����
    em_text_utf8,           //utf8����
    em_text_no_unicode,     //δ��,��unicode,gbk����utf8
    em_text_unicode_le,     //unicode little endian����
    em_text_unicode_be,     //unicode big endian����
    em_text_unknown         //δ֪����
};

class CTextDecoder {
public:
    static CTextDecoder *GetInst();
    TextEncodeType GetFileType(const std::string &filePath, int &bomLen);
    TextEncodeType GetTextType(const std::string &text);
    std::string GetTextStr(const std::string &text, TextEncodeType type);

private:
    bool IsUnicodeStr(const std::string &str) const;
    int GetUtf8Len(unsigned char c) const;
    bool IsUtf8Str(const std::string &str) const;

    CTextDecoder();
    virtual ~CTextDecoder();
};