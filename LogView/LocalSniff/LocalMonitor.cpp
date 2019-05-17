#include <WinSock2.h>
#include "LocalMonitor.h"
#include "WinFileNoitfy.h"
#include "../LogReceiver.h"

using namespace std;

CLocalMonitor *CLocalMonitor::GetInst() {
    static CLocalMonitor *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLocalMonitor();
    }
    return sPtr;
}

bool CLocalMonitor::Init(const MonitorCfg &cfg, CMonitorEvent *listener) {
    mCfg = cfg, mListener = listener;
    return true;
}

bool CLocalMonitor::Start() {
    return true;
}

bool CLocalMonitor::Stop() {
    return true;
}

bool CLocalMonitor::AddPath(const mstring &path) {
    CWinFileNotify::GetInst()->Register(path, -1, FileNotify);
    mPathSet.push_back(path);
    return true;
}

list<mstring> CLocalMonitor::GetPathSet() const {
    return mPathSet;
}

bool CLocalMonitor::IsStart() {
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

   if (cache->mFileSize < (DWORD)size)
   {
       fseek(fp, cache->mLastPos, SEEK_SET);
       char buffer[1024];
       int count = 0;
       while ((count = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            cache->mLastCache.append(buffer, count);
       }
   }
   fclose(fp);
   cache->mLastPos = size;
   cache->mFileSize = size;
   GetInst()->OnLogReceived(cache);
}