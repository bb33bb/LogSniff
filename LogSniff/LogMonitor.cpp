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

    struct stat fileStat = {0};
    if (0 == stat(filePath, &fileStat))
    {
        map<string, LogFileInfo *>::iterator it = GetInst()->mLogCache.find(filePath);
        LogFileInfo *pFileInfo = 0;
        if (GetInst()->mLogCache.end() == it)
        {
            dp("new file:%hs", filePath);
            LogFileInfo *newFile = new LogFileInfo();
            newFile->mFilePath = filePath;
            newFile->mLastModified = fileStat.st_mtime;
            newFile->mFileSize = fileStat.st_size;
            newFile->mLastPos = 0;

            GetInst()->mLogCache.insert(make_pair(filePath, newFile));
            pFileInfo = newFile;
        } else {
            dp("file aaaa");
            pFileInfo = it->second;
        }

        if (fileStat.st_size > it->second->mLastPos)
        {
            GetInst()->DispatchLog(it->second, fileStat.st_size);
            it->second->mLastPos = fileStat.st_size;
            it->second->mFileSize = fileStat.st_size;
        }
    } else {
        GetInst()->mLogCache.erase(filePath);
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

            map<std::string, LogFileInfo *> *pCache = (map<std::string, LogFileInfo *> *)param;
            struct stat fileStat = {0};
            if (0 == stat(filePath, &fileStat))
            {
                LogFileInfo *newFile = new LogFileInfo();
                newFile->mFilePath = filePath;
                newFile->mLastModified = fileStat.st_mtime;
                newFile->mFileSize = fileStat.st_size;

                AutoLocker locker(CLogMonitor::GetInst());
                pCache->insert(make_pair(filePath, newFile));
            }
            return true;
        }
    };

    mPathSet.push_back(path);
    EnumFiles(path, true, CFileEnumProc::FileEnumProc, &mLogCache);
    mNotifyHandle = CFileNotify::GetInst()->Register(path, FD_NOTIFY_ALL, FileNotifyProc);
    mTcpServ.InitServ(LOG_PORT, this);
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
        dp("register");
        mListener.insert(client);
    }
}

void CLogMonitor::DispatchLog(LogFileInfo *info, long fileSize) const {
    FILE *fp = fopen(info->mFilePath.c_str(), "rb");

    if (0 == fp)
    {
        return;
    }
    fseek(fp, info->mLastPos, 0);

    char buffer[1024];
    string str;
    int readCount = fileSize - info->mLastPos;
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
        info->mLastCache += str;

        //send log by line
        size_t curPos = 0;
        size_t lastPos = 0;
        while (true) {
            curPos = info->mLastCache.find("\n", lastPos);
            if (string::npos == curPos) {
                break;
            }

            if (curPos > lastPos)
            {
                string lineStr = info->mLastCache.substr(lastPos, curPos - lastPos);
                if (!lineStr.empty())
                {
                    string d;
                    CLogProtocol::GetInst()->EncodeLog(info->mFilePath, str, d);
                    for (set<unsigned int>::const_iterator it = mListener.begin() ; it != mListener.end() ; it++)
                    {
                        send(*it, d.c_str(), d.size(), 0);
                    }
                }
            }
            lastPos = curPos + 1;
        }

        if (lastPos > 0)
        {
            info->mLastCache.erase(0, lastPos);
        }
    }
}

void CLogMonitor::OnServRecvData(unsigned int client, const std::string &strRecved, std::string &strResp) {
    AutoLocker locker(this);
    mDataCache[client].append(strRecved);
    dp("recv:%d", strRecved.size());

    list<LpResult> result;
    if (CLogProtocol::GetInst()->GetRecvResult(mDataCache[client], result) > 0)
    {
        dp("result size:%d", result.size());
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