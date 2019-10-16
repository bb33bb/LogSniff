#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <LogLib/mstring.h>
#include "../LogServMgr.h"

class CServTreeDlg : public LogServEvent {
    enum TreeNodeType {
        em_tree_root_node,      //树型控件根节点
        em_tree_file_log,       //文件日志监控
        em_tree_dbg_msg,        //调试信息监控
        em_tree_file_search,    //文件内容检索
        em_tree_dir_list,       //文件列表信息
        em_tree_dir_root,
        em_tree_dir_child,
        em_tree_file
    };

    struct TreeRootCache {
        const LogServDesc *mServDesc;
        HTREEITEM mFileLogItem;
        HTREEITEM mFileSetItem;
        list<mstring> mPathList;

        TreeRootCache() {
            mServDesc = NULL;
            mFileLogItem = NULL;
            mFileSetItem = NULL;
        }
    };

    struct TreeCtrlParam {
        TreeNodeType mNodeType;
        const LogServDesc *mServDesc;
        mstring mFilePath;

        TreeCtrlParam() {
            mNodeType = em_tree_root_node;
         }
    };

    //文件树缓存
    struct FileTreeCache {
        HTREEITEM mTreeNodeIndex;
        mstring mFilePath;
    };
public:
    CServTreeDlg();
    virtual ~CServTreeDlg();

    BOOL CreateDlg(HWND hParent);
    HWND GetWindow();
    BOOL MoveWindow(int x, int y, int cx, int cy);
    void OnNewLogFiles(const list<mstring> &fileSet);

private:
    mstring ShowFileDlg(const mstring &initDir) const;
    INT_PTR OnInitDialog(WPARAM wp, LPARAM lp);
    INT_PTR OnCommand(WPARAM wp, LPARAM lp);
    INT_PTR OnNotify(WPARAM wp, LPARAM lp);
    INT_PTR OnClose(WPARAM wp, LPARAM lp);

    void OnServTreeUpdate(const LogServDesc *desc);
    INT_PTR OnServAddedInternal(const LogServDesc *desc);
    INT_PTR OnServAlterInternal(const LogServDesc *desc);
    static INT_PTR CALLBACK ServTreeDlgProc(HWND hlg, UINT msg, WPARAM wp, LPARAM lp);

    HTREEITEM InsertItem(HTREEITEM parent, const std::mstring &name, const TreeCtrlParam *param) const;
    BOOL SetItemStat(HTREEITEM treeItem, DWORD statMask) const;

    bool IsServInCache(const LogServDesc *desc) const;
    bool InsertServToCache(const LogServDesc *desc, HTREEITEM hFileLog, HTREEITEM hFileSet);
    bool UpdateServCache(const LogServDesc *desc);

    void DeleteChildByName(HTREEITEM parent, const std::mstring &name);
    TreeRootCache *GetCacheFromDesc(const LogServDesc *desc) const;

    //文件树接口
    void InsertPathToFileTree(const mstring &path);
    void DeletePathFromFileTree(const mstring &path);
private:
    virtual void OnLogServAdd(const LogServDesc *d);
    virtual void OnLogServSwitch(const LogServDesc *d1, const LogServDesc *d2);
    virtual void OnLogServAlter(const LogServDesc *d);

private:
    HWND mhWnd;
    HWND mParent;
    HWND mTreeCtrl;
    std::vector<const LogServDesc *> mServDesc;
    std::vector<TreeRootCache *> mTreeCache;
    //文件树缓存
    list<FileTreeCache> mFileTreeCache;
};