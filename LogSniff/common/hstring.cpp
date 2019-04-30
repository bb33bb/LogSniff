#include <stdlib.h>
#include "hstring.h"
#if _MSC_VER
#  include <Windows.h>
#else // #if _MSC_VER
#  include <iconv.h>
#endif // #if _MSC_VER

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

using namespace std;

//   |   UCS2 code  |  UTF-8 code
// n |    (hex)     |    (binary)
// --+--------------+---------------------------
// 1 | 0000 - 007F  |                   0xxxxxxx
// 2 | 0080 - 07FF  |          110xxxxx 10xxxxxx
// 3 | 0800 - FFFF  | 1110xxxx 10xxxxxx 10xxxxxx
static int _utf8_len(unsigned char hdr_char)
{
    if (hdr_char >= 0xe0)
    {
        return 3;
    }

    if (hdr_char >= 0xc0)
    {
        return 2;
    }

    // 01111111 - 11011111 is invalid utf8 hdr
    return (hdr_char <= 0x80) ? 1 : 0;
}

static wchar_t _utf8_to_ucs2char(const char* utf8_hdr, int len)
{
    wchar_t c = 0;
    unsigned char* tmp = (unsigned char*)&c;

    if (1 == len)
    {
        tmp[0] = utf8_hdr[0];
    }
    else if (2 == len)
    {
        tmp[0] |= (utf8_hdr[1] & 0x3f);
        tmp[0] |= ((utf8_hdr[0] & 0x03) << 6);
        tmp[1] |= ((utf8_hdr[0] & 0x3c) >> 2);
    }
    else if (3 == len)
    {
        tmp[0] |= (utf8_hdr[2] & 0x3f);
        tmp[0] |= ((utf8_hdr[1] & 0x03) << 6);
        tmp[1] |= ((utf8_hdr[1] & 0x3c) >> 2);
        tmp[1] |= ((utf8_hdr[0] & 0x0f) << 4);
    }

    return c;
}

static void _ucs2_to_utf8_char(wchar_t ucs2_char, unsigned char* utf8_buff)
{
    unsigned char* tmp = (unsigned char*)&ucs2_char;
    if (ucs2_char <= 0x7F)
    {
        utf8_buff[0] = (char)ucs2_char;
    }
    else if (ucs2_char <= 0x7FF)
    {
        utf8_buff[1] = (0x80 | tmp[0] & 0x3f);
        utf8_buff[0] = (((tmp[0] & 0xc0) >> 6) | ((tmp[1] & 0x07) << 2) | 0xc0);
    }
    else
    {
        utf8_buff[2] = (0x80 | (tmp[0] & 0x3f));
        utf8_buff[1] = (tmp[0] & 0xc0) >> 6;
        utf8_buff[1] |= (((tmp[1] & 0x0f) << 2) | 0x80);
        utf8_buff[0] = (((tmp[1] & 0xf0) >> 4) | 0xe0);
    }
}

#if _MSC_VER
#else
static char* _u2g_g2u(const char* orig_str, int u2g)
{
    char* ret = 0;
    int state = 1;
    char* tmp = 0;
    iconv_t converter = (iconv_t)-1;

    do
    {
        size_t in_len = strlen(orig_str);
        size_t out_len = 0;
        size_t alloc_len = u2g ? (in_len + 1) : (in_len * 2 + 1);

        if (u2g)
        {
            converter = iconv_open("gbk", "utf8");
        }
        else
        {
            converter = iconv_open("utf8", "gbk");
        }

        if ((iconv_t)-1 == converter)
        {
            break;
        }

        ret = (char*)malloc(alloc_len * sizeof(char));
        if (!ret)
        {
            break;
        }
        out_len = alloc_len;
        tmp = ret;

        if ((size_t)-1 == iconv(converter, (char**)&orig_str, &in_len, (char**)&tmp, &out_len))
        {
            break;
        }
        ret[alloc_len - out_len] = 0;

        state = 0;
    } while (0);

    if ((iconv_t)-1 != converter)
    {
        iconv_close(converter);
    }

    if (0 != state && ret)
    {
        free((void*)ret);
        ret = 0;
    }

    return ret;
}
#endif

// 0x10xxxxxx
static int _is_valid_utf8_body(unsigned char c)
{
    if ((c >= 0x80) && (c < 0xc0))
    {
        return 1;
    }

    return 0;
}

