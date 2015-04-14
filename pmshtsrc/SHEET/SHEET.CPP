/*---------------------------------------------------------------------------*/
/*  Name: sheet.cpp                                                          */
/*                                                                           */
/*  Description : Implements the colorbar at the bottom of the main window.  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define INCL_WIN
#define INCL_GPI
#define INCL_DOSDATETIME
#define INCL_SHLERRORS
#define INCL_DOS
#include <os2.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <direct.h>
#include "shtdir.hpp"
#include "sheet.hpp"
#include "..\resource.h"


#define ID_VALUESET 0x1000
#define VAL_COLUMNS 6
#define VAL_ROWS    6
#define VAL_SMALL   6 // COLUMNS
#define VAL_MEDIUM  5
#define VAL_LARGE   4

#define PIC_SPATIE   5 /* space between the slided */

#define CX_SHEET      640 // pixels minimum width of slidewindow
#define CY_SHEET      768 // pixels
#define MIN_ROWS       10

#define  DRG_IMGSIZE      75    /* Drag image size in pixels.     */
#define  ID_DRGITEM      150

#define DO_IMAGEDROP       DO_UNKNOWN + 160
#define DO_METAFILEDROP    DO_UNKNOWN + 165
#define DO_DRAWITFILEDROP  DO_UNKNOWN + 170
#define DO_WINDOWSWMFDROP  DO_UNKNOWN + 175
#define ID_TIMER     1
#define ID_SLIDETIMEOUT 4000
static BOOL bSetScrollbars;
static char *suppfiles[] = {"JPG",NULL};   /* supported files on drop */


HAB   hab;
sheet *pShtCreate;	/* temp var */
static szErrBuf[CCHMAXPATH];
#define DB_RAISED    0x0400	    /* the undocumented value's */
#define DB_DEPRESSED 0x0800     /* drawbox.                 */

static int drawStatusLine(HPS hps, char *pszText)
{
   FONTMETRICS fm;
   RECTL rcl;
   printf("DrawStatusLine %s\n",pszText);
   GpiQueryFontMetrics(hps,(LONG)sizeof(FONTMETRICS),&fm);
   rcl.xLeft = 0;
   rcl.xRight = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
   rcl.yBottom= 5;
   rcl.yTop   = rcl.yBottom + fm.lMaxBaselineExt + 5;
   WinDrawText(hps,-1,(PCH)pszText,
               &rcl,CLR_BLUE,CLR_BACKGROUND,
               DT_CENTER | DT_VCENTER | DT_ERASERECT);
   printf("DONE DrawStatusLine %s\n",pszText);
   return 0;
}
/*---------------------------------------------------------------------------*/
/* slideshow windowproc.                                                     */
/*---------------------------------------------------------------------------*/
MRESULT sheet::slwndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
    static    HPS    hps;
    SIZEL    sizl;
    RECTL  rcl;
    POINTL ptl;
    static RECTL  rclWnd;
    static LONG cx,cy;
    static hPicture *pPict;
    HDC    hdcClient;
    BOOL   bPause;

    switch (ulMsg)
    {
    case WM_CREATE:
        rclWnd.xLeft   = 0;
        rclWnd.xRight  = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
        rclWnd.yBottom = 0;
        rclWnd.yTop    = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
        pPict          = NULL;
        pPict          = loadNextSelected(m_hwndSlide,pPict);
        WinSetFocus(HWND_DESKTOP,m_hwndSlide);
        return 0;
    case WM_TIMER:
        if (!m_bPaused)
        {
           pPict      = loadNextSelected(m_hwndSlide,pPict);
           WinStopTimer(hab,m_hwndSlide,ID_TIMER);
        }
        return 0;
    case WM_BUTTON1DBLCLK:
        return 0;    
    case WM_BUTTON2DOWN:
        return 0;
    case WM_BUTTON1DOWN:
        return 0;
    case UM_DETAIL:   /* LOADED DETAIL OF IMAGE */
        WinInvalidateRect(m_hwndSlide,NULL,TRUE);
        return 0;
    case WM_CHAR:
        bPause = m_bPaused;
        handleKeyBoard(mp1,mp2);
        if (m_bPaused != bPause && m_bPaused)
        {
            WinStopTimer(hab,m_hwndSlide,ID_TIMER);
            WinInvalidateRect(m_hwndSlide,NULL,TRUE);
        }
        else if (m_bPaused != bPause && !m_bPaused)
        {
           hps = WinGetPS(m_hwndSlide);
           drawStatusLine(hps,"***** SlideShow continued *****");
           WinInvalidateRect(m_hwndSlide,NULL,TRUE);
           WinReleasePS(hps);
        }
        return 0;
    case WM_PAINT:
        hps = WinBeginPaint (m_hwndSlide,(HPS)0,&rcl);
        rclWnd.xLeft   = 0;
        rclWnd.xRight  = WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
        rclWnd.yBottom = 0;
        rclWnd.yTop    = WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
        if (!m_bPaused)
           WinFillRect(hps,&rclWnd,CLR_BLACK);

        if (pPict)
            pPict->paintDetail(hps,&rclWnd);

        if (m_bPaused)
            drawStatusLine(hps,"**** Paused.....any key to continue ****");
        WinEndPaint(hps);
        if (!m_bPaused && pPict->getDetailImageData())
            WinStartTimer(hab,m_hwndSlide,ID_TIMER,ID_SLIDETIMEOUT);
        return (MRESULT)0;
   }
   return (MRESULT)WinDefWindowProc(m_hwndSlide,ulMsg,mp1,mp2);
}
/*-----------------------------------------------[ private ]--------------*/
MRESULT EXPENTRY slideWndProc(HWND hwnd,ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
   sheet *pSheet;

   if (pShtCreate)
      pSheet = pShtCreate;
   else
      pSheet = (sheet *)WinQueryWindowPtr(hwnd,0);

   if (ulMsg == WM_CREATE)
        pSheet->m_hwndSlide = hwnd;


    return pSheet->slwndProc(ulMsg,mp1,mp2);
}
/*-----------------------------------------------[ private ]--------------*/
static HWND createSlideWindow(sheet *pSht, HWND hParent)
{
   HWND    hwnd;
   static  CHAR szClass[]= "slideWindow";
   ULONG   ulStyle = (CS_SAVEBITS | CS_SIZEREDRAW );
   void    *pControl;

   WinRegisterClass(hab,(PSZ)szClass,
                    (PFNWP)slideWndProc,
                    ulStyle,4L);

   hwnd = WinCreateWindow(HWND_DESKTOP,
                          (PSZ)szClass,(PSZ)NULL,
                           WS_VISIBLE,
                           0,
                           0,
                           WinQuerySysValue(HWND_DESKTOP,	SV_CXSCREEN),
                           WinQuerySysValue(HWND_DESKTOP,	SV_CYSCREEN),
                           hParent,
                           HWND_TOP,
                           ID_VALUESET+1,
                           (void *)pSht,        // control data
                           (PVOID)NULL);

    //WinStartTimer(hab,hwnd,ID_TIMER,ID_SLIDETIMEOUT);

    pControl = WinQueryWindowPtr(hwnd,QWL_USER);
    if (pControl==NULL)
        WinSetWindowPtr(hwnd,QWL_USER,(PVOID)pSht);

    WinSetPresParam(hwnd,PP_FONTNAMESIZE,(ULONG)strlen("8.Helv")+1,(void *)"8.Helv");
    return hwnd;
}
/*------------------------------------------------------[ private ]-------*/
void sheet::destroySlideShowWindow(void)
{
    hPicture * pC;

    cancelDetailLoad(TRUE);

    pC = picList->getNextSelected(NULL); // with null gives first

    while (pC)
    {
        pC->freeDetailImage();
        pC = picList->getNextSelected(pC);
    }

    if (m_hwndSlide)
    {
        WinStopTimer(hab,m_hwndSlide,ID_TIMER);
        WinDestroyWindow(m_hwndSlide);
    }
    m_hwndSlide = NULL;
    changeOpMode(OP_SLIDES);
    WinSetFocus(HWND_DESKTOP,m_hwndSheet);
}
/*------------------------------------------------------[ private ]-------*/
hPicture * sheet::loadNextSelected(HWND hwnd,hPicture * hPict)
{
    hPicture * pC;

    if (!picList)
        return NULL;

    if (hPict)
        hPict->freeDetailImage();

    if (!picList)
        return NULL;

    pC = picList->getNextSelected(hPict); // with null gives first

   //printf("loadNextSelected on window %08lx\n",hwnd);
    if (pC)
        loadDetailForImage(hwnd,(HAB)0,pC);
    else
        destroySlideShowWindow();
    return pC;
}
/*---------------------------------------------------------------------------*/
/* Parameters   : HWND hwnd - Window handle.                                 */
/*                                                                           */
/* returns      : MRESULT - Message result.                                  */
/*---------------------------------------------------------------------------*/
MRESULT EXPENTRY progressBarWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
   HPS hps;
   RECTL  rcl;
   static int count;        /* # files loaded */
   static int iTotal;       /* total # files  */
   static int iFitt;        /* how many blocks fitt in this window */
   int iNr,x,mod;
   POINTL ptl,ptle;

   switch(msg)
   {
    case UM_SHOWFILENAME:
        count++;
        iTotal = (int)mp2;
        WinQueryWindowRect(hwnd,&rcl);
        iFitt = (rcl.xRight/12);
        return 0;
      case WM_CREATE:
         count = 0;
         return (MRESULT)0;
      case WM_PAINT:
          iNr = 1;      /* # blocks per file */
          mod = 1;
         WinQueryWindowRect(hwnd,&rcl);         
         hps = WinBeginPaint (hwnd,(HPS)0,&rcl);
         WinFillRect(hps,&rcl,CLR_PALEGRAY);
         WinDrawBorder(hps, &rcl,1L,1L,0L, 0L,DB_DEPRESSED);
         ptl.x = 4;
         ptl.y = 4;
         ptle.x = 10;
         ptle.y = 10;
         if (iTotal && iTotal < iFitt)
            iNr = (iFitt / iTotal)+1;
         else if (iTotal == 1)
             iNr = iFitt;
         else if (iTotal && iTotal > iFitt)
         {
             mod = iTotal / iFitt;
         }
         
         GpiSetColor(hps,CLR_BLUE);
         for ( int i = 0; i < count; i += mod)
         {
             for ( x = 0; x < iNr; x++)
             {
                 GpiMove(hps,&ptl);
                 GpiBox(hps,DRO_FILL,&ptle,0,0);
                 ptl.x += 12;
                 ptle.x+= 12;
             }
         }
         WinEndPaint(hps);
         return 0;
   }
   return (WinDefWindowProc(hwnd,msg,mp1,mp2));
}
/*------------------------------------------------------[ private ]-------*/
MRESULT EXPENTRY progressDlgProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2 )
{
    static BOOL    bInit;
    static HWND    hProgress;
    SWP    swp;    /* Screen Window Position Holder     */
   

    switch (msg)
    {
    case WM_INITDLG:
        bInit = TRUE;
           /* Centre dialog	on the screen			*/
        WinQueryWindowPos(hwnd, (PSWP)&swp);
        WinSetWindowPos(hwnd, HWND_TOP,
           ((WinQuerySysValue(HWND_DESKTOP,	SV_CXSCREEN) - swp.cx) / 2),
           ((WinQuerySysValue(HWND_DESKTOP,	SV_CYSCREEN) - swp.cy) / 2),
           0, 0, SWP_MOVE);
        hProgress = WinWindowFromID(hwnd, ID_PROGRESSWND);
        bInit = FALSE;
        return 0;

    case WM_COMMAND:
        switch(LOUSHORT(mp1))
	    {
        case DID_CANCEL:
            cancelLoad( TRUE );
            return 0;

        }
        return 0;
    case UM_SHOWFILENAME:
        WinSendMsg(hProgress,msg,mp1,mp2);
        WinSetDlgItemText (hwnd,TXT_FILENAME,(PSZ)mp1);
        WinInvalidateRect (hProgress,NULL,FALSE);
        free((void *)mp1);
        return 0;

    }
    return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}
