#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <LogLib/mstring.h>
#include "../LogServMgr.h"

class CServTreeDlg : public LogServEvent {
    enum TreeNodeType {
        em_tree_local_root_node,      //���Ϳؼ����ڵ�
        em_tree_local_config,         //������־����ѡ��
        em_tree_local_file_log,       //�ļ���־���
        em_tree_local_dbg_msg,        //������Ϣ���
        em_tree_local_file_search,    //�ļ����ݼ���
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
            mNodeType = em_tree_local_root_node;
         }
    };

    //�ļ�������
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

    //�ļ����ӿ�
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
    //�ļ�������
    list<FileTreeCache> mFileTreeCache;
};