#pragma once
#include <Windows.h>

struct ShowConfig {
    BOOL mTopMost;
    BOOL mAutoScroll;
    BOOL mPause;

    ShowConfig() {
        mTopMost = FALSE, mAutoScroll = FALSE, mPause = FALSE;
    }
};

extern ShowConfig gShowConfig;