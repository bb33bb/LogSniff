#include "TextDecoder.h"
#include "StrUtil.h"

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

TextEncodeType CTextDecoder::GetFileType(const string &filePath, int &bomLen) {
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (!fp)
    {
        return em_text_unknown;
    }

    unsigned char bom[3] = {0};
    fread(bom, 1, 3, fp);

    TextEncodeType type = em_text_unknown;
    if (bom[0] == 0xff && bom[1] == 0xfe)
    {
        type = em_text_unicode_le;
        bomLen = 2;
    } else if (bom[0] == 0xfe && bom[1] == 0xff)
    {
        type = em_text_unicode_be;
        bomLen = 2;
    } else if (bom[0] == 0xef && bom[1] == 0xbb && bom[2] == 0xbf)
    {
        type = em_text_utf8;
        bomLen = 3;
    } else {
        //尝试通过文件内容识别文件编码格式
        char buff[4096];
        fseek(fp, 0, SEEK_SET);

        size_t count = fread(buff, 1, sizeof(buff), fp);
        if (count > 0 && count <= sizeof(buff))
        {
            type = GetTextType(string(buff, count));
        }
    }
    fclose(fp);
    return type;
}

TextEncodeType CTextDecoder::GetTextType(const string &text) {
    TextEncodeType type = em_text_unknown;

    if (IsUnicodeStr(text))
    {
        return em_text_unicode_le;
    } else if (IsUtf8Str(text))
    {
        return em_text_utf8;
    }
    return type;
}

string CTextDecoder::GetTextStr(const string &text, TextEncodeType type) {
    string result;
    switch (type) {
        case em_text_gbk:
            result = text;
            break;
        case em_text_utf8:
            result = UtoA(text);
            break;
        case em_text_unicode_le:
            result = WtoA((const wchar_t *)text.c_str());
            break;
        default:
            //未决类型默认为gbk
            result = text;
            break;
    }
    return result;
}

bool CTextDecoder::IsUnicodeStr(const std::string &str) const {
    int count = 0;
    for (size_t i = 0 ; i < str.size() ; i++)
    {
        if (0x00 == str[i])
        {
            count++;

            if (count >= 2) {
                return true;
            }
        }
    }
    return false;
}

//   |   UCS2 code  |  UTF-8 code
// n |    (hex)     |    (binary)
// --+--------------+---------------------------
// 1 | 0000 - 007F  |                   0xxxxxxx
// 2 | 0080 - 07FF  |          110xxxxx 10xxxxxx
// 3 | 0800 - FFFF  | 1110xxxx 10xxxxxx 10xxxxxx
int CTextDecoder::GetUtf8Len(unsigned char c) const {
    if (c >= 0xe0)
    {
        return 3;
    }

    if (c >= 0xc0)
    {
        return 2;
    }

    // 01111111 - 11011111 is invalid utf8 hdr
    return (c <= 0x80) ? 1 : 0;
}

bool CTextDecoder::IsUtf8Str(const std::string &str) const {
    for (int i = 0 ; i < (int)str.size() ;)
    {
        char c = str[i];
        char c2 = 0x00;
        char c3 = 0x00;
        int len = GetUtf8Len(c);

        if (0 == len)
        {
            return false;
        }

        if (2 == len)
        {
            if (i >= (int)str.size() - 1)
            {
                return false;
            }

            c2 = str[i + 1];
            if (!((c2 >= 0x80) && (c2 < 0xc0)))
            {
                return false;
            }
            i += 2;
            continue;
        }

        if (3 == len)
        {
            if (i >= (int)str.size() - 2)
            {
                return false;
            }

            c2 = str[i + 1];
            c3 = str[i + 2];
            if (!(c2 >= 0x80 && (c2 < 0xc0)) || !(c3 >= 0x80 && (c3 < 0xc0)))
            {
                return false;
            }
            i += 3;
            continue;
        }
        i++;
    }
    return true;
}
