#include "LogMonitor.h"
#include "FileNotify.h"
#include "common/Util.h"
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "LogProtocol.h"

using namespace std;

CLogMonitor *CLogMonitor::GetInst() {
    static CLogMonitor *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CLogMonitor();
    }
    return sPtr;
}

void CLogMonitor::FileNotifyProc(const char *filePath, unsigned int mask) {
    AutoLocker locker(GetInst());

    map<string, LogFileInfo>::iterator it = GetInst()->mLogCache.find(filePath);
    struct stat fileStat = {0};
    if (0 == stat(filePath, &fileStat))
    {
        if (fileStat.st_size > it->second.mLastPos)
        {
            GetInst()->DispatchLog(filePath, it->second.mLastPos, fileStat.st_size);
            it->second.mLastPos = fileStat.st_size;
            it->second.mFileSize = fileStat.st_size;
        }
    } else {
        GetInst()->mLogCache.erase(it);
    }
}

void CLogMonitor::InitMonitor(const char *path) {
    class CFileEnumProc {
    public:
        static bool FileEnumProc(bool isDir, const char *filePath, void *param) {
            if (isDir)
            {
                return true;
            }

            map<std::string, LogFileInfo> *pCache = (map<std::string, LogFileInfo> *)param;
            struct stat fileStat = {0};
            if (0 == stat(filePath, &fileStat))
            {
                LogFileInfo newFile;
                newFile.mFilePath = filePath;
                newFile.mLastModified = fileStat.st_mtime;
                newFile.mFileSize = fileStat.st_size;

                AutoLocker locker(CLogMonitor::GetInst());
                pCache->insert(make_pair(filePath, newFile));
            }
            return true;
        }
    };

    mPathSet.push_back(path);
    EnumFiles(path, true, CFileEnumProc::FileEnumProc, &mLogCache);
    mNotifyHandle = CFileNotify::GetInst()->Register(path, FD_NOTIFY_ALL, FileNotifyProc);
}

list<string> CLogMonitor::GetPathSet() {
    return mPathSet;
}

CLogMonitor::CLogMonitor() {
}

CLogMonitor::~CLogMonitor() {
}

void CLogMonitor::run() {
}

void CLogMonitor::OnServAccept(unsigned int client) {
}

void CLogMonitor::OnRecvComplete(unsigned int client, const LpResult &result) {
    if (result.mCommand == em_cmd_register)
    {
        mListener.insert(client);
    }
}

void CLogMonitor::DispatchLog(const std::string &filePath, long startPos, long endPos) const {
    FILE *fp = fopen(filePath.c_str(), "rb");

    if (0 == fp)
    {
        return;
    }
    fseek(fp, startPos, 0);

    char buffer[1024];
    string str;
    int readCount = endPos - startPos;
    str.reserve(readCount);

    while (true) {
        int c = fread(buffer, 1, sizeof(buffer), fp);

        if (c <= 0)
        {
            break;
        }
        str.append(buffer, c);
        readCount -= c;

        if (readCount <= 0)
        {
            break;
        }
    }
    fclose(fp);

    if (str.size() > 0)
    {
        string d;
        CLogProtocol::GetInst()->EncodeLog(filePath, str, d);
        for (set<unsigned int>::const_iterator it = mListener.begin() ; it != mListener.end() ; it++)
        {
            send(*it, d.c_str(), d.size(), 0);
        }
    }
}

void CLogMonitor::OnServRecvData(unsigned int client, const std::string &strRecved, std::string &strResp) {
    AutoLocker locker(this);
    mDataCache[client].append(strRecved);

    list<LpResult> result;
    if (CLogProtocol::GetInst()->GetRecvResult(mDataCache[client], result) > 0)
    {
        for (list<LpResult>::const_iterator it = result.begin() ; it != result.end() ; it++)
        {
            OnRecvComplete(client, *it);
        }
    }
}

void CLogMonitor::OnServSocketErr(unsigned int client) {
    mListener.erase(client);
}

void CLogMonitor::OnServSocketClose(unsigned int client) {
    mListener.erase(client);
}