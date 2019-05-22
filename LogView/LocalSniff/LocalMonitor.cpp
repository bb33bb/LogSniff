#include <WinSock2.h>
#include "LocalMonitor.h"
#include "WinFileNoitfy.h"
#include "../LogReceiver.h"
#include <LogLib/LogUtil.h>

using namespace std;

CLocalMonitor *CLocalMonitor::GetInst() {
    static CLocalMonitor *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLocalMonitor();
    }
    return sPtr;
}

bool CLocalMonitor::Init(CMonitorEvent *listener) {
    mListener = listener;
    CWinFileNotify::GetInst()->InitNotify();
    mInit = false;
    return true;
}

bool CLocalMonitor::Run(const LogServDesc &servDesc) {
    list<mstring> added;
    list<mstring> deled;

    if (!mInit)
    {
        added = servDesc.mLocalServDesc.mPathSet;
    } else {
        list<mstring>::const_iterator it1, it2;
        const list<mstring> &set1 = servDesc.mLocalServDesc.mPathSet;
        const list<mstring> &set2 = mCfg.mLocalServDesc.mPathSet;

        bool flag = false;
        for (it1 = set1.begin() ; it1 != set1.end() ; it1++)
        {
            flag = false;
            for (it2 = set2.begin() ; it2 != set2.end() ; it2++)
            {
                if (*it1 == *it2)
                {
                    flag = true;
                    break;
                }
            }

            if (!flag)
            {
                added.push_back(*it1);
            }
        }

        for (it1 = set2.begin() ; it1 != set2.end() ; it1++)
        {
            flag = false;
            for (it2 = set1.begin() ; it2 != set1.end() ; it2++)
            {
                if (*it1 == *it2)
                {
                    flag = true;
                    break;
                }
            }

            if (!flag)
            {
                deled.push_back(*it1);
            }
        }
    }

    list<mstring>::const_iterator it;
    for (it = added.begin() ; it != added.end() ; it++)
    {
        AddPath(*it);
    }

    for (it = deled.begin() ; it != deled.end() ; it++)
    {
        //½â³ý¼à¿Ø
    }
    return true;
}

bool CLocalMonitor::Stop() {
    return true;
}

bool CLocalMonitor::AddPath(const mstring &path) {
    if (IsFileInCache(path))
    {
        return true;
    }

    mPathSet.push_back(path);
    EnumFiles(path, true, FileEnumProc, 0);

    CWinFileNotify::GetInst()->Register(path, -1, FileNotify);
    mstring low = path;
    mPathSet.push_back(low.makelower());
    return true;
}

list<mstring> CLocalMonitor::GetPathSet() const {
    return mPathSet;
}

bool CLocalMonitor::IsRunning() {
    return true;
}

CLocalMonitor::LocalLogCache *CLocalMonitor::GetFileCache(const mstring &filePath) {
    mstring low = filePath;
    low.makelower();

    map<mstring, LocalLogCache *>::iterator it = mLogCache.find(low);
    if (mLogCache.end() == it)
    {
        LocalLogCache *newCache = new LocalLogCache();
        newCache->mFilePath = filePath;
        mLogCache[low] = newCache;
        return newCache;
    } else {
        return it->second;
    }
}

void CLocalMonitor::OnLogReceived(LocalLogCache *cache) {
    if (cache->mLastCache.empty())
    {
        return;
    }

    size_t curPos = 0;
    size_t lastPos = 0;
    while (true) {
        curPos = cache->mLastCache.find("\n", lastPos);
        if (string::npos == curPos) {
            break;
        }

        if (curPos > lastPos)
        {
            string lineStr = cache->mLastCache.substr(lastPos, curPos - lastPos);
            if (!lineStr.empty())
            {
                mListener->OnLogReceived(cache->mFilePath, lineStr);
            }
        }
        lastPos = curPos + 1;
    }

    if (lastPos > 0)
    {
        cache->mLastCache.erase(0, lastPos);
    }
}

bool CLocalMonitor::IsFileInCache(const mstring &filePath) const {
    mstring low = filePath;
    low.makelower();
    for (list<mstring>::const_iterator it = mPathSet.begin() ; it != mPathSet.end() ; it++)
    {
        mstring low2 = *it;
        low2.makelower();

        if (low.startwith(low2.c_str()))
        {
            return true;
        }
    }
    return false;
}

bool CLocalMonitor::FileEnumProc(bool isDir, const char *filePath, void *param) {
     if (isDir)
    {
        return true;
    }

    if (IsLogFile(filePath))
    {
        LocalLogCache *newCache = new LocalLogCache();
        newCache->mFilePath = filePath;
        newCache->mFilePath.makelower();
        FILE *fp = fopen(filePath, "rb");

        if (NULL != fp)
        {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            newCache->mLastPos = size;
            newCache->mFileSize = size;
            fclose(fp);
        }
        GetInst()->mLogCache[newCache->mFilePath] = newCache;
    }
    return true;
}

void CLocalMonitor::FileNotify(const char *filePath, unsigned int mask) {
    AutoLocker locker(GetInst());

   if (INVALID_FILE_ATTRIBUTES == GetFileAttributesA(filePath))
   {
       return;
   }

   LocalLogCache *cache = GetInst()->GetFileCache(filePath);
   FILE *fp = fopen(filePath, "rb");
   if (!fp)
   {
       return;
   }
   fseek(fp, 0, SEEK_END);
   int size = ftell(fp);

   if (0 == size)
   {
       int dd = 1234;
   }

   if (cache->mFileSize < (DWORD)size)
   {
       fseek(fp, cache->mLastPos, SEEK_SET);
       char buffer[1024];
       int count = 0;
       while ((count = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            cache->mLastCache.append(buffer, count);
       }
       cache->mLastPos = size;
       cache->mFileSize = size;
   }
   fclose(fp);
   GetInst()->OnLogReceived(cache);
}