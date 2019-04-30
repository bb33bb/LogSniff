#ifndef _H_HCONST
#define _H_HCONST

#include <string.h>
#if _MSC_VER
#  include <Windows.h>
#else // #if _MSC_VER
#  include <limits.h>
#endif // #if _MSC_VER

#if _MSC_VER
#  define HPLATFORM_CALL                WINAPI
#  define HSNPRINTF                     wnsprintfA
#  define HVSNPRINTF                    wvnsprintfA
#  define HSTRICMP(a, b)                stricmp(a, b)
#  define HPATHSEPARATOR                '\\'
#else // #if _MSC_VER
#  define HPLATFORM_CALL
#  define HSNPRINTF                     snprintf
#  define HVSNPRINTF                    vsnprintf
#  define HSTRICMP(a, b)                strcasecmp(a, b)
#  define HPATHSEPARATOR                '/'
#endif // #if _MSC_VER

#define HEQUAL(a, b)                    (a) && (b) && (0 == strcmp(a, b))
#define HIEQUAL(a, b)                   (a) && (b) && (0 == HSTRICMP(a, b))
#define HSTR_LEN(s)                     (s ? strlen(s) : 0)
#define HARRSIZEOF(arr)                 (sizeof((arr)) / sizeof((arr)[0]))
#define HEXPAND_ARG(a)                  a, HARRSIZEOF(a)
#define HTIME_FORMAT                    "%04d-%02d-%02d %02d:%02d:%02d"
#define HYMD_FORMAT                     "%04d-%02d-%02d"
#define HHMS_FORMAT                     "%02d:%02d:%02d"
#define HTIME2STR(t, s)                 HSNPRINTF(HEXPAND_ARG(s), HTIME_FORMAT, HEXPAND_TIME(t))
#define HTIME2STRP(t, s)                HSNPRINTF(HEXPAND_ARG(s), HTIME_FORMAT, HEXPAND_TIMEP(t))
#define HYMD2STR(t, s)                  HSNPRINTF(HEXPAND_ARG(s), HYMD_FORMAT, HEXPAND_YMD(t))
#define HYMD2STRP(t, s)                 HSNPRINTF(HEXPAND_ARG(s), HYMD_FORMAT, HEXPAND_YMDP(t))
#define HHMS2STR(t, s)                  HSNPRINTF(HEXPAND_ARG(s), HHMS_FORMAT, HEXPAND_HMS(t))
#define HHMS2STRP(t, s)                 HSNPRINTF(HEXPAND_ARG(s), HHMS_FORMAT, HEXPAND_HMSP(t))

#if _MSC_VER
#  define HEXPAND_TIME(t)               (unsigned short)t.wYear, (unsigned short)t.wMonth, (unsigned short)t.wDay, (unsigned short)t.wHour, (unsigned short)t.wMinute, (unsigned short)t.wSecond
#  define HEXPAND_TIMEP(t)              (unsigned short)(t->wYear), (unsigned short)(t->wMonth), (unsigned short)(t->wDay), (unsigned short)(t->wHour), (unsigned short)(t->wMinute), (unsigned short)(t->wSecond)
#  define HEXPAND_YMD(t)                (unsigned short)t.wYear, (unsigned short)t.wMonth, (unsigned short)t.wDay
#  define HEXPAND_YMDP(t)               (unsigned short)t->wYear, (unsigned short)t->wMonth, (unsigned short)t->wDay
#  define HEXPAND_HMS(t)                (unsigned short)t.wHour, (unsigned short)t.wMinute, (unsigned short)t.wSecond
#  define HEXPAND_HMSP(t)               (unsigned short)t->wHour, (unsigned short)t->wMinute, (unsigned short)t->wSecond
#  define HCURTIME2STR(s)               do {SYSTEMTIME t; GetLocalTime(&t); HTIME2STR(t, s);} while (0)
#  define HCURYMD2STR(s)                do {SYSTEMTIME t; GetLocalTime(&t); HYMD2STR(t, s);} while (0)
#  define HCURHMS2STR(s)                do {SYSTEMTIME t; GetLocalTime(&t); HHMS2STR(t, s);} while (0)
#else // #if _MSC_VER
#  define HEXPAND_TIME(t)               (unsigned short)t.tm_year + 1900, (unsigned short)t.tm_mon + 1, (unsigned short)t.tm_mday, (unsigned short)t.tm_hour, (unsigned short)t.tm_min, (unsigned short)t.tm_sec
#  define HEXPAND_TIMEP(t)              (unsigned short)t->tm_year + 1900, (unsigned short)t->tm_mon + 1, (unsigned short)t->tm_mday, (unsigned short)t->tm_hour, (unsigned short)t->tm_min, (unsigned short)t->tm_sec
#  define HEXPAND_YMD(t)                (unsigned short)t.tm_year + 1900, (unsigned short)t.tm_mon + 1, (unsigned short)t.tm_mday
#  define HEXPAND_YMDP(t)               (unsigned short)t->tm_year + 1900, (unsigned short)t->tm_mon + 1, (unsigned short)t->tm_mday
#  define HEXPAND_HMS(t)                (unsigned short)t.tm_hour, (unsigned short)t.tm_min, (unsigned short)t.tm_sec
#  define HEXPAND_HMSP(t)               (unsigned short)t->tm_hour, (unsigned short)t->tm_min, (unsigned short)t->tm_sec
#  define HCURTIME2STR(s)               do {time_t tt; struct tm* t; time(&tt); t = localtime(&tt); HTIME2STRP(t, s);} while (0)
#  define HCURYMD2STR(s)                do {time_t tt; struct tm* t; time(&tt); t = localtime(&tt); HYMD2STRP(t, s);} while (0)
#  define HCURHMS2STR(s)                do {time_t tt; struct tm* t; time(&tt); t = localtime(&tt); HHMS2STRP(t, s);} while (0)
#endif // #if _MSC_VER

#if _MSC_VER
#  define HMAX_PATH                     MAX_PATH
#else // #if _MSC_VER
#  define HMAX_PATH                     PATH_MAX
#endif // #if _MSC_VER

#define HSIZE_B(s)                      s
#define HSIZE_K(s)                      HSIZE_B(s) * 1024
#define HSIZE_M(s)                      HSIZE_K(s) * 1024
#define HSIZE_G(s)                      (long long)HSIZE_M(s) * 1024

#define GD_IPFORMAT                     "%d.%d.%d.%d"
#define GD_IPFORMAT2                    "%d.%d.%d.%d:%d"
#define EXPAND_IP(i)                    (unsigned char)(i), (unsigned char)(i >> 8), (unsigned char)(i >> 16), (unsigned char)(i >> 24)
#define REXPAND_IP(i)                   (unsigned char)(i >> 24), (unsigned char)(i >> 16), (unsigned char)(i >> 8), (unsigned char)(i)

#endif // #ifndef _H_HCONST
