

class XDIRDATA;
class XDIRLIST;

class XDIRLIST
{
public:
    XDIRLIST ();
    virtual ~XDIRLIST ();
    void addDirectory(XDIRDATA *pParent,XDIRDATA *pDir); /* if parent not defined it must be a drive */
    XDIRDATA * getNext(XDIRDATA *pDir);
    XDIRDATA * getFirstChild(XDIRDATA *pDir);
    XDIRDATA * getBase();
    void creatPathForDir(XDIRDATA *pDir, char *pszDir);
    void cleanupList(XDIRDATA *pDir);
private:
    XDIRDATA *pBase;    /* The base of the complete tree */
};


class XDIRDATA
{
friend class XDIRLIST;
    
public:
    XDIRDATA ();
  CHAR            szDirectoryName[CCHMAXPATH];
  CHAR            szDisplayName[CCHMAXPATH];
  int             iType;             /* floppydrive/hardrive/cdrom/dir */
  XDIRDATA        *parent;          /* parent directory               */
  XDIRDATA        *child;           /* child  directory               */
  XDIRDATA        *next;            /* next directory on same level   */
  XDIRDATA        *prew;            /* prev directory                 */
  void            *pControlData;
};



typedef struct _DIRDATARECORD
{
  MINIRECORDCORE   MiniRec;          /* Container record               */
  XDIRDATA        *pDirData;
} DIRDATARECORD;
typedef DIRDATARECORD *PDIRDATARECORD;
/*
** function prototypes of the worker threads.
** Must be of type _System when compiled with the icc ibm compiler
*/
extern "C" {
    void _System makeDriveList( ULONG lDataRef );
    void _System makeFolderList( ULONG lDataRef );
    void _System checkFolderList( ULONG lDataRef );
    void _System fileCopyThread (void *pBlock);
}
/*
** Comunication block to be given to the different threads
*/
typedef struct _comblock
{
    HWND   hwnd;            /* window to be notified by thread    */
    HWND   mainParent;      /* Control parent window handle       */
    char   szFolder[255];   /* search profile <drive>:\<dir>\*.*  */
    XDIRDATA *pParent;      /* Do we wanna get info for this parent? */
    XDIRLIST *pList;        /* ticket to the data tree maintained by the dirCtrl */
} comblock;
/*
** The directory control
*/
class dirCtrl
{
public:
    dirCtrl(HAB hab, HWND hParent, long controlid, long flAttribute, unsigned long  ulNotify );
    virtual ~dirCtrl();
    MRESULT WndProc(HWND hwnd, ULONG msg, MRESULT mp1, MRESULT mp2);
    long getControlId( void ){ return m_controlId; }
    HWND getWindowHandle( void ){ return m_hParent;} // The main window in the dirctrl
    BOOL getDirectoryName( char *szBuf, int iSize ); // Get the directory name of current selection.

    MRESULT processDrop( HWND hwnd, PCNRDRAGINFO pCnrDragInfo );

private:
    HWND m_hwnd;
    HWND m_hParent;
    HWND m_hMainParent;
    HWND m_hProgress;
    XDIRLIST * m_pList;
    long m_controlId;
    long m_parentId;
    HPOINTER  m_hptrFolderIcon;
    HPOINTER  m_hptrNetDrvIcon;
    HPOINTER  m_hptrHrdDrvIcon;
    HPOINTER  m_hptrFlpDrvIcon;
    HPOINTER  m_hptrCdDrvIcon;

    BOOL      m_bChanged;    // Was there a selection done?
    ULONG     m_dirNotify;   // bigger than WM_USER means, start timer and notify parent on dir changes.
    char m_szDirectory[CCHMAXPATH]; // communicated to parent when directory name is for more than 1 sec the same
    char m_szTemp[CCHMAXPATH];

    DIRDATARECORD * m_pCurrent;                         // current selected item.
    DIRDATARECORD * m_retryOption;

    BOOL addIncompleteNode(DIRDATARECORD *  pChain,BOOL bTraverse);
    BOOL remIncompleteNode(DIRDATARECORD *  pChain);
    void createDriveList( void );
    BOOL addTreeDriveList(XDIRLIST *pList, int iCount);
    BOOL addFolderList(XDIRDATA * pParentDir,int iCount);
    BOOL makePathString(XDIRDATA *pDir, char *pszDir);
    void createFolderList(MPARAM mp2);
    void checkIncompleteNodes(XDIRDATA   *pParent);
    void setCurrentSelection( DIRDATARECORD * p){ m_pCurrent = p; m_bChanged = TRUE; }
    void onTimerTick(void);
    DIRDATARECORD * getRetryOption(){ return m_retryOption; }
};

extern "C" {

MRESULT EXPENTRY treeParentWndProc (HWND hwnd, ULONG msg, MRESULT mp1, MRESULT mp2);

}
