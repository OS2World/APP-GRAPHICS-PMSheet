#define CNR_ID             100
#define UM_LETMEKNOW      WM_USER+100
#define UM_FOLDERSTRING   WM_USER+39   /* don't change!!!*/
#define UM_IMAGESTRING    WM_USER+40   /* don't change!!!*/

class appmain
{
public:
    appmain ( void );
    MRESULT wndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2);

private:
    MRESULT handleCommand(MPARAM mp1,MPARAM mp2);
    MRESULT layoutControls( void );
    void    initTrackInfo(TRACKINFO *pTrack);
    sheet    *m_pSheet;
    dirCtrl  *m_pTree;
    statline *m_pStatusLine;
    tbar *m_pTool;
    HWND  m_hwnd;
    HWND  m_hwndFrame;
    LONG  m_cx;
    LONG  m_cy;
    RECTL m_rSheet;
    RECTL m_rTree;
    RECTL m_rSplit;
    BOOL  m_bHit;
};

