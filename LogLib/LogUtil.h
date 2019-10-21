#pragma once
#ifndef __linux__
#include <Windows.h>
#include <Tlhelp32.h>
#include <vector>
#include "mstring.h"
#include "json/json.h"
#endif //__linux__

#include <stdlib.h>

void printDbgInternal(const char *tag, const char *file, int line, const char *fmt, ...);
#define dp(f, ...) printDbgInternal("LogSniff", __FILE__, __LINE__, f, ##__VA_ARGS__);

#ifndef __linux__
typedef struct _FILE_MAPPING_STRUCT
{
    HANDLE hFile;
    HANDLE hMap;
    LPVOID lpView;
    LARGE_INTEGER fileSize;
    DWORD mappedSize;
} FILE_MAPPING_STRUCT, *PFILE_MAPPING_STRUCT;

PFILE_MAPPING_STRUCT __stdcall MappingFileA(LPCSTR fileName, BOOL bWrite = FALSE, DWORD maxViewSize = 1024 * 1024 * 64);
void __stdcall CloseFileMapping(PFILE_MAPPING_STRUCT pfms);

std::mstring GetStrFormJson(const Json::Value &json, const std::mstring &name);

int GetIntFromJson(const Value &json, const std::mstring &name);
std::mstring GetWindowStrA(HWND hwnd);
std::ustring GetWindowStrW(HWND hwnd);
void CentreWindow(HWND hSrcWnd, HWND hDstWnd);

class HandleAutoClose {
public:
    inline HandleAutoClose(HANDLE h) {
        mHandle = h;
    }

    inline virtual ~HandleAutoClose() {
        if (mHandle&& INVALID_HANDLE_VALUE != mHandle)
        {
            CloseHandle(mHandle);
        }
    }

private:
    HANDLE mHandle;
};

template <class T>
class MemoryAlloc {
public:
    MemoryAlloc() {
        mBuffer = NULL;
        mSize = 0;
    }

    virtual ~MemoryAlloc() {
        if (mBuffer)
        {
            delete []mBuffer;
        }
    }

    T *GetMemory(int size) {
        if (size < mSize)
        {
            return mBuffer;
        } else {
            if (mBuffer)
            {
                delete []mBuffer;
            }
            mSize = size;
            mBuffer = new T[size];
        }
        return mBuffer;
    }

    T *GetPtr() {
        return mBuffer;
    }

    int GetSize() {
        return mSize;
    }

private:
    T *mBuffer;
    int mSize;
};

struct AdapterMsg
{
    UINT m_com_idex;
    UINT m_idex;
    std::mstring m_name;
    std::mstring m_desc;
    std::mstring m_mac;
    std::mstring m_ip;
    std::mstring m_mask;
    std::mstring m_gateway;
    std::mstring m_type;
    bool m_dhcp_enable;
};

BOOL GetAdapterSet(std::vector<AdapterMsg> &nets);

typedef bool (* pfnFileHandler)(bool isDir, LPCSTR filePath, void *param);
BOOL EnumFiles(const std::mstring &dir, BOOL recursion, pfnFileHandler pfn, void *param);
std::mstring GetOSVersion();

std::mstring GetProcPathFromPid(DWORD pid);

typedef BOOL (WINAPI* pfnProcHandler)(PPROCESSENTRY32 info, void *param);
void ProcIterateProc(pfnProcHandler handler, void* lpParam);

BOOL GetFileVersion(LPCSTR pFile, std::mstring& ver);
BOOL ReleaseRes(const char *path, DWORD id, const char *type);
#endif //__linux__