/*------------------------------------------------------[ private ]-------*/
/*  Name: getDropAction                                                   */
/*                                                                        */
/*  Description : Functions checks the extension in the filename to see   */
/*                if we can load it. Only used in dragover to show user   */
/*                if we can accept a drop.                                */
/*                                                                        */
/*  Parameters  : char *szFilename - contains the filename.               */
/*                                                                        */
/*  Returns SHORT 0 on failure. DO_IMAGEDROP | DO_METAFILEDROP            */
/*------------------------------------------------------------------------*/
static SHORT getDropAction(char *szFilename)
{
   char sztmp[CCHMAXPATH];
   char *pDot;
   long i;

   char *szcopy = &sztmp[0];

   szcopy = strupr(szFilename);
//   printf("SuppBmps: szcopy[%s]\n",szcopy);
   pDot = strrchr(szcopy,'.');

   if (!pDot)
      return 0;

   pDot++; /* Jump over dot in filename */

   if (!strncmp("MET",pDot,3))
      return DO_METAFILEDROP;

   if (!strncmp("JSP",pDot,3))
      return DO_DRAWITFILEDROP;

   if (!strncmp("WMF",pDot,3))
      return DO_WINDOWSWMFDROP;

//   printf("SuppBmps: pDot[%s]\n",pDot);
   for (i=0; i < 1 && suppfiles[i]; i++)
      if (!strncmp(suppfiles[i],pDot,3))
         return DO_IMAGEDROP;
   return 0;
}
/*---------------------------------------------------------------------------*/
void sheet::initThreadBlock(LOADERDEF *pLoader)
{
    pLoader->hwndClient = m_hwndSheet;
    pLoader->cxSheet    = m_cxSheet;
    pLoader->cySheet    = m_cySheet;
    pLoader->pList      = picList;
    pLoader->hab        = hab;
}
/*--------------------------- sheet::initDrag -------------------------------*/
/*                                                                           */
/* Function        : sheet::initDrag.                                        */
/*                                                                           */
/* Description     : Sets up the drag info structures. This means we are the */
/*                   source where we drag off.                               */
/*                                                                           */
/* Return values   : BOOL TRUE on success.                                   */
/*---------------------------------------------------------------------------*/
BOOL sheet::initDrag (HWND hWindow, POINTL ptlMouse )
{
   CHAR             chDrive,*p;
   ULONG            ulItemsDragged,i;
   HWND             hwndTarget;
   DRAGITEM         *pDragitem,*pt;
   HSTR             hstrType, hstrRMF, hstrContainerName;
   USHORT           fsControl;
   USHORT           fsSupportedOps;
   CHAR             szFileName [64];
   CHAR             szFullContainerName [CCHMAXPATH];
   int              handle;
   PDRAGINFO        pSourceDraginfo;
   DRAGIMAGE        dimg;
   HBITMAP          hbm;
   int              cxImg,cyImg;
   float            fScale,fx,fy;
   hPicture *pFirst = NULL;                     // in case one image is selected....
   hPicture *pPict  = NULL;


   hbm = (HBITMAP)0;

   if (!picList)
       return FALSE;

   pFirst = pPict = picList->getNextSelected(NULL); // with null gives first

   ulItemsDragged = picList->countSelected();

   if (ulItemsDragged == 1 || !ulItemsDragged || !pPict)
   {      
       pFirst = pPict = picList->selectPicture(m_hwndSheet,NULL,&ptlMouse,FALSE);
       ulItemsDragged = 1;
   }

   if (!ulItemsDragged || !pPict)
       return FALSE;

   pt = pDragitem = (DRAGITEM  *)calloc(sizeof(DRAGITEM)*ulItemsDragged,1);
   m_pDragitems    = pt;
   m_lItemsDragged = ulItemsDragged;
   /*
   ** Allocate the DRAGINFO structure
   */
   pSourceDraginfo = DrgAllocDraginfo(ulItemsDragged);
   /*
   ** Initialize variable used to initialize the internal DRAGITEM
   ** structure.
   */
   hstrType  = DrgAddStrHandle ((PSZ)DRT_BITMAP);  /* file type is bitmap  */
   hstrRMF   = DrgAddStrHandle ((PSZ)"<DRM_OS2FILE,DRF_BITMAP>,<DRM_DISCARD,DRF_BITMAP>");

   for (i = 0; i < ulItemsDragged && pPict; i++)
   {
       strcpy(szFullContainerName,pPict->getFileName());
       p  = strrchr(szFullContainerName,'\\');
       if (p )
           *p = 0;

      //printf("INITDRAG: Containername = %s\n",szFullContainerName);

       hstrContainerName = DrgAddStrHandle((PSZ)szFullContainerName);

       fsControl = 0;
       chDrive = (CHAR)szFullContainerName[0];

       if((chDrive=='A') || (chDrive=='B') || (chDrive=='a') || (chDrive=='b'))
       {       /* is source container removable media?   */
          fsControl = fsControl | (SHORT)DC_REMOVEABLEMEDIA;
       }

       fsSupportedOps = DO_COPYABLE | DO_MOVEABLE;    /* can only move or copy */


       /*
       ** 1 - Generate unique filename.
       ** 2 - Save bitmap with this file into the 'container' so
       **     the target can get it out.
       */
       strcpy(szFileName,pPict->getShortName());
       /*
       ** Initialize common portions of the internal Dragitem buffer.
       */

       pDragitem->hwndItem          = hWindow;
       pDragitem->hstrType          = hstrType;
       pDragitem->hstrRMF           = hstrRMF;
       pDragitem->hstrContainerName = hstrContainerName;
       pDragitem->fsControl         = fsControl;
       pDragitem->fsSupportedOps    = fsSupportedOps;
       pDragitem->hstrSourceName    = DrgAddStrHandle ((PSZ)szFileName);
       pDragitem->hstrTargetName    = pDragitem->hstrSourceName;
       pDragitem->ulItemID          = ID_DRGITEM+i;
       pDragitem->fsControl         = pDragitem->fsControl | (SHORT)DC_CONTAINER;

       if(!DrgSetDragitem(pSourceDraginfo, pDragitem, sizeof(DRAGITEM), i))
       {
          return FALSE;
       }

       pPict = picList->getNextSelected(pPict);

       pDragitem++;
   }
   /*
   **  Drag the items.
   **  We stretch the image into a given
   **  rectangle...
   **  Bassically our rect is DRG_IMGSIZE pixels
   **  which will be adjusted to the aspectratio
   **  of the picture.
   */
   pSourceDraginfo->hwndSource = hWindow;

   pPict = pFirst;

   cxImg = pPict->getWidth();
   cyImg = pPict->getHeight();

   if (cxImg > cyImg )
   {
      dimg.sizlStretch.cx = DRG_IMGSIZE;
      fx = (float)cxImg;
      fy = (float)cyImg;
      fScale = (float)fy/fx;
      dimg.sizlStretch.cy = (int)(DRG_IMGSIZE * fScale);
   }
   else
   {
      dimg.sizlStretch.cy = DRG_IMGSIZE;
      fx = (float)cxImg;
      fy = (float)cyImg;
      fScale = (float)fx/fy;
      dimg.sizlStretch.cx = (int)(DRG_IMGSIZE * fScale);
   }
   
   if (ulItemsDragged == 1 )
        hbm = pPict->pict2BitmapHandle();
   else
   {
        if ((hbm = GpiLoadBitmap(getPS(),(HMODULE)0,ID_DRAGBMP,32,32))== 0)
            hbm = pPict->pict2BitmapHandle();
   }

   dimg.cb       = sizeof (DRAGIMAGE);
   dimg.hImage   = hbm;
   dimg.fl       = DRG_BITMAP | DRG_STRETCH;
   dimg.cxOffset = 0;
   dimg.cyOffset = 0;
   hwndTarget=DrgDrag(hWindow, pSourceDraginfo, &dimg, 1L, VK_BUTTON2, NULL);

   if (hwndTarget == NULLHANDLE)
   {
     /*
      * User cancelled the drag or asked for help, so we can release
      * the resources allocated to the drag operation.
      */
      if(!DrgFreeDraginfo(pSourceDraginfo))
         return FALSE;
   }

   GpiDeleteBitmap(hbm);

   free(pt);
   return TRUE;
}
/*------------------------------------------------------------------------*/
MRESULT sheet::dragDiscardObject(HWND hwnd, HAB hab, PDRAGINFO pDraginfo)
{
    USHORT        cItems,usIndex;
    PDRAGITEM     pditem;
    ULONG         ulBytesWritten;
    char          SourceDir [CCHMAXPATH];
    char          szSourceFile[CCHMAXPATH];
    char          szCheck[CCHMAXPATH];
    APIRET        ulrc;
    BOOL          bDeleted = FALSE;
    /*
    * Get access to the DRAGINFO structure.
    */
    if(!DrgAccessDraginfo(pDraginfo))
    {
       //printf("Cannot access draginfo\n");
        return (MRESULT)0;
    }
    cItems = DrgQueryDragitemCount (pDraginfo);

    for (usIndex = 0; usIndex < cItems; usIndex++)
    {
        pditem = DrgQueryDragitemPtr (pDraginfo, usIndex);
        /*
        ** Try to check on the containername source name...
        */
        ulBytesWritten = DrgQueryStrName(pditem->hstrSourceName,sizeof(szSourceFile),
                                         (PSZ)szSourceFile);
        if (ulBytesWritten)
            ulBytesWritten = DrgQueryStrName(pditem->hstrContainerName,
                                             sizeof(SourceDir),
                                             (PSZ)SourceDir);


        if (SourceDir[strlen(SourceDir)-1]!='\\')
            sprintf(szCheck,"%s\\%s",SourceDir,szSourceFile);
        else
            sprintf(szCheck,"%s%s",SourceDir,szSourceFile);

        if (ulBytesWritten)
        {
            ulrc = DosDelete((PSZ)szCheck);
           //printf("Dropped on shredder:%s\n",szCheck);

            if (ulrc)
            {
                sprintf(szCheck,"DosDelete error : %d",ulrc);
                WinMessageBox(HWND_DESKTOP,hwnd,
                                       (PSZ)szCheck,(PSZ)"ERROR", 0,
                                        MB_OK | MB_APPLMODAL | MB_MOVEABLE | 
                                        MB_ICONASTERISK);

            }
            else
            {
                if (picList->deletePicture( szSourceFile ))
                    bDeleted = TRUE;

            }
        }
    }
    DrgDeleteDraginfoStrHandles((PDRAGINFO)pDraginfo);
    DrgFreeDraginfo(pDraginfo);

    if (bDeleted)
        sizeSlideSheet();

    return (MRESULT)0;
}
/*------------------------------------------------------------------------*
 *
 *  Name       : DragOver(pDraginfo, pCurrentDir)
 *
 *  Description: Provides visual feedback to the user as to whether it
 *               can support the drag operation
 *
 *  Concepts   :  direct manipulation
 *
 *  API's      :  DrgAccessDraginfo
 *                DrgFreeDraginfo
 *                DrgQueryDragitemPtr
 *                DrgQueryStrName
 *                DrgQueryDragitemCount
 *                DrgVerifyRMF
 *                WinGetLastError
 *
 *  Parameters :  pDraginfo  = pointer to drag information structure
 *                pCurrentDir= the name of the current directory
 *
 *  Return     :  DOR_DROP, DO_MOVE for a default operation
 *
 *--------------------------------------------------------------------------*/