int HPLATFORM_CALL hstr_verify_utf8(const char* str)
{
    const char* hdr = str;
    int state = 0;

    while (*hdr)
    {
        int char_len = _utf8_len((unsigned char)*hdr);
        if (!char_len)
        {
            state = 1;
            break;
        }

        if (2 == char_len)
        {
            if (!_is_valid_utf8_body((unsigned char)hdr[1]))
            {
                state = 1;
                break;
            }
        }
        else if (3 == char_len)
        {
            if (!_is_valid_utf8_body((unsigned char)hdr[1]) || !_is_valid_utf8_body((unsigned char)hdr[2]))
            {
                state = 1;
                break;
            }
        }

        hdr += char_len;
    }

    return state;
}

#if _MSC_VER
wchar_t* HPLATFORM_CALL hstr_g2w(const char* gbk_str)
{
    int needSize = 0;
    wchar_t* ret = 0;

    if (!gbk_str)
    {
        return 0;
    }

    needSize = MultiByteToWideChar(GetACP(), 0, gbk_str, -1, 0, 0);
    ret = (wchar_t*)malloc((needSize + 1) * sizeof(wchar_t));
    if (ret)
    {
        RtlZeroMemory(ret, (needSize + 1) * sizeof(wchar_t));
        MultiByteToWideChar(GetACP(), 0, gbk_str, -1, ret, needSize);
    }

    return ret;
}

char* HPLATFORM_CALL hstr_g2u(const char* gbk_str)
{
    wchar_t* ucs2_str = hstr_g2w(gbk_str);
    if (ucs2_str)
    {
        char* utf8_str = hstr_w2u(ucs2_str);
        HSTR_FREE(ucs2_str);
        return utf8_str;
    }

    return 0;
}
#else // #if _MSC_VER
wchar_t* HPLATFORM_CALL hstr_g2w(const char* gbk_str)
{
    char* utf8_str = hstr_g2u(gbk_str);
    if (utf8_str)
    {
        wchar_t* ucs2_str = hstr_u2w(utf8_str);
        HSTR_FREE(utf8_str);
        return ucs2_str;
    }

    return 0;
}

char* HPLATFORM_CALL hstr_g2u(const char* gbk_str)
{
    return _u2g_g2u(gbk_str, 0);
}
#endif // #if _MSC_VER

char* HPLATFORM_CALL hstr_g2g(const char* gbk_str)
{
    char* ret = 0;

    if (gbk_str)
    {
        ret = (char*)malloc(HSTR_LEN(gbk_str) + sizeof(char));
        if (ret)
        {
            strcpy(ret, gbk_str);
        }
    }

    return ret;
}

wchar_t* HPLATFORM_CALL hstr_u2w(const char* utf8_str)
{
    int len = HSTR_LEN(utf8_str);
    int cursor = 0;
    int utf8_len = 0;
    int state = 1;
    const char* hdr_char = 0;
    wchar_t* ucs2_str = 0;

    if (!len)
    {
        return 0;
    }

    ucs2_str = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    if (!ucs2_str)
    {
        return ucs2_str;
    }
    hdr_char = utf8_str;

    while (1)
    {
        utf8_len = _utf8_len(*hdr_char);
        if (!utf8_len)
        {
            break;
        }

        if (utf8_len + hdr_char > utf8_str + len)
        {
            break;
        }

        ucs2_str[cursor++] = _utf8_to_ucs2char(hdr_char, utf8_len);
        if (utf8_len + hdr_char == utf8_str + len)
        {
            state = 0;
            break;
        }

        hdr_char += utf8_len;
    }

    if (0 != state)
    {
        free((void*)ucs2_str);
        ucs2_str = 0;
    }
    else
    {
        ucs2_str[cursor] = 0;
    }

    return ucs2_str;
}

#if _MSC_VER
char* HPLATFORM_CALL hstr_u2g(const char* utf8_str)
{
    wchar_t* ucs2_str = hstr_u2w(utf8_str);

    if (ucs2_str)
    {
        char* gbk_str = hstr_w2g(ucs2_str);
        HSTR_FREE(ucs2_str);
        return gbk_str;
    }

    return 0;
}

