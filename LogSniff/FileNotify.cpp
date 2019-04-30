#include "FileNotify.h"
#include "common/Util.h"

using namespace std;

#define NOTIFY_MASK (IN_MODIFY | IN_CREATE | IN_DELETE)

CFileNotify::CFileNotify() {
    mSerial = 0xf1;
}

CFileNotify::~CFileNotify() {
}

CFileNotify *CFileNotify::GetInst() {
    static CFileNotify *sPtr = NULL;

    if (NULL == sPtr)
    {
        sPtr = new CFileNotify();
    }
    return sPtr;
}

void CFileNotify::MonitorPath(const char *filePath, FileNotifyRegister *ptr) {
    AutoLocker locker(GetInst());

    int wd = 0;
    map<string, int>::const_iterator it = mWdGlobal1.find(filePath);
    if (mWdGlobal1.end() == it)
    {
        wd = inotify_add_watch(mFd, filePath, NOTIFY_MASK);
        mWdGlobal1[filePath] = wd;
        mWdGlobal2[wd] = filePath;
    } else {
        wd = it->second;
    }
    ptr->mWdSet.insert(wd);
}

bool CFileNotify::FileEnumProc(bool isDir, const char *filePath, void *param) {
    FileNotifyRegister *ptr = (FileNotifyRegister *)param;
    if (!isDir)
    {
        return true;
    }

    GetInst()->MonitorPath(filePath, ptr);
    return true;
}

HFileNotify CFileNotify::Register(const string &filePath, unsigned int mask, pfnFileNotify pfn, bool withSub) {
    AutoLocker locker(this);

    FileNotifyRegister newItem;
    newItem.mask = mask;
    newItem.mIndex = mSerial++;
    newItem.mMonitorPath = filePath;
    newItem.mNotify = pfn;
    newItem.mWithSubDir = withSub;

    MonitorPath(filePath.c_str(), &newItem);
    EnumFiles(filePath.c_str(), true, FileEnumProc, &newItem);
    mRegisterCache.push_back(newItem);
    return newItem.mIndex;
}

void CFileNotify::UnResister(HFileNotify h) {
}

void CFileNotify::OnFileEvent(const inotify_event *notifyEvent) {
    if (!notifyEvent) {
        return;
    }

    int wd = notifyEvent->wd;
    for (list<FileNotifyRegister>::iterator it = mRegisterCache.begin() ; it != mRegisterCache.end() ; it++)
    {
        set<int>::const_iterator ij = it->mWdSet.find(wd);
        if (it->mWdSet.end() != ij)
        {
            string filePath = mWdGlobal2[wd];
            filePath += "/";

            if (notifyEvent->name[0] == '.')
            {
                filePath += notifyEvent->name + 1;
            } else {
                filePath += notifyEvent->name;
            }

            //文件夹创建,增加监控
            if ((notifyEvent->mask & IN_CREATE) && (notifyEvent->mask & IN_ISDIR))
            {
                dp("monitor dir:%hs, mask:0x%08x", filePath.c_str(), notifyEvent->mask);
                GetInst()->MonitorPath(filePath.c_str(), &(*it));
            } else {
                dp("file info:%hs, mask:0x%08x", filePath.c_str(), notifyEvent->mask);

                int notifyMask = 0;
                if (it->mask & FD_NOTIFY_MODIFIED)
                {
                    if (notifyEvent->mask & IN_MODIFY)
                    {
                        notifyMask |= FD_NOTIFY_MODIFIED;
                    }
                }

                if (it->mask & FD_NOTIFY_CREATE)
                {
                    if (notifyEvent->mask & IN_CREATE)
                    {
                        notifyMask |= FD_NOTIFY_CREATE;
                    }
                }

                if (it->mask & FD_NOTIFY_DELETE)
                {
                    if (notifyEvent->mask & IN_DELETE)
                    {
                        notifyMask |= FD_NOTIFY_DELETE;
                    }
                }

                if (notifyMask)
                {
                    it->mNotify(filePath.c_str(), notifyMask);
                }
            }
        }
    }
}

void CFileNotify::run() {
    inotify_event *notifyEvent = 0;
    char buffer[1024];
    while (true) {
        int count = read(mFd, buffer, sizeof(buffer));

        int pos = 0;
        while (true) {
            if (count <= 0) {
                break;
            }

            notifyEvent = (inotify_event *)(buffer + pos);
            if (notifyEvent->mask && notifyEvent->len > 0)
            {
                GetInst()->OnFileEvent(notifyEvent);
            }

            int ss = sizeof(inotify_event) + notifyEvent->len;
            count -= ss;
            pos += ss;

            dp("len:%d, count:%d", notifyEvent->len, count);
            if (notifyEvent->len == 0)
            {
                break;
            }
        }
    }
}

void CFileNotify::InitNotify() {
    mFd = inotify_init();
    mThread.StartThread(this, false);
}