MRESULT sheet::dragOver(HWND hwnd, HAB hab, PDRAGINFO pDraginfo, PSZ pCurrentDir)
{
   USHORT        cItems,usIndex;
   ULONG         ulBytesWritten;
   USHORT        usDropAction;      /* DO_IMAGEDROP | DO_METAFILEDROP | 0 */
   PDRAGITEM     pditem;
   CHAR          SourceDir [CCHMAXPATH];
   char          szSourceFile[CCHMAXPATH];
   char          szTarget[CCHMAXPATH];
   USHORT        usIndicator;
  /*
   * Get access to the DRAGINFO structure.
   */
   if(!DrgAccessDraginfo(pDraginfo))
   {
     //printf("Cannot access draginfo\n");
      return (MRFROM2SHORT (DOR_NODROPOP, 0));
   }

   usDropAction = pDraginfo->usOperation;
   /*
    * Determine default operation if current operation is default.
    * The default is move unless either the source or the target is
    * on different removable media.
   */
   pditem = DrgQueryDragitemPtr (pDraginfo, 0);
   ulBytesWritten = DrgQueryStrName(pditem->hstrContainerName,
                                    sizeof(SourceDir),
                                    (PSZ)SourceDir);

   DrgQueryStrName(pditem->hstrTargetName,CCHMAXPATH,(PSZ)szTarget);


//   printf("hstrTargetName %s  Source %s\n",szTarget,SourceDir);

   if(ulBytesWritten == 0L)
   {

//      if(ulBytesWritten)


      ulBytesWritten = DrgQueryStrName(pditem->hstrContainerName,
                                    sizeof(SourceDir),
                                    (PSZ)SourceDir);

//      if(ulBytesWritten)
//         printf("hstrContainerName %s\n",SourceDir);

      return (MRFROM2SHORT (DOR_NODROPOP, 0));
   }

    /* Can we accept this drop? */
    switch (pDraginfo->usOperation)
    {

      /* Return DOR_NODROPOP if current operation */
      /* is link or unknown                       */
      case DO_UNKNOWN:
         DrgFreeDraginfo(pDraginfo);
         return (MRFROM2SHORT (DOR_NODROPOP, 0));
         break;

      /* Our default operation is Move */
      case DO_DEFAULT:
         usDropAction =  DO_MOVE;
         break;

      /* Do the requested specific operation */
      case DO_MOVE:
      case DO_COPY:
         break;
    }

   /*
   ** In order to support the operation, the source must support the
   ** operation this target has decided upon.  The source must also
   ** support a rendering mechanism of <DRM_OS2FILE,DRF_UNKNOWN>.
   ** This target doesn't care about the file type.
   */
   cItems = DrgQueryDragitemCount (pDraginfo);
   /*
   ** inspect the first item to see if it is acceptable
   */
   pditem = DrgQueryDragitemPtr (pDraginfo, 0);
   /*
   ** If source same as target window, refuse!
   */
   if (hwnd == pditem->hwndItem)
        return (MRFROM2SHORT (DOR_NODROPOP, 0));

    usIndicator = DOR_DROP;

    for (usIndex = 0; usIndex < cItems && usIndicator == DOR_DROP; usIndex++)
    {
        pditem = DrgQueryDragitemPtr (pDraginfo, usIndex);
        /*
        ** Try to check on the containername source name...
        */
        DrgQueryStrName(pditem->hstrSourceName,sizeof(szSourceFile),
                        (PSZ)szSourceFile);

        //usDropAction = getDropAction(szSourceFile);

          /* Make sure we can move for a Move request */
          /* or copy for a Copy                       */
        if (((pditem->fsSupportedOps & DO_COPYABLE)   &&
            (usDropAction == (USHORT)DO_COPY))        ||
            ((pditem->fsSupportedOps & DO_MOVEABLE)   &&
            (usDropAction == (USHORT)DO_MOVE)))
        {
            usIndicator = DOR_DROP;
        }
        else
        {
           //printf("pditem->fsSupportedOps=%d\n",pditem->fsSupportedOps);
            usIndicator = DOR_NODROPOP;
        }

        if (!getDropAction(szSourceFile))
        {
            /*
            ** One of the items is not supported by us,
            ** cancel acceptance
            */
            DrgFreeDraginfo(pDraginfo);
            return (MRFROM2SHORT (DOR_NODROPOP, 0));
        }
    }
//    printf("DragOver: szSourceFile [%s] dropaction %d\n",szSourceFile,usDropAction);

    if(!usDropAction && !DrgFreeDraginfo(pDraginfo))
    {
        if(PMERR_SOURCE_SAME_AS_TARGET != WinGetLastError(hab))
        {
            return (MRFROM2SHORT (DOR_NODROPOP, 0));
        }
    }
    return (MRFROM2SHORT(usIndicator,usDropAction));
}  /*  End of DragOver  */

