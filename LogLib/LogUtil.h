#pragma once
#include <Windows.h>
#include <vector>
#include "mstring.h"
#include "json/json.h"

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

VOID __stdcall PrintDbgInternal(LPCWSTR wszTarget, LPCSTR wszFile, DWORD dwLine, LPCWSTR wszFormat, ...);
#define dp(f, ...) PrintDbgInternal(L"LogSniff", __FILE__, __LINE__, f, ##__VA_ARGS__);

std::mstring GetStrFormJson(const Json::Value &json, const std::mstring &name);

int GetIntFromJson(const Value &json, const std::mstring &name);
std::ustring GetWindowStrW(HWND hwnd);
void CentreWindow(HWND hSrcWnd, HWND hDstWnd);

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