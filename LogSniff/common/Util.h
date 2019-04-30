#pragma once

void PrintDbgInternal(const char *tag, const char *file, unsigned int line, const char *fmt, ...);
#define dp(f, ...) PrintDbgInternal("LogSniff", __FILE__, __LINE__, f, ##__VA_ARGS__)

typedef bool (*pfnFileCallback)(bool isDir, const char *filePath, void *param);
bool EnumFiles(const char *path, bool subDir, pfnFileCallback pfn, void *param = 0);

template <class T>
class MemoryAlloc {
public:
    MemoryAlloc() {
        mBuffer = 0;
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