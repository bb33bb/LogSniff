#ifdef __linux__
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#else
#include <Shlwapi.h>
#include <iphlpapi.h>
#include "LogUtil.h"
#include "StrUtil.h"

#pragma comment(lib, "Iphlpapi.lib")
#endif

#include <stdio.h>
#include <string.h>

using namespace std;

void printDbgInternal(const char *tag, const char *file, int line, const char *fmt, ...)
{
    char format1[1024];
    char format2[1024];
    strcpy(format1, "[%hs][%hs.%d]%hs");
    strcat(format1, "\n");
#ifdef __linux__
    snprintf(format2, sizeof(format2), format1, tag, file, line, fmt);
#else
    _snprintf(format2, sizeof(format2), format1, tag, file, line, fmt);
#endif //__linux__

    char logInfo[1024];
    va_list vList;
    va_start(vList, fmt);
    vsnprintf(logInfo, sizeof(logInfo), format2, vList);
    va_end(vList);

#ifndef __linux__
    OutputDebugStringA(logInfo);
#else
    fprintf(stdout, logInfo);
#endif //__linux__
}

#ifndef __linux__
int GetIntFromJson(const Value &json, const std::mstring &name) {
    Value node = json[name];
    if (node.type() == intValue)
    {
        return node.asInt();
    }
    return 0;
}

std::ustring GetWindowStrW(HWND hwnd) {
    if (!IsWindow(hwnd))
    {
        return L"";
    }

    WCHAR buffer[256];
    buffer[0] = 0;
    int size = GetWindowTextLength(hwnd);
    if (size < 256)
    {
        GetWindowTextW(hwnd, buffer, 256);
        return buffer;
    } else {
        MemoryAlloc<WCHAR> alloc;
        WCHAR *ptr = alloc.GetMemory(size + 4);
        GetWindowTextW(hwnd, ptr, size + 4);
        return ptr;
    }
}

std::mstring __stdcall GetWindowStrA(HWND hwnd) {
    return WtoA(GetWindowStrW(hwnd));
}

std::mstring GetStrFormJson(const Value &json, const std::mstring &name) {
    Value node = json[name];
    if (node.type() != nullValue)
    {
        if (node.type() == stringValue)
        {
            return node.asString();
        } else {
            return FastWriter().write(node);
        }
    }
    return "";
}

