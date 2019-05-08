#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "GroupReceiver.h"
#include "common/Util.h"
#include "FileNotify.h"
#include "LogProtocol.h"
#include "LogMonitor.h"

using namespace std;

time_t gStartTime = 0;

int main(int argc, char *argv[]) {
    if (argc == 1)
    {
        printf("Err:need path param\n");
        return 0;
    }

    gStartTime = time(0);
    const char *filePath = argv[1];
    struct stat fileStat = {0};
    if (0 != stat(filePath, &fileStat) || !S_ISDIR(fileStat.st_mode)) {
        printf("Err:invalid path:%s", filePath);
        return 0;
    }

    dp("logPath:%s", filePath);
    CLogMonitor::GetInst()->InitMonitor(filePath);
    CGroupReceiver::GetInst()->InitRecviver(GROUP_PORT);
    sleep(-1);
    return 0;
}