char* HPLATFORM_CALL hstr_w2g(const wchar_t* ucs2_str)
{
    int needSize = 0;
    char* ret = 0;

    if (!ucs2_str)
    {
        return 0;
    }

    needSize = WideCharToMultiByte(GetACP(), 0, ucs2_str, -1, 0, 0, 0, 0);
    ret = (char*)malloc(needSize + sizeof(char));
    if (ret)
    {
        RtlZeroMemory(ret, needSize + 1);
        WideCharToMultiByte(GetACP(), 0, ucs2_str, -1, ret, needSize, 0, 0);
    }

    return ret;
}
#else // #if _MSC_VER
char* HPLATFORM_CALL hstr_u2g(const char* utf8_str)
{
    return _u2g_g2u(utf8_str, 1);
}

char* HPLATFORM_CALL hstr_w2g(const wchar_t* ucs2_str)
{
    char* utf8_str = hstr_w2u(ucs2_str);
    if (utf8_str)
    {
        char* gbk_str = hstr_u2g(utf8_str);
        HSTR_FREE(utf8_str);
        return gbk_str;
    }

    return 0;
}
#endif // #if _MSC_VER

char* HPLATFORM_CALL hstr_u2u(const char* utf8_str)
{
    return hstr_g2g(utf8_str);
}

char* HPLATFORM_CALL hstr_w2u(const wchar_t* ucs2_str)
{
    int len = (int)wcslen(ucs2_str);
    int location = 0;
    char* utf8_str = 0;

    utf8_str = (char*)malloc(len * 3 + 1);
    if (!utf8_str)
    {
        return utf8_str;
    }
    utf8_str[0] = 0;

    while (location < len)
    {
        unsigned char utf8_buff[4] = {0};
        _ucs2_to_utf8_char(ucs2_str[location++], utf8_buff);
        strcat(utf8_str, (const char*)utf8_buff);
    }

    return utf8_str;
}

wchar_t* HPLATFORM_CALL hstr_w2w(const wchar_t* ucs2_str)
{
    wchar_t* ret = 0;

    if (ucs2_str)
    {
        ret = (wchar_t*)malloc(sizeof(wchar_t) * (wcslen(ucs2_str) + 1));
        if (ret)
        {
            wcscpy(ret, ucs2_str);
        }
    }

    return ret;
}

wchar_t* HPLATFORM_CALL hstr_toggle_byteorder(const wchar_t* ucs2_str)
{
    int len = ucs2_str ? wcslen(ucs2_str) : 0;
    wchar_t* ret = 0;
    int i = 0;

    if (!len)
    {
        return ret;
    }

    ret = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (ret)
    {
        for (i = 0; i < len; ++i)
        {
            ret[i] = (ucs2_str[i] << 8) | (ucs2_str[i] >> 8);
        }
    }

    return ret;
}

void HPLATFORM_CALL hstr_free(void* buff)
{
    free(buff);
}

void HPLATFORM_CALL hstr_split(const char* str_orig, const char* str_divide, pfnSubstrHandler handler, void* param)
{
    char* cur_str = 0;
    int idx = 0;

    if (!str_orig || !str_divide || !handler)
    {
        return;
    }

    cur_str = (char*)malloc(HSTR_LEN(str_orig) + 1);
    if (!cur_str)
    {
        return;
    }
    cur_str[0] = 0;

    while (*str_orig)
    {
        if (strchr(str_divide, *str_orig))
        {
            if (idx)
            {
                int ret = handler(cur_str, param);
                cur_str[0] = 0;
                idx = 0;

                if (!ret)
                {
                    break;
                }
            }
        }
        else
        {
            cur_str[idx++] = *str_orig;
            cur_str[idx] = 0;
        }

        str_orig++;
    }

    if (cur_str[0])
    {
        handler(cur_str, param);
    }

    free((void*)cur_str);
}

void HPLATFORM_CALL hstr_tolowercase(char* str)
{
    while (*str)
    {
        if (*str >= 'A' && *str <= 'Z')
        {
            *str = *str | 32;
        }

        str++;
    }
}

void HPLATFORM_CALL hstr_touppercase(char* str)
{
    while (*str)
    {
        if (*str >= 'a' && *str <= 'z')
        {
            *str = *str & (~32);
        }

        str++;
    }
}

std::string FormatA(const char *format, ...)
{
    char szText[2048] = {0};
    va_list val;

    va_start(val, format);
    vsnprintf(szText, sizeof(szText), format, val);
    va_end(val);

    return szText;
}

std::wstring FormatW(const wchar_t *format, ...)
{
    wchar_t wszText[2048] = {0};
    va_list val;

    va_start(val, format);
    vswprintf(wszText, sizeof(wszText) / sizeof(wchar_t), format, val);
    va_end(val);

    return wszText;
}