PFILE_MAPPING_STRUCT __stdcall MappingFileA(LPCSTR fileName, BOOL bWrite, DWORD maxViewSize)
{
    PFILE_MAPPING_STRUCT pfms = NULL;

    do
    {
        if (!fileName)
        {
            break;
        }

        pfms = (PFILE_MAPPING_STRUCT)malloc(sizeof(FILE_MAPPING_STRUCT));
        if (!pfms)
        {
            break;
        }
        RtlZeroMemory(pfms, sizeof(FILE_MAPPING_STRUCT));

        pfms->hFile = CreateFileA(
            fileName,
            GENERIC_READ | (bWrite ? GENERIC_WRITE : 0),
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (INVALID_HANDLE_VALUE == pfms->hFile)
        {
            break;
        }

        if (!GetFileSizeEx(pfms->hFile, &(pfms->fileSize)) || !pfms->fileSize.QuadPart)
        {
            break;
        }

        if (!maxViewSize)
        {
            pfms->mappedSize = pfms->fileSize.QuadPart > 0xffffffff ? 0xffffffff : pfms->fileSize.LowPart;
        }
        else
        {
            pfms->mappedSize = pfms->fileSize.QuadPart > maxViewSize ? maxViewSize : pfms->fileSize.LowPart;
        }

        pfms->hMap = CreateFileMapping(pfms->hFile, NULL, bWrite ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);
        if (!pfms->hMap)
        {
            break;
        }

        pfms->lpView = MapViewOfFile(pfms->hMap, FILE_MAP_READ | (bWrite ? FILE_MAP_WRITE : 0), 0, 0, pfms->mappedSize);
        if (!pfms->lpView)
        {
            break;
        }
    } while (FALSE);

    return pfms;
}

void __stdcall CloseFileMapping(PFILE_MAPPING_STRUCT pfms)
{
    if (pfms)
    {
        if (pfms->lpView)
        {
            UnmapViewOfFile(pfms->lpView);
        }

        if (pfms->hMap)
        {
            CloseHandle(pfms->hMap);
        }

        if (INVALID_HANDLE_VALUE != pfms->hFile)
        {
            CloseHandle(pfms->hFile);
        }

        free((void*)pfms);
    }
}

void CentreWindow(HWND hSrcWnd, HWND hDstWnd)
{
    if (!hDstWnd)
    {
        hDstWnd = GetDesktopWindow();
    }

    RECT rt = {0};
    GetWindowRect(hDstWnd, &rt);
    RECT crt = {0};
    GetWindowRect(hSrcWnd, &crt);
    int iX = 0;
    int iY = 0;
    int icW = crt.right - crt.left;
    int iW = rt.right - rt.left;
    int icH = crt.bottom - crt.top;
    int iH = rt.bottom - rt.top;
    iX = rt.left + (iW - icW) / 2;
    iY = rt.top + (iH - icH) / 2;
    MoveWindow(hSrcWnd, iX, iY, icW, icH, TRUE);
}

BOOL GetAdapterSet(OUT vector<AdapterMsg> &nets)
{
    BOOL state = FALSE;
    PIP_ADAPTER_INFO adapterInfo = NULL;
    PIP_ADAPTER_INFO adapter = NULL;
    ULONG length = sizeof(IP_ADAPTER_INFO);
    adapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO));
    DWORD ret = GetAdaptersInfo(adapterInfo, &length);
    do 
    {
        if (ERROR_SUCCESS != ret)
        {
            if (ERROR_BUFFER_OVERFLOW == ret)
            {
                adapterInfo = (PIP_ADAPTER_INFO)realloc(adapterInfo, length);
            }
            else
            {
                break;
            }
        }

        if ((GetAdaptersInfo(adapterInfo, &length)) != NO_ERROR)
        {
            break;
        }
        adapter = adapterInfo;
        while (adapter)
        {
            AdapterMsg tmp;
            tmp.m_idex = adapter->ComboIndex;
            tmp.m_name = adapter->AdapterName;
            tmp.m_desc = adapter->Description;
            mstring vt;
            size_t i = 0;
            for (i = 0; i < adapter->AddressLength; i++)
            {
                if (i == (adapter->AddressLength - 1))
                {
                    vt.format("%.2X", (int)adapter->Address[i]);
                }
                else
                {
                    vt.format("%.2X-", (int) adapter->Address[i]);
                }
                tmp.m_mac += vt;
            }
            tmp.m_idex = adapter->Index;
            switch (adapter->Type)
            {
            case MIB_IF_TYPE_OTHER:
                tmp.m_type = "其它";
                break;
            case MIB_IF_TYPE_ETHERNET:
                tmp.m_type = "以太网";
                break;
            case MIB_IF_TYPE_TOKENRING:
                tmp.m_type = "令牌环";
                break;
            case MIB_IF_TYPE_FDDI:
                tmp.m_type = "FDDI";
                break;
            case MIB_IF_TYPE_PPP:
                tmp.m_type = "PPP";
                break;
            case MIB_IF_TYPE_LOOPBACK:
                tmp.m_type = "回路";
                break;
            case MIB_IF_TYPE_SLIP:
                tmp.m_type = "Slip";
                break;
            default:
                tmp.m_type = "未知网卡类型";
                break;
            }
            //ip
            tmp.m_ip = adapter->IpAddressList.IpAddress.String;
            //mask
            tmp.m_mask = adapter->IpAddressList.IpMask.String;
            //网关
            tmp.m_gateway = adapter->GatewayList.IpAddress.String;

            if (adapter->DhcpEnabled)
            {
                tmp.m_dhcp_enable = true;
            }
            else
            {
                tmp.m_dhcp_enable = false;
            }
            adapter = adapter->Next;
            nets.push_back(tmp);
            state = TRUE;
        }
    } while (FALSE);

    if (adapterInfo)
    {
        free(adapterInfo);
    }
    return state;
}
#endif //__linux__