/*------------------------------------------------------------------------*/
/*  Name: Drop                                                            */
/*                                                                        */
/*  Description : Called directly from a drop event. We only accept       */
/*                images for the moment. Color drops and fonts are handled*/
/*                in the windowproc itself.                               */
/*                of objects.                                             */
/*  Parameters  : HAB - Anchor block handle of the calling thread.        */
/*                PDRAGINFO - pointer to the drag info structure.         */
/*                                                                        */
/*  Returns : Message result 0.                                           */
/*------------------------------------------------------------------------*/
MRESULT sheet::drop(HAB hab,PDRAGINFO pDraginfo)
{
   char  szSourceFile[CCHMAXPATH];
   char  szContain[CCHMAXPATH];
   char  szFilename[CCHMAXPATH];
   char  szCheck[CCHMAXPATH];
   char  szWarning[50];
   ULONG ulBytesWritten;
   RECTL   rcl;
   POINTL  ptlPos;
   char    *p;
   SHORT   xPos,yPos;
   USHORT  usResponse;
   PDRAGITEM pditem = NULL;
   struct stat bf;
   TID         ThreadID;       /* New thread ID (returned)                   */
   ULONG       ThreadFlags;    /* When to start thread,how to allocate stack */
   ULONG       StackSize;      /* Size in bytes of new thread's stack        */
   USHORT      cItems,usIndex;
   dropcopyblock *pBlock;
   unsigned int  *pIAction;
   /*
   ** Get access to the Draginfo structure.
   */
   if(!DrgAccessDraginfo (pDraginfo))
   {
     //printf("Drop DrgAccessDraginfo ERROR[%d]\n",WinGetLastError(hab));
      return (MRESULT)0;
   }

   ptlPos.x = pDraginfo->xDrop;
   ptlPos.y = pDraginfo->yDrop;


   szWarning[0] = 0;

   cItems = DrgQueryDragitemCount (pDraginfo);

   
   if (!(pDraginfo->usOperation == DO_IMAGEDROP || pDraginfo->usOperation == DO_COPY ||
         pDraginfo->usOperation == DO_MOVE)  || m_szFolder[0] == 0)
   {
       DrgDeleteDraginfoStrHandles((PDRAGINFO)pDraginfo);
       DrgFreeDraginfo(pDraginfo);

       return (MRESULT)NULL;
   }

    pIAction = ( unsigned int *)calloc(sizeof(int),cItems);

   for ( usIndex = 0;  usIndex < cItems; usIndex++)
   {
        pditem = DrgQueryDragitemPtr (pDraginfo,usIndex);
        /*
        ** Lets get the filename!
        */
        ulBytesWritten =DrgQueryStrName(pditem->hstrSourceName,
                                      sizeof(szSourceFile),
                                      (PSZ)szSourceFile);
        if (!ulBytesWritten)
         return (MRESULT)0;

        ulBytesWritten =DrgQueryStrName(pditem->hstrContainerName,
                                      sizeof(szContain),
                                      (PSZ)szContain);
        if (!ulBytesWritten)
         return (MRESULT)0;

        if (szContain[strlen(szContain)-1]!='\\')
         sprintf(szFilename,"%s\\%s",szContain,szSourceFile);
        else
         sprintf(szFilename,"%s%s",szContain,szSourceFile);

       //printf("DROP:szFilename = %s  pDraginfo->usOperation = %d\n",szFilename,pDraginfo->usOperation);

        if (m_szFolder[strlen(m_szFolder)-1]!='\\')
            sprintf(szCheck,"%s\\%s",m_szFolder,szSourceFile);
        else
            sprintf(szCheck,"%s%s",m_szFolder,szSourceFile);

       //printf("Target %s Source %s\n",szCheck,szFilename);
        
        if (_stat(szCheck,&bf) == 0)
        {
            WinLoadString(hab, (HMODULE)0, IDS_REPLACE_FILE ,
                           CCHMAXPATH, (PSZ)szErrBuf);

            sprintf(szCheck,(const char *)szErrBuf,szSourceFile);

            if (!*szWarning)
                WinLoadString(hab, (HMODULE)0, IDS_WARNING ,
                              50, (PSZ)szWarning);

            usResponse = WinMessageBox(HWND_DESKTOP,m_hwndClient,
                                       (PSZ)szCheck,(PSZ)szWarning, 0,
                                       MB_YESNOCANCEL | MB_APPLMODAL |
                                       MB_MOVEABLE | 
                                       MB_ICONQUESTION);
            if ( usResponse == MBID_NO )
            {
                pIAction[usIndex]=UACT_SKIP;
            }
            else if ( usResponse == MBID_CANCEL )
            {
                DrgDeleteDraginfoStrHandles((PDRAGINFO)pDraginfo);
                DrgFreeDraginfo(pDraginfo);
                free(pIAction);
                return (MRESULT)NULL;
            }    
            else
                pIAction[usIndex]=UACT_OVERWRITE;
        }
        else
        {
            pIAction[usIndex]=UACT_NEWITEM;
        }
    }

   if ( pDraginfo->usOperation == DO_COPY || pDraginfo->usOperation == DO_MOVE )
   {
         pBlock = ( dropcopyblock * )calloc(sizeof(dropcopyblock),1);
         initThreadBlock(&pBlock->loader);
         pBlock->pAction         = pIAction;
         pBlock->pDraginfo       = pDraginfo;
         pBlock->pszTargetFolder = m_szFolder;

         ThreadFlags = 0;        /* Indicate that the thread is to */
                                 /* be started immediately         */
         StackSize = 14096;      /* Set the size for the new       */
                                 /* thread's stack                 */
         DosCreateThread(&ThreadID,(PFNTHREAD)dropCopyThread,
                         (ULONG)pBlock,ThreadFlags,StackSize);
     
   }
   else
   {
      //printf("Drop no actions for us\n");
       DrgDeleteDraginfoStrHandles((PDRAGINFO)pDraginfo);
       DrgFreeDraginfo(pDraginfo);
       free(pIAction);
   }
    return (MRESULT)NULL;
}    /*  End of Drop  */
/*---------------------------------------------------------------------------*/
MRESULT sheet::dragEndConversation(MPARAM mp1,MPARAM mp2)
{
    char   szSourceFile[150];
    char   szContainer[150];
    char   szFile[CCHMAXPATH];
    int i;

   //printf("ITEM ID %d \n",(int)mp1);
   //printf("result  %d m_lItemsDragged=%d\n",(int)mp2,m_lItemsDragged);
    for (i = 0; i < m_lItemsDragged; i++)
    {
       //printf("id's in  m_pDragitems %d \n",m_pDragitems[i].ulItemID);

        if (m_pDragitems[i].ulItemID == (int)mp1 )
        {

            DrgQueryStrName(m_pDragitems[i].hstrSourceName,
                            sizeof(szSourceFile),
                           (PSZ)szSourceFile);
           //printf("Target told me it was done with... %s\n",szSourceFile);
            DrgQueryStrName(m_pDragitems[i].hstrContainerName,
                            sizeof(szContainer),
                           (PSZ)szContainer);

           //printf("Container name  %s\n",szContainer);

            if (szContainer[strlen(szContainer)-1]!='\\')
                sprintf(szFile,"%s\\%s",szContainer,szSourceFile);
            else
                sprintf(szFile,"%s%s",szContainer,szSourceFile);

       
//            if ( (m_pDragitems[i].fsSupportedOps & DO_MOVEABLE ) && 
//                !(m_pDragitems[i].fsSupportedOps & DO_COPYABLE))
//                printf("Only move!\n");
//            printf("File : %s\n",szFile);

            if (( m_pDragitems[i].fsControl & DC_REMOVEABLEMEDIA ))
            {
               //printf("Source is DC_REMOVEABLEMEDIA\n");
                if (m_bShift)
                {
                    printf("Move to DC_REMOVEABLEMEDIA requested....\n");

                }
                return (MRESULT)0;
            }
            else
            {
                if (!m_bControl)
                {
                    printf("Move requested. file %s\n",szFile);
                }
            }

        }
    }
    return (MRESULT)0;
}
/*****************************************************************************/
/*                                                                           */
/* Create a memory DC and an associated PS.                                  */
/*                                                                           */
/*****************************************************************************/
BOOL CreateBitmapHdcHps( PHDC phdc, PHPS phps)
{
  SIZEL    sizl;
  HDC      hdc;
  HPS      hps;

  hdc = DevOpenDC(hab,OD_MEMORY,(PSZ)"*",0L,NULL,NULLHANDLE);
  if( !hdc)
    return( FALSE);

  sizl.cx = sizl.cy = 0L;
  hps = GpiCreatePS( hab , hdc, &sizl, PU_PELS | GPIA_ASSOC | GPIT_MICRO );

  if( !hps)
    return( FALSE);

  *phdc = hdc;
  *phps = hps;
  return( TRUE);
}
/*------------------------------------------------------------------------*/
MRESULT sheet::wndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
    PDRAGINFO pDInfo;                              /* Pointer to DRAGINFO      */
    PDRAGITEM pDItem;                              /* Pointer to DRAGITEM      */
    HPS    hps;
    SIZEL    sizl;
    RECTL  rcl;
    RECTL  rclWnd;
    static LONG cx,cy;
    static BOOL bActive;
    static POINTL ptlPosn;
    USHORT usReply;
    USHORT  usRow,usCol,usIndex;
    hPicture *pPict;
    HDC    hdcClient;
    BOOL   bShift;

    switch (ulMsg)
    {
    case WM_CREATE:
        hdcClient = WinOpenWindowDC(m_hwndSheet);
        hps = GpiCreatePS(hab,hdcClient, &sizl, PU_PELS | GPIF_DEFAULT | GPIT_NORMAL  | GPIA_ASSOC);
        setPS(hps);
        return 0;
     case WM_BUTTON1DBLCLK:
        ptlPosn.x   = SHORT1FROMMP(mp1); /* @check */
        ptlPosn.y   = SHORT2FROMMP(mp1);
        handleMouseBtn1dblclk(ptlPosn,mp2);
        return 0;    
    case WM_BUTTON2DOWN:
        ptlPosn.x   = SHORT1FROMMP(mp1); /* @check */
        ptlPosn.y   = SHORT2FROMMP(mp1);
        return 0;
    case WM_BUTTON1DOWN:
        ptlPosn.x   = SHORT1FROMMP(mp1);
        ptlPosn.y   = SHORT2FROMMP(mp1);
        return handleMouseButton1down(ptlPosn, mp2);

     case UM_DOSERROR:
         if (!bActive)
         {
            bActive = TRUE;
            WinMessageBox(HWND_DESKTOP,m_hwndSheet,
                                   (PSZ)mp1,(PSZ)"ERROR", 0,
                                   MB_OK | MB_APPLMODAL |
                                   MB_MOVEABLE |
                                   MB_ICONEXCLAMATION);
            bActive = FALSE;
        }
        free((void *)mp1);
        return (MRESULT)0;


     case UM_JPGERROR:
         if (m_hProgress)
         {
             WinDismissDlg( m_hProgress, TRUE );
             m_hProgress = 0;
         }

         if (!bActive)
         {
            bActive = TRUE;
            usReply = WinMessageBox(HWND_DESKTOP,m_hwndSheet,
                                   (PSZ)mp1,(PSZ)"JPEG_ERROR", 0,
                                   MB_RETRYCANCEL | MB_APPLMODAL |
                                   MB_MOVEABLE |
                                   MB_ICONEXCLAMATION);
            bActive = FALSE;
            if (usReply == MBID_RETRY && picList )
               continueOnError(m_hwndSheet,hab, picList );
            else if (picList )
            {
                picList->deleteNonLoaded();
                sizeSlideSheet();
                picList->formatFolderString(m_hwndSheet);
            }
        }
        free((void *)mp1);
        return (MRESULT)0;

     case UM_FOLDERSTRING:
        //printf("** UM_FOLDERSTRING  **\n");
         WinSendMsg(m_hwndApplic,ulMsg,mp1,mp2);           
         if (mp1)
              free((void *)mp1);
         return 0;

     case UM_IMAGESTRING:
        //printf("** UM_IMAGESTRING  **\n");
         WinSendMsg(m_hwndApplic,ulMsg,mp1,mp2);           
         if (mp1)
              free((void *)mp1);
         return 0;

      case UM_NEWLIST:
         //printf("** UM_NEWLIST  **\n");
          picList = ( picturelist * )mp2;
          WinInvalidateRect(m_hwndSheet,NULL,TRUE);
          return 0;

      case UM_DROPCOPYDONE:
          if (mp1)              /* Are the new items added to the list? */
              sizeSlideSheet();
          return 0;

      case UM_DETAIL:   /* LOADED DETAIL OF IMAGE */
         //printf("** UM_DETAIL  **\n");
          sizeDetailView( NULL);
          WinInvalidateRect(m_hwndSheet,NULL,TRUE);
          return 0;

      case UM_SHOWPROGRESS:
          if (!m_hProgress)
          {
             m_hProgress = WinLoadDlg(HWND_DESKTOP,m_hwndSheet,
                                      (PFNWP)progressDlgProc,(HMODULE)0,
                                      DLG_PROGRESS,NULL);
          }
          return 0;
      case UM_SHOWFILENAME:
          if (m_hProgress)
            WinSendMsg(m_hProgress,ulMsg,mp1,mp2);
          else if (mp1)
              free((void *)mp1);
          return 0;

      case UM_CLOSEPROGRESS:
          if (m_hProgress)
          {
              WinDismissDlg( m_hProgress, TRUE );
              m_hProgress = 0;
          }
          WinInvalidateRect(m_hwndSheet,(RECTL *)0,TRUE);
          return 0;

      case UM_COUNTPICTURES:
         sizeSlideSheet();
         return 0;
      case UM_NEWITEM:
         //printf("** UM_NEWITEM  **\n");
          pPict = (hPicture *)mp2;
          pPict->loadPicRect(&rcl);
          WinInvalidateRect(m_hwndSheet,(RECTL *)&rcl,TRUE);
          free((void *)mp1);
          return 0;

      case WM_CHAR:
         return handleKeyBoard(mp1,mp2);
       case WM_SIZE:
          cx = (LONG)SHORT1FROMMP(mp2);
          cy = (LONG)SHORT2FROMMP(mp2);
          setSheetWindowSize((int)cx,(int)cy);
          return (MRESULT)0;
       case WM_PAINT:
          hps = WinBeginPaint (m_hwndSheet,(HPS)0,&rcl);
          rclWnd.xLeft = 0;
          rclWnd.yTop  = cy;
          rclWnd.yBottom = 0;
          rclWnd.xRight = cx;
          GpiErase(hps);

          if (getOpMode() == OP_DETAILVIEW && (pPict = getSingleSelection()))
          {
              pPict->paintDetail(hps,&rclWnd);
          }
          else
          {
              paintSlides(hps,&rclWnd,&rcl);
          }
          WinEndPaint(hps);
          return (MRESULT)0;
      case DM_RENDER:
	     //printf("DM_RENDER\n");
          return (MRESULT)0;
      case DM_DRAGOVER:
          return dragOver(m_hwndSheet,hab, (PDRAGINFO)mp1,(PSZ)"\\sheet\\");
      case DM_DROP:                           /* Object being dropped     */
           return drop(hab,(PDRAGINFO)mp1);
      case DM_ENDCONVERSATION:
         //printf("DM_ENDCONVERSATION\n");
          return dragEndConversation(mp1,mp2);
          break;
      case DM_DISCARDOBJECT:
         //printf("DM_DISCARDOBJECT - SHREDDER ANSWERED ON DROP\n" );
          if (picList)
            return dragDiscardObject(m_hwndSheet,hab, (PDRAGINFO)mp1);
          else
            break;

      case WM_BEGINDRAG:
          return (MRESULT)initDrag(m_hwndSheet,ptlPosn);   
   }
   return (MRESULT)WinDefWindowProc(m_hwndSheet,ulMsg,mp1,mp2);
}
/*------------------------------------------------------------------------*/
MRESULT EXPENTRY sheetWndProc(HWND hwnd,ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
   sheet *pSheet;

   if (pShtCreate)
      pSheet = pShtCreate;
   else
      pSheet = (sheet *)WinQueryWindowPtr(hwnd,0);

    return pSheet->wndProc(ulMsg,mp1,mp2);
}
/*-----------------------------------------------[ private ]--------------*/
static HWND createSheetWindow(sheet *pSht, HWND hParent)
{
   HWND    hwndValue;   
   static  CHAR szClass[]= "sheetWindow";
   ULONG   ulStyle = (CS_SAVEBITS | CS_SIZEREDRAW );
   void    *pControl;

   WinRegisterClass(hab,(PSZ)szClass,
                    (PFNWP)sheetWndProc,
                    ulStyle,4L);

   hwndValue = WinCreateWindow(hParent,(PSZ)szClass,(PSZ)NULL,
                               WS_VISIBLE,
                               0,0,0,0,
                               hParent,
                               HWND_TOP,
                               ID_VALUESET,
                               (void *)pSht,        // control data
                               (PVOID)NULL);


    pControl = WinQueryWindowPtr(hwndValue,QWL_USER);
    if (pControl==NULL)
        WinSetWindowPtr(hwndValue,QWL_USER,(PVOID)pSht);

    WinSetPresParam(hwndValue,PP_FONTNAMESIZE,(ULONG)strlen("8.Helv")+1,(void *)"8.Helv");
    return hwndValue;
}
/*-----------------------------------------------[ private ]--------------*/
void  sheet::setSheetWindowSize( int cx, int cy)
{
    m_cySheet = cy;
    m_cxSheet = cx;
}
/*-----------------------------------------------[ private ]--------------*/
/*  Name: positionWindowInViewPort.                                       */
/*                                                                        */
/*  Description : Positions the window correctly within the viewport.     */
/*                                                                        */
/*                                                                        */
/*------------------------------------------------------------------------*/
void sheet::positionWindowInViewPort( void )
{
   LONG x,y;

   x = y = 0;

   x -= view.sHscrollPos;
   y  = m_cyClient - view.cySheet;
   y += view.sVscrollPos;

  //printf("Set windowpos sheet x=%d y=%d\n",x,y);

   WinSetWindowPos(m_hwndSheet,(HWND)0,x,y,0,0,SWP_MOVE);
}
/*------------------------------------------------------------------------*/
BOOL sheet::changeOpMode( int opmode )
{   
    if (m_opMode == opmode)
        return FALSE;

    switch ( m_opMode)
    {
    case OP_DETAILVIEW:
        if (m_pvCurrent)
        {
            m_pvCurrent->freeDetailImage();
        }
        break;
    }

    m_opMode = opmode;

    switch(opmode)
    {
    case OP_DETAILVIEW:
        if (m_pvCurrent)
        {
            sizeDetailView( m_pvCurrent );
        }
        break;
    case OP_SLIDESHOW:
        return TRUE;
    default:
        createPictureSheet(m_slideSize);
        break;
    }
    WinInvalidateRect(m_hwndSheet,(RECTL *)0,TRUE);
    return TRUE;
}
/*------------------------------------------------------------------------*/
void sheet::SetScrollBar(BOOL bSetForm)
{

   if (bSetScrollbars)
      return;

   bSetScrollbars = TRUE;

   view.sVscrollMax = max (0, view.cySheet - m_cyClient);
   view.sVscrollPos = min (view.sVscrollPos, view.sVscrollMax);

   WinSendMsg (view.hwndVscroll, SBM_SETSCROLLBAR,
               MPFROM2SHORT (view.sVscrollPos,0),
               MPFROM2SHORT (0,view.sVscrollMax));

   WinSendMsg(view.hwndVscroll, SBM_SETTHUMBSIZE,
              MPFROM2SHORT(m_cyClient,view.cySheet), 0);

   view.sHscrollMax  = max ( 0, view.cxSheet - m_cxClient);
   view.sHscrollPos  = min (view.sHscrollPos, view.sHscrollMax);

   WinSendMsg (view.hwndHscroll, SBM_SETSCROLLBAR,
               MPFROM2SHORT (view.sHscrollPos,0),
               MPFROM2SHORT (0,view.sHscrollMax));


   WinSendMsg(view.hwndHscroll, SBM_SETTHUMBSIZE,
              MPFROM2SHORT(m_cxClient,view.cxSheet), 0);

   WinEnableWindow (view.hwndHscroll, view.sHscrollMax ? TRUE : FALSE);

   if (bSetForm )
   {
      positionWindowInViewPort();
   }
   bSetScrollbars = FALSE;
}
/*---------------------------------------------------------------------------*/
VOID sheet::sizeSlideSheet( void )
{
   LONG cxSheet,cySheet;
   LONG xPos,yPos;
   if (!m_hwndSheet)
      return;

   cxSheet = max(m_cxClient,CX_SHEET);

   view.cxSheet = cxSheet;

    m_Columns = cxSheet / (m_slideSize + PIC_SPATIE);

    if (picList)
        m_Rows = picList->calcRowCount(m_slideSize,cxSheet);
    else
        m_Rows = 100;

    view.cySheet = cySheet = (m_slideSize + PIC_SPATIE) * m_Rows;
    view.cySheet = cySheet = max(cySheet,CY_SHEET);

    xPos    = 0;
    yPos    = 0;
    yPos    = m_cyClient - cySheet;
   
    if (picList)
        picList->reCalcPictureRect((HWND)0,m_slideSize,cxSheet,cySheet);

    WinSetWindowPos(m_hwndSheet,HWND_TOP,xPos,yPos,cxSheet,cySheet,
                    SWP_MOVE | SWP_SIZE | SWP_ZORDER );
    SetScrollBar(TRUE);
    WinInvalidateRect(m_hwndSheet,(RECTL *)0,TRUE);
}
/*---------------------------------------------------------------------------*/
/* sheet::rePositionSlides                                                   */
/*---------------------------------------------------------------------------*/
void sheet::rePositionSlides( void )
{
    if (picList)
    {
        picList->reCalcPictureRect(m_hwndSheet,m_slideSize,view.cxSheet,view.cySheet);
    }
}
/*---------------------------------------------------------------------------*/
VOID sheet::sizeDetailView( hPicture *pPict )
{
   LONG cxSheet,cySheet;
   LONG xPos,yPos;
   
   if (!pPict)
       pPict = m_pvCurrent;

    if (!m_hwndSheet || !pPict)
        return;

    cxSheet = pPict->getDetailWidth();
    cySheet = pPict->getDetailHeight();

    view.cxSheet = cxSheet;
    view.cySheet = cySheet;

    xPos    = 0;
    yPos    = 0;
    yPos    = m_cyClient - cySheet;
   
    WinSetWindowPos(m_hwndSheet,HWND_TOP,xPos,yPos,cxSheet,cySheet,
                    SWP_MOVE | SWP_SIZE | SWP_ZORDER );
    SetScrollBar(TRUE);
}

