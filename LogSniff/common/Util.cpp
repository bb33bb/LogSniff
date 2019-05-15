#include "Util.h"
#include "hstring.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <list>
#include <string>

using namespace std;

#ifdef __linux__
#define _snprintf snprintf  //linux has no _snprintf
#endif //_snprintf

bool EnumFiles(const char *path, bool subDir, pfnFileCallback pfn, void *param) {
    DIR *dir = 0;
    struct dirent *ptr = 0;
    char base[1024];
    base[0] = 0;
    
    list<string> pathSet;
    pathSet.push_back(path);
    bool endEnum = false;
    while (!pathSet.empty()) {
        string tmp = *pathSet.begin();
        pathSet.pop_front();

        if (NULL == (dir = opendir(tmp.c_str()))) {
            continue;
        }

        string baseDir = tmp;
        if (baseDir[baseDir.size() - 1] != '/')
        {
            baseDir += '/';
        }

        while (NULL != (ptr = readdir(dir)))
        {
            if (strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..") == 0) {
                continue;
            }
            else if(ptr->d_type == 8)    //file
            {
                string filePath = baseDir + ptr->d_name;
                if (!pfn(false, filePath.c_str(), param))
                {
                    endEnum = true;
                }
            }
            else if(ptr->d_type == 10)   //link file
            {}
            else if(ptr->d_type == 4)    //dir
            {
                string subDirPath = baseDir + ptr->d_name;
                if (!pfn(true, subDirPath.c_str(), param))
                {
                    endEnum = true;
                }

                if (subDir)
                {
                    pathSet.push_back(subDirPath);
                }
            }

            if (endEnum)
            {
                break;
            }
        }
        closedir(dir);

        if (endEnum)
        {
            break;
        }
    }
    return true;
}

void PrintDbgInternal(const char *tag, const char *file, unsigned int line, const char *fmt, ...)
{
    extern bool gDebugMode;
    if (!gDebugMode)
    {
        return;
    }

    char format1[1024];
    char format2[1024];
    strcpy(format1, "[%hs][%hs.%d]%hs");
    strcat(format1, "\n");
    _snprintf(format2, sizeof(format2), format1, tag, file, line, fmt);

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