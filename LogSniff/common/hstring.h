#ifndef _H_HSTRING
#define _H_HSTRING
#include <string>
#include <wchar.h>
#include "hmacros.h"

#if __cplusplus
extern "C" {
#endif // #if __cplusplus

// if parameter str is utf8 string, return value is 0
int HPLATFORM_CALL hstr_verify_utf8(const char* str);
wchar_t* HPLATFORM_CALL hstr_g2w(const char* gbk_str);
char* HPLATFORM_CALL hstr_g2u(const char* gbk_str);
char* HPLATFORM_CALL hstr_g2g(const char* gbk_str);
wchar_t* HPLATFORM_CALL hstr_u2w(const char* utf8_str);
char* HPLATFORM_CALL hstr_u2g(const char* utf8_str);
char* HPLATFORM_CALL hstr_u2u(const char* utf8_str);
char* HPLATFORM_CALL hstr_w2g(const wchar_t* ucs2_str);
char* HPLATFORM_CALL hstr_w2u(const wchar_t* ucs2_str);
wchar_t* HPLATFORM_CALL hstr_w2w(const wchar_t* ucs2_str);
wchar_t* HPLATFORM_CALL hstr_toggle_byteorder(const wchar_t* ucs2_str);
void HPLATFORM_CALL hstr_free(void* buff);
#define HSTR_FREE(str)                  hstr_free((void*)str)

typedef int (HPLATFORM_CALL* pfnSubstrHandler)(const char*, void*);
void HPLATFORM_CALL hstr_split(const char* str_orig, const char* str_divide, pfnSubstrHandler handler, void* param);

void HPLATFORM_CALL hstr_tolowercase(char* str);
void HPLATFORM_CALL hstr_touppercase(char* str);

// 格式化显示快捷函数，注意：返回结果最多不超过2048个字符，超过的会被自动截断
std::string FormatA(const char *format, ...);
std::wstring FormatW(const wchar_t *format, ...);
#if __cplusplus
}
#endif // #if __cplusplus

#endif // #ifndef _H_HSTRING
