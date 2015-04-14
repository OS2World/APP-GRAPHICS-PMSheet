#define OP_SLIDES     0
#define OP_DETAILVIEW 1
#define OP_SLIDESHOW  2
#define MSG_HANDLED   (MRESULT)1

#define SLIDE_LARGE  240  // PIXELS
#define SLIDE_MEDIUM 160
#define SLIDE_SMALL  100

#define ROTATE_270   270
#define ROTATE_90     90


#define UM_NEWLIST          WM_USER+1
#define UM_NEWITEM          WM_USER+2
#define UM_COUNTPICTURES    WM_USER+3
#define UM_JPGERROR         WM_USER+32 
#define UM_DETAIL           WM_USER+33
#define UM_DROPCOPYDONE     WM_USER+34
#define UM_CLOSEPROGRESS    WM_USER+35
#define UM_SHOWPROGRESS     WM_USER+36
#define UM_SHOWFILENAME     WM_USER+37
#define UM_DOSERROR         WM_USER+38
#define UM_FOLDERSTRING     WM_USER+39
#define UM_IMAGESTRING      WM_USER+40

typedef struct _canvas{
                  HWND   hwndVscroll;        /* Vertical scrollbar   */
                  HWND   hwndHscroll;        /* HoriZontal scrollbar */
                  SHORT  sVscrollMax;        /* Scrollrange. vert    */
                  SHORT  sHscrollMax;        /* Scrollrange. horz    */
                  SHORT  sVscrollPos;        /* Scrollpos.           */
                  SHORT  sHscrollPos;        /* Scrollpos.           */
                  USHORT cySheet;
                  USHORT cxSheet;
                  int    iZoom;
                  HWND   hwndClient;
} CANVAS,*PCANVAS;



class sheet
{

public:
   sheet (HAB hab,HWND hwnd);
   ~sheet();
   MRESULT wndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2);
    MRESULT slwndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2);

   MRESULT viewPortscrollVert(MPARAM mp1, MPARAM mp2);
   MRESULT viewPortscrollHorz(MPARAM mp1, MPARAM mp2);

   HWND    getFrameWindow( void );
   MRESULT paintSlides(HPS hps, RECTL *rclSheet, RECTL *rclInvalidate);
   VOID    sizeSlideSheet( void );
   VOID    sizeDetailView( hPicture *pPict );
   void    SetScrollBar(BOOL bSet);
   void    positionWindowInViewPort(void);
   MRESULT loadPicturesFromDir(char *pszDirectory);
   MRESULT forceReloadPictures(void);  /* <F5> */
   MRESULT createPictureSheet(int slideSize);
   MRESULT handleKeyBoard(MPARAM mp1,MPARAM mp2);

   MRESULT showDetailOnSelection( void ); //app calls this from menu detail
   MRESULT sortOnSize ( void );           //app calls this from menu sort
   MRESULT sortOnName ( void );           //app calls this from menu sort
   MRESULT selectAll  ( void );           //app calls this from menu select all
   MRESULT unSelectAll( void );           //app calls this from menu 
   MRESULT playSlides ( void );           //app calls this from menu 
   MRESULT deleteSelected(void);          //app calls this from menu and toolbar
   MRESULT rotateSelected(int iRot);      //app calls this from menu and toolbar


   void    showTitles( BOOL bShow );
   void    createViewPort(HWND hwnd);
   void    setSheetWindowSize( int cx, int cy);
   void    setPS(HPS hps){ m_hps = hps; }
   HPS     getPS( void ) { return m_hps;}
   BOOL    getShowName( void ){ return m_bShowName; }
   BOOL    isKeyboardBlocked( void ){ return m_keyboardblock; }
   void    blockKeyboard( BOOL bLock = TRUE ){ m_keyboardblock = bLock; }

   void    setSingleSelection(hPicture *pPict){ m_pvCurrent = pPict; }
   hPicture * getSingleSelection( void ){  return m_pvCurrent; }

   MRESULT handleMouseButton1down(POINTL ptlMouse,MPARAM mp2);
   MRESULT handleMouseBtn1dblclk(POINTL ptlMouse,MPARAM mp2);

//must be private:
   void initThreadBlock(LOADERDEF *pLoader);
   void scrollSelectionInView( BOOL bAbs );
   MRESULT drop(HAB hab,PDRAGINFO pDraginfo);
   MRESULT dragOver(HWND hwnd, HAB hab, PDRAGINFO pDraginfo, PSZ pCurrentDir);
   MRESULT dragDiscardObject(HWND hwnd, HAB hab, PDRAGINFO pDraginfo);
   BOOL    initDrag (HWND hWindow, POINTL ptlMouse );
   MRESULT dragEndConversation(MPARAM mp1,MPARAM mp2);

   BOOL    changeOpMode( int opmode );
   int     getOpMode( void ) { return m_opMode; }
   void    rePositionSlides( void );

   void    destroySlideShowWindow(void);
   hPicture * loadNextSelected(HWND hwnd,hPicture * hPict);

   HWND hwndFrame;
   HWND m_hwndClient;
   HWND m_hwndSheet;
   HWND m_hwndSlide;
   HWND m_hProgress;
   HWND m_hwndApplic;
   HWND hwndList;
   picturelist *picList;
   HPS  m_hpsMem;
   int  m_Columns;
   int  m_Rows;
   int  m_slideSize;
   int  m_cxClient;
   int  m_cyClient;
   int  m_cxSheet;
   int  m_cySheet;
   int  m_opMode;
   long m_lCharHeight;
   HPS  m_hps;
   BOOL m_bShowName;            /* show filename in slide           */
   hPicture *m_pvCurrent;       /* Current selected picture         */
   char m_szFolder[CCHMAXPATH]; /* Current folder being in view     */
   BOOL m_keyboardblock;
   CANVAS view;
   DRAGITEM   *m_pDragitems;
   ULONG       m_lItemsDragged;
   BOOL        m_bControl;      /* drag/drop copy when true   */
   BOOL        m_bShift;        /* drag/drop move to diskette */
   BOOL        m_bPaused;       /* Paused slide show TRUE|FALSE*/
};