/*---------------------------------------------------------------------------*/
MRESULT sheet::paintSlides(HPS hps, RECTL *rclSheet, RECTL *prclInv)
{
    hPicture *pPict;    
  
   //printf("paintSlides\n");
    if (!picList)
        return (MRESULT)0;

    pPict = picList->getFirstPicture();

    while (pPict)
    {
        pPict->paint(hps,prclInv,m_bShowName);
        pPict = picList->getNextPicture(pPict);
    }
   //printf("paintSlides done?\n");
    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
void sheet::showTitles( BOOL bShow )
{
   BOOL bChange = (m_bShowName != bShow );

   m_bShowName = bShow;

   if (bChange)
      WinInvalidateRect(m_hwndSheet,(RECTL *)0,TRUE);
   
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::loadPicturesFromDir(char * pszDirectory)
{

   if (!pszDirectory )
        return (MRESULT)0;

   if (!pszDirectory[0])
        return (MRESULT)0;

   if (picList && m_szFolder[0] && !stricmp(m_szFolder,pszDirectory))
	   return (MRESULT)0;

	if (picList && !m_hProgress)
	{
      changeOpMode(OP_SLIDES);
      cancelLoad( TRUE );
      delete picList;
	}
    else if (picList && m_hProgress)
    {
      cancelLoad( TRUE );
      strcpy(m_szFolder,pszDirectory);
      return (MRESULT)0;
    }
    picList = NULL;

   m_szFolder[0]=0;

   if (!m_hwndSheet)
       createPictureSheet(SLIDE_SMALL);

   strcpy(m_szFolder,pszDirectory);

   WinPostMsg(m_hwndSheet,UM_IMAGESTRING, (MPARAM)strdup(" "),0);
   WinPostMsg(m_hwndSheet,UM_FOLDERSTRING,(MPARAM)strdup(" "),0);

   createPictureList(m_hwndSheet,(HAB)0,pszDirectory,m_cxSheet,m_cySheet,m_slideSize);
   return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::forceReloadPictures(void)  /* <F5> */
{

    if (!m_szFolder[0] || m_hProgress)
        return (MRESULT)0;

	if (picList)
	{
      changeOpMode(OP_SLIDES);
      cancelLoad( TRUE );
      delete picList;
	}
    picList = NULL;

    if (!m_hwndSheet)
        createPictureSheet(SLIDE_SMALL);

    WinPostMsg(m_hwndSheet,UM_IMAGESTRING, (MPARAM)strdup(" "),0);
    WinPostMsg(m_hwndSheet,UM_FOLDERSTRING,(MPARAM)strdup(" "),0);

    createPictureList(m_hwndSheet,(HAB)0,m_szFolder,m_cxSheet,m_cySheet,m_slideSize);
    return (MRESULT)0;
}
/*-----------------------------------------------[ public ]------------------*/
MRESULT sheet::showDetailOnSelection( void )
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList && m_pvCurrent)
    {
        changeOpMode(OP_DETAILVIEW);
        loadDetailForImage(m_hwndSheet,(HAB)0,m_pvCurrent);
    }
    return (MRESULT)0;    
}
/*-----------------------------------------------[ public ]------------------*/
MRESULT sheet::sortOnSize ( void )
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList)
    {
        if ( picList->sortPicture(SORT_SIZE))
        {
            rePositionSlides();
        }
    }
    
    return (MRESULT)0;    
}
/*-----------------------------------------------[ public ]------------------*/
MRESULT sheet::sortOnName ( void )
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList)
    {
        if ( picList->sortPicture(SORT_NAME))
        {
            rePositionSlides();
        }
    }
    
    return (MRESULT)0;    
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::selectAll( void )
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList)
    {
        picList->selectAllPicture(m_hwndSheet,TRUE,TRUE);
    }
    return (MRESULT)0;    
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::unSelectAll( void )
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList)
    {
        picList->selectAllPicture(m_hwndSheet,FALSE,TRUE);
        m_pvCurrent=NULL;
    }
    return (MRESULT)0;    
}
/*-----------------------------------------------[ public ]------------------*/
MRESULT sheet::deleteSelected ( void )                  //app calls this from menu & VK_DELETE!
{
    ULONG ulItems;
    char szWarning[50];
    char szExpl[150];
    USHORT usResponse;
    QMSG   qmsg;

    if (isKeyboardBlocked())
        return 0;

    if (picList && getOpMode() != OP_DETAILVIEW)
    {
        ulItems = picList->countSelected();
        if (ulItems)
        {
            WinLoadString(hab, (HMODULE)0, IDS_WARNING ,
                          50, (PSZ)szWarning);

            WinLoadString(hab, (HMODULE)0, IDS_DELQUESTION,
                          150, (PSZ)szExpl);

            usResponse = WinMessageBox(HWND_DESKTOP,m_hwndClient,
                                   (PSZ)szExpl,(PSZ)szWarning, 0,
                                   MB_YESNO | MB_APPLMODAL |
                                   MB_MOVEABLE | 
                                   MB_ICONQUESTION);
            if ( usResponse == MBID_YES)
            {
                /*
                ** it is too dangerous for threading here so...just pretend it..
                */
                blockKeyboard();
                while (WinPeekMsg(hab,&qmsg,(HWND)NULL,(LONG)NULL,(LONG)NULL,PM_REMOVE))
                    WinDispatchMsg(hab, &qmsg);

                picList->delSelectedFromDisk(m_hwndClient);
                sizeSlideSheet();
                blockKeyboard(FALSE);
            }
        }
    }
    return (MRESULT)0;    
}
/*-----------------------------------------------[ public ]------------------*/
MRESULT sheet::rotateSelected(int iRot)                    //app calls this from menu 
{
   TID         ThreadID;       /* New thread ID (returned)                   */
   ULONG       ThreadFlags;    /* When to start thread,how to allocate stack */
   ULONG       StackSize;      /* Size in bytes of new thread's stack        */
   USHORT      cItems,usIndex;
   dropcopyblock *pBlock;

    if (picList && getOpMode() != OP_DETAILVIEW)
    {
        switch (iRot)
        {
        case ROTATE_90:
        case ROTATE_270:
            {
            pBlock = ( dropcopyblock * )calloc(sizeof(dropcopyblock),1);
            initThreadBlock(&pBlock->loader);
            pBlock->iRot = iRot;
            ThreadFlags = 0;        /* Indicate that the thread is to */
                                 /* be started immediately         */
            StackSize = 14096;      /* Set the size for the new       */
                                 /* thread's stack                 */
            DosCreateThread(&ThreadID,(PFNTHREAD)rotateSelectedThread,
                         (ULONG)pBlock,ThreadFlags,StackSize);

            break;
            }
        }
    }
    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::playSlides ( void )                  //app calls this from menu 
{
    ULONG    ulItems;

    if (getOpMode() == OP_DETAILVIEW || !picList)
        return (MRESULT)0;

    ulItems = picList->countSelected();

    if (!ulItems)
        return (MRESULT)0;

    changeOpMode(OP_SLIDESHOW);

    m_hwndSlide = createSlideWindow(this,m_hwndApplic);

    return (MRESULT)0;

}
/*---------------------------------------------------------------------------*/
MRESULT sheet::handleMouseBtn1dblclk(POINTL ptlMouse,MPARAM mp2)
{
    if (getOpMode() == OP_DETAILVIEW)
        return (MRESULT)0;

    if (picList)
    {
       if ((m_pvCurrent = picList->selectPicture(m_hwndSheet,NULL,&ptlMouse,FALSE))!=NULL)
       {
           changeOpMode(OP_DETAILVIEW);
           loadDetailForImage(m_hwndSheet,(HAB)0,m_pvCurrent);
       }
    }
    return 0;    
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::handleMouseButton1down(POINTL ptlMouse,MPARAM mp2)
{
    
    BOOL bShift      = (BOOL)((SHORT2FROMMP(mp2) & KC_SHIFT));
    BOOL bControl    = (BOOL)((SHORT2FROMMP(mp2) & KC_CTRL));    
    hPicture *pPict  = NULL;

    if (bShift && bControl )
        return (MRESULT)0;

    WinSetFocus(HWND_DESKTOP,m_hwndSheet);

    if (picList && getOpMode() == OP_SLIDES)
    {
        if (bControl)
        {
            m_pvCurrent = picList->selectSinglePicture(m_hwndSheet,m_pvCurrent,&ptlMouse);
        }
        else
        {
           pPict = picList->selectPicture(m_hwndSheet,m_pvCurrent,&ptlMouse,bShift);
           if (!bShift)
           {
               setSingleSelection(pPict);
               if (!pPict)
                   unSelectAll();
           }
        }
    }
    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
void sheet::scrollSelectionInView( BOOL bAbsolute )
{
    RECTL rcl,rclView;
    ULONG ulScroll;
    SHORT sVscrollInc;
   //printf("scrollSelectionInView \n");
    WinQueryWindowRect(view.hwndClient,&rclView);
    m_pvCurrent->getImageRect(&rcl);
   //printf("scrollSelectionInView image rcl.yBottom = %d \n",rcl.yBottom);
    WinMapWindowPoints(m_hwndSheet,view.hwndClient,(POINTL *)&rcl,2);
   //printf("scrollSelectionInView mapped cl.yBottom = %d \n",rcl.yBottom);

    if (!bAbsolute)                
    {
     /* 
     ** Just a selection by keyboard, going from one selection to the other 
     */
    if ( rcl.yBottom < 0)
    {
        viewPortscrollVert(0, MPFROM2SHORT(0,SB_LINEDOWN));
    }
    else if ( rcl.yTop > rclView.yTop)
    {
        viewPortscrollVert(0, MPFROM2SHORT(0,SB_LINEUP));
    }
    else if ( rcl.xRight > rclView.xRight)
    {
        viewPortscrollHorz(0, MPFROM2SHORT(0,SB_LINERIGHT));
    }
    else if ( rcl.xRight < rclView.xRight)
    {
        viewPortscrollHorz(0, MPFROM2SHORT(0,SB_LINELEFT));
    }
    }
    else
    {
       /*
       ** Absolute selection....The selected item must be scrolled
       ** into view. Happens after detail view and coming back to
       ** the picture sheet mode. And since at this point both scrollbars
       ** has been reset to zero we only can scroll down or to the left.
       ** To the left is.... left out.
       */
       if ( rcl.yBottom < 0)
       {
           /* SB_LINEDOWN */
           sVscrollInc = abs(rcl.yBottom);

           sVscrollInc = max(-view.sVscrollPos,
                     min(sVscrollInc,view.sVscrollMax - view.sVscrollPos));
    
           view.sVscrollPos += sVscrollInc;
           WinScrollWindow (m_hwndClient,0,sVscrollInc,(PRECTL)NULL,
                           (PRECTL)NULL,(HRGN)NULLHANDLE,
                           (PRECTL)NULL,SW_INVALIDATERGN | SW_SCROLLCHILDREN );

           WinSendMsg (view.hwndVscroll, SBM_SETPOS,
                       MPFROMSHORT (view.sVscrollPos), NULL);

           positionWindowInViewPort();
           WinUpdateWindow (m_hwndClient);
       }
    }
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::handleKeyBoard(MPARAM mp1,MPARAM mp2)
{
    char c;
    BOOL   fProcessed;  /* Flag is key is pocessed.             */
    ULONG ulItems;
    char szWarning[50];
    char szExpl[150];
    char szDirectory[CCHMAXPATH];
    USHORT usResponse;
    QMSG   qmsg;
    hPicture *pC = m_pvCurrent;

    fProcessed = FALSE;


    if (isKeyboardBlocked())
        return 0;

    if (SHORT1FROMMP(mp1) & KC_KEYUP )
    {
        m_bControl = FALSE;
        m_bShift   = FALSE;
        return 0;
    }

    if (SHORT1FROMMP(mp1) & KC_INVALIDCHAR )
    {
        m_bControl = FALSE;
        m_bShift   = FALSE;
        return 0;
    }

    m_bControl = (BOOL)((SHORT1FROMMP(mp1) & KC_CTRL));
    m_bShift   = (BOOL)((SHORT1FROMMP(mp1) & KC_SHIFT));

    if (SHORT1FROMMP(mp1) & KC_VIRTUALKEY)
    {
        switch (SHORT2FROMMP(mp2))
        {
        case VK_PAGEUP:
        case VK_PAGEDOWN:
            fProcessed = TRUE;
            if (getOpMode() != OP_SLIDESHOW)
            {
                return WinSendMsg(view.hwndVscroll,WM_CHAR,mp1,mp2);
            }
            return 0;
        case VK_UP:
            fProcessed = TRUE;
            if (getOpMode() == OP_DETAILVIEW)
                return WinSendMsg(view.hwndVscroll,WM_CHAR,mp1,mp2);
            else if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectPrevRow(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_DOWN:
            fProcessed = TRUE;
            if (getOpMode() == OP_DETAILVIEW)
                return WinSendMsg(view.hwndVscroll,WM_CHAR,mp1,mp2);
            if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectNextRow(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_LEFT:
            fProcessed = TRUE;
            if (getOpMode() == OP_DETAILVIEW)
                return WinSendMsg(view.hwndHscroll,WM_CHAR,mp1,mp2);
            if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectPrevPicture(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_RIGHT:
            fProcessed = TRUE;
            if (getOpMode() == OP_DETAILVIEW)
                return WinSendMsg(view.hwndHscroll,WM_CHAR,mp1,mp2);
            if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectNextPicture(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_END:
            fProcessed = TRUE;
            if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectLastPicture(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_HOME:
            fProcessed = TRUE;
            if (picList && getOpMode() == OP_SLIDES)
                m_pvCurrent = picList->selectFirstPicture(m_pvCurrent,m_hwndSheet,m_bShift);
            break;
        case VK_ESC:
            fProcessed = TRUE;
            if (picList)
            {
                if (getOpMode() == OP_SLIDES)
                    picList->selectAllPicture(m_hwndSheet,FALSE,TRUE);
                else if (getOpMode() == OP_SLIDESHOW)
                    destroySlideShowWindow();
            }
            changeOpMode(OP_SLIDES);
            break;
        case VK_F5:
            fProcessed = TRUE;
            if (m_szFolder[0] && getOpMode() == OP_SLIDES)
            {
                strcpy(szDirectory,m_szFolder);
                m_szFolder[0] = 0;
                loadPicturesFromDir(szDirectory);
            }
            break;

        case VK_DELETE:
            fProcessed = TRUE;
            deleteSelected();
            break;
        case VK_ENTER:
        case VK_NEWLINE:
            fProcessed = TRUE;
            if (getOpMode() == OP_DETAILVIEW)
                return (MRESULT)0;
            if (picList && m_pvCurrent)
            {
                changeOpMode(OP_DETAILVIEW);
                loadDetailForImage(m_hwndSheet,(HAB)0,m_pvCurrent);            
            }
			return (MRESULT)0;
        }
    }

   /*------------------------
     Process character keys
   ------------------------*/

    if (!fProcessed && SHORT1FROMMP(mp1) & KC_CHAR)
    {
        if (getOpMode() == OP_SLIDESHOW)
        {
           if ( 'p' == (CHAR)SHORT1FROMMP(mp2) || 
                'P' == (CHAR)SHORT1FROMMP(mp2) )
           {
              m_bPaused = TRUE;
           }
           else
           {
              m_bPaused = FALSE;
           }
        }
     }

    
    if (pC != m_pvCurrent)
        scrollSelectionInView(FALSE); /* Just one row/column */

    if (fProcessed)
        return (MRESULT)1;

    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
void sheet::createViewPort(HWND hwnd)
{
   view.hwndVscroll = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                                                     FID_VERTSCROLL);
   view.hwndHscroll = WinWindowFromID(WinQueryWindow(hwnd, QW_PARENT),
                                                     FID_HORZSCROLL);
   view.hwndClient = hwnd;
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::viewPortscrollVert(MPARAM mp1, MPARAM mp2)
{
    SHORT  sVscrollInc;               /* Vertical scrollbar incr val    */

    if (!m_Rows)
      m_Rows = MIN_ROWS;

    switch (SHORT2FROMMP (mp2))
    {
    case SB_LINEUP:
       sVscrollInc = -view.cySheet/m_Rows;
       break;
    case SB_LINEDOWN:
       sVscrollInc =  view.cySheet/m_Rows;
       break;
    case SB_PAGEUP:
       sVscrollInc = -view.cySheet;
       break;
    case SB_PAGEDOWN:
       sVscrollInc = view.cySheet;
       break;
    case SB_SLIDERPOSITION:
       sVscrollInc = SHORT1FROMMP (mp2) - view.sVscrollPos;
       break;
    default:
       sVscrollInc = 0;
       break;
    }
    sVscrollInc = max(-view.sVscrollPos,
                   min(sVscrollInc,view.sVscrollMax - view.sVscrollPos));
    if ( sVscrollInc != 0 )
    {
        view.sVscrollPos += sVscrollInc;
        WinScrollWindow (m_hwndClient,0,sVscrollInc,(PRECTL)NULL,
                         (PRECTL)NULL,(HRGN)NULLHANDLE,
                         (PRECTL)NULL,SW_INVALIDATERGN | SW_SCROLLCHILDREN );

        WinSendMsg (view.hwndVscroll, SBM_SETPOS,
                     MPFROMSHORT (view.sVscrollPos), NULL);

        positionWindowInViewPort();
        WinUpdateWindow (m_hwndClient);
    }
    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::viewPortscrollHorz(MPARAM mp1, MPARAM mp2)
{
    SHORT  sHscrollInc;               /* Horizontal scrollbar incr val  */

    switch(SHORT2FROMMP (mp2))
    {
    case SB_LINELEFT:
       sHscrollInc = -view.cxSheet/m_Columns;
       break;
    case SB_LINERIGHT:
       sHscrollInc = view.cxSheet/m_Columns;
       break;
    case SB_PAGELEFT:
       sHscrollInc = -view.cxSheet;
       break;
    case SB_PAGERIGHT:
       sHscrollInc = view.cxSheet;
       break;
    case SB_SLIDERPOSITION:
       sHscrollInc = SHORT1FROMMP (mp2) - view.sHscrollPos;
       break;
    default:
       sHscrollInc = 0;
       break;
    }
    sHscrollInc = max ( -view.sHscrollPos,
               min (sHscrollInc, view.sHscrollMax - view.sHscrollPos));

    if ( sHscrollInc !=0)
    {
        view.sHscrollPos += sHscrollInc;
        WinScrollWindow(m_hwndClient,-sHscrollInc,0L,
                        (PRECTL)NULL,(PRECTL)NULL,
                        (HRGN)NULLHANDLE,(PRECTL)NULL,
                        SW_INVALIDATERGN | SW_SCROLLCHILDREN);

        WinSendMsg (view.hwndHscroll, SBM_SETPOS,
                    MPFROMSHORT (view.sHscrollPos), NULL);
        WinUpdateWindow (m_hwndClient);
        positionWindowInViewPort();
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
MRESULT sheet::createPictureSheet(int slideSize)
{
    switch ( m_opMode)
    {
    case OP_DETAILVIEW:
        if (m_pvCurrent)
        {
            m_pvCurrent->freeDetailImage();
        }
        break;
    }
    m_opMode = OP_SLIDES;

	switch (slideSize)
	{
		case SLIDE_LARGE:
		case SLIDE_MEDIUM:
		case SLIDE_SMALL:
			m_slideSize = slideSize;
			break;
		default:
			m_slideSize = SLIDE_SMALL;
			break;
	}

   m_Rows      = VAL_SMALL;   /* A value is needed for the scrollbars */
   m_Columns   = VAL_COLUMNS; /* A value is needed for the scrollbars */
 
   if (!m_hwndSheet && m_hwndClient)
       m_hwndSheet = createSheetWindow(this, m_hwndClient);

    view.sVscrollPos = 0;
    view.sHscrollPos = 0;

    sizeSlideSheet();
//    WinInvalidateRect(m_hwndSheet,NULL,TRUE);
    return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
MRESULT EXPENTRY viewPortWndProc(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
   RECTL rcl;
   HPS   hps;
   LONG  cxClient,cyClient;
   sheet *pSheet;
  
   if (pShtCreate)
      pSheet = pShtCreate;
   else
      pSheet = (sheet *)WinQueryWindowPtr(hwnd,0);


   switch(msg)
   {
      case WM_CREATE:
         pSheet->createViewPort(hwnd);
         pSheet->createPictureSheet(SLIDE_MEDIUM);
         WinSetWindowPtr(hwnd,0,pShtCreate);
         return 0;
 
      case WM_SIZE:
         cxClient = (LONG)SHORT1FROMMP(mp2);
         cyClient = (LONG)SHORT2FROMMP(mp2);
         pSheet->m_cxClient = ( int )cxClient;
         pSheet->m_cyClient = ( int )cyClient;
         pSheet->sizeSlideSheet();         
         return (MRESULT)0;
      case WM_CHAR:
         return pSheet->handleKeyBoard(mp1,mp2);
      case WM_PAINT:
         WinQueryWindowRect(hwnd,&rcl);
         hps = WinBeginPaint (hwnd,(HPS)0,&rcl);
         WinFillRect(hps,&rcl,CLR_PALEGRAY);
         WinEndPaint(hps);
         return 0;
/*========================Vertical=Scrolling================================*/
      case WM_VSCROLL:      return pSheet->viewPortscrollVert(mp1,mp2);
/*========================Horizontal=Scrolling==============================*/
      case WM_HSCROLL:      return pSheet->viewPortscrollHorz(mp1,mp2);
   }
   return (WinDefWindowProc(hwnd,msg,mp1,mp2));
}
/*---------------------------------------------------------------------------*/
HWND sheet::getFrameWindow( void )
{
   return hwndFrame;
}
/*---------------------------------------------------------------------------*/
sheet::~sheet()
{
   if (picList)
      delete picList;
   picList = NULL;

   if (m_hpsMem)
      GpiDestroyPS(m_hpsMem);
   if (m_hps)
       GpiDestroyPS(m_hps);
}
/*---------------------------------------------------------------------------*/
sheet::sheet( HAB hab, HWND hParent)
{
    static CHAR szClientClass[]    = "ClientWindow";
    HDC      hdcBitmapFile;
    static CHAR szCircleClass[]    = "PROGRESSCLASS";


    static ULONG FrameFlags = FCF_BORDER | FCF_HORZSCROLL | FCF_VERTSCROLL ;


   memset(&view,0,sizeof(CANVAS));

   WinRegisterClass( hab,			                // Another block handle
                    (PSZ)szClientClass,             // Name of class being registered
                    (PFNWP)viewPortWndProc,	        // Window procedure for class
                    CS_CLIPCHILDREN | CS_SIZEREDRAW,// Class style
                    sizeof( void *));			    // Extra bytes to reserve


    WinRegisterClass( hab,                     // Another block handle
                     (PSZ)szCircleClass,       // Name of class being registered
                     (PFNWP)progressBarWndProc,// Window procedure for class
                     CS_SIZEREDRAW,            // Class style
                     (BOOL)0);                 // Extra bytes to reserve

   pShtCreate = this;
   picList    = NULL;
   hwndFrame = WinCreateStdWindow (hParent,
                                   WS_VISIBLE,
                                   &FrameFlags,
                                   (PSZ)szClientClass,
                                   (PSZ)"PictureSheet",
                                   0,
                                   (HMODULE)0L,
                                   0,
                                   &m_hwndClient);
   m_Columns = VAL_COLUMNS;
   m_Rows    = VAL_ROWS;
   m_slideSize = SLIDE_SMALL;
   m_opMode    = OP_SLIDES;
   m_bShowName = TRUE;
   m_lCharHeight = 0;
   m_hwndSheet = NULL;
   m_hProgress = NULL;
   m_cySheet = 0;
   m_cxSheet = 0;   
   m_szFolder[0]=0;
   m_keyboardblock = FALSE;
   m_hwndSlide = NULL;
   m_hwndApplic= hParent;
   m_pDragitems = NULL;
   m_bPaused = FALSE;
   CreateBitmapHdcHps( &hdcBitmapFile,&m_hpsMem);
}
