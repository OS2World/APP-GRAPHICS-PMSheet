#define INCL_WIN
#define INCL_GPI
#define INCL_DOSDATETIME
#define INCL_SHLERRORS
#define INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "statline.hpp"
#include "shtdir.hpp"
#include "shthlp.h"
#include "sheet.hpp"
#include "dirctrl.hpp"
#include "tbar.hpp"
#include "resource.h"
#include "appmain.hpp"


#define STATUS_HEIGHT 25
#define TOOLBAR_HEIGHT 45
#define SPLITBAR_WIDTH 4

#define DB_RAISED    0x0400	    /* the undocumented value's */
#define DB_DEPRESSED 0x0800     /* drawbox.                 */

static appmain *pAppCreate;
HAB   mainhab;
static BOOL fHelpEnabled;

MRESULT EXPENTRY appMainWndProc(HWND hwnd,ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
    appmain *pAppMain;

    if (pAppCreate)
      pAppMain = pAppCreate;
    else
      pAppMain = (appmain *)WinQueryWindowPtr(hwnd,0);

    if (pAppMain->wndProc(ulMsg,mp1,mp2))
        return 0;

   return (MRESULT)WinDefWindowProc(hwnd,ulMsg,mp1,mp2);
}

/*---------------------------------------------------------------------------*/
MRESULT appmain::handleCommand(MPARAM mp1,MPARAM mp2)
{
   switch(LOUSHORT(mp1))
   {
//   case IDM_NEW:
//      return m_pSheet->loadPicturesFromDir("x:\\sheet\\");
//   case IDM_CDROM:
//      return m_pSheet->loadPicturesFromDir("x:\\sheet\\jpg\\");
   case IDM_EXIT:
       WinPostMsg(m_hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
       break;
   case IDBTN_LARGE:
   case IDM_LARGE:
      return m_pSheet->createPictureSheet(SLIDE_LARGE);
   case IDBTN_MEDIUM:
   case IDM_MEDIUM:
      return m_pSheet->createPictureSheet(SLIDE_MEDIUM);
   case IDBTN_SMALL:
   case IDM_SMALL:
      return m_pSheet->createPictureSheet(SLIDE_SMALL);
   case IDM_DETAIL:
   case IDBTN_ZOOM:
      return m_pSheet->showDetailOnSelection();
   case IDM_SLIDESHOW:
   case IDBTN_PROJECT:
      return m_pSheet->playSlides();
   case IDM_SORTSIZE:
   case IDBTN_SORTSIZE:
       return m_pSheet->sortOnSize();
   case IDM_SORTNAME:
   case IDBTN_SORTNAME:
       return m_pSheet->sortOnName();
   case IDM_SEL:
       return m_pSheet->selectAll();
   case IDM_NONE:
       return m_pSheet->unSelectAll();
   case IDBTN_DELETE:
       return m_pSheet->deleteSelected();
   case IDBTN_ROTRIGHT:
       return m_pSheet->rotateSelected(ROTATE_270);
   case IDBTN_ROTLEFT:
       return m_pSheet->rotateSelected(ROTATE_90);
   case IDM_RELOAD:
       return m_pSheet->forceReloadPictures();
   case  IDM_HELP:
       DisplayHelpPanel(PANEL_HELP);
       return (MRESULT)0;
   case  IDM_HELPINDEX:
        HelpIndex((MPARAM)0);
        return (MRESULT)0;
   case IDM_ABOUT:
       WinMessageBox(HWND_DESKTOP,m_hwnd,
                       (PSZ)"PMSheet version 3.0 \nJasper de Keijzer & Co 2004",(PSZ)"About", 0,
                        MB_OK | MB_APPLMODAL | MB_MOVEABLE | 
                        MB_INFORMATION);

   }
   return (MRESULT)0;
}
/*---------------------------------------------------------------------------*/
MRESULT appmain::layoutControls( void )
{
    LONG   x,y,cx,cy;
    MRESULT mr = (MRESULT)0;
    HWND   hwndControl = WinWindowFromID (m_hwnd,CNR_ID);
        

    if (!m_cx || !m_cy)
        return mr;

    if (m_pTree && hwndControl )
    {
        y = STATUS_HEIGHT;
        x = 0;
        cx= m_cx / 3;
        cy= m_cy;
        cy -= (STATUS_HEIGHT + TOOLBAR_HEIGHT);

        cx -= SPLITBAR_WIDTH;

        if (m_rTree.xRight == 0)
        {
            m_rTree.xLeft   = x ;
            m_rTree.xRight  = x + cx;
        }
        else
        {
            m_rTree.xLeft  = 0;
            cx = m_rTree.xRight - m_rTree.xLeft;
        }
        m_rTree.yTop    = y + cy;
        m_rTree.yBottom = y;

        WinSetWindowPos (hwndControl,HWND_TOP,0,y,cx,cy,SWP_MOVE | SWP_SIZE);
    }

    if (m_pSheet && m_pTree ) 
    {
        
        cy= m_cy;
        cy -= (STATUS_HEIGHT + TOOLBAR_HEIGHT);
        
        x = m_rTree.xRight + (SPLITBAR_WIDTH*2);
        cx= m_cx - x;

        m_rSheet.xLeft   = x;
        m_rSheet.xRight  = x + cx;
        m_rSheet.yTop    = y + cy;
        m_rSheet.yBottom = y;

        WinSetWindowPos(m_pSheet->getFrameWindow(),
                        HWND_TOP,
                        x,y,cx,cy,
                        SWP_MOVE | SWP_SIZE | SWP_ZORDER | SWP_SHOW);
        mr = (MRESULT)1;
    }

    if (m_pStatusLine)
    {
        m_pStatusLine->setSize(m_cx,STATUS_HEIGHT);
    }

    if (m_pTool)
    {
        y = m_cy - TOOLBAR_HEIGHT;
        m_pTool->sizepos(0, y, m_cx,TOOLBAR_HEIGHT);
    }

    m_rSplit.xLeft  = m_rTree.xRight;
    m_rSplit.xRight = m_rSheet.xLeft;
    m_rSplit.yBottom= m_rSheet.yBottom;
    m_rSplit.yTop   = m_rSheet.yTop;
    return mr;
}
/*---------------------------------------------------------------------------*/
void appmain::initTrackInfo(TRACKINFO *pTrack)
{
   pTrack->cxGrid = 1;
   pTrack->cyGrid = 1;    /* smooth tracking with mouse */
   pTrack->cxKeyboard = 8;
   pTrack->cyKeyboard = 8; /* faster tracking using cursor keys */

   pTrack->ptlMinTrackSize.x = 10;
   pTrack->ptlMinTrackSize.y = 10;   /* set smallest allowed size of rectangle */
   pTrack->ptlMaxTrackSize.x = m_cx;
   pTrack->ptlMaxTrackSize.y = m_cy; /* set largest allowed size of rectangle */
 
   pTrack->fs = TF_LEFT | TF_RIGHT;

   pTrack->cxBorder = pTrack->cyBorder = 1; //SPLITBAR_WIDTH;  /* Set track width                       */
   pTrack->rclTrack = m_rTree;                            /* Set current tracking bounds           */
   pTrack->rclBoundary.xLeft  = 0;
   pTrack->rclBoundary.xRight = m_cx;
   pTrack->rclBoundary.yBottom= 0;
   pTrack->rclBoundary.yTop   = m_cy;


}
/*---------------------------------------------------------------------------*/
MRESULT appmain::wndProc(ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
    HPS    hps;
    RECTL  rcl;
    LONG   x,y,cx,cy;
    POINTL ptlPosn;
    TRACKINFO Track;
    static char         szDir[255];

    switch (ulMsg)
    {
    case WM_CREATE:
        WinSetPresParam(m_hwnd,PP_FONTNAMESIZE,(ULONG)strlen("8.Helv")+1,(void *)"8.Helv");
        return (MRESULT)1;
    case UM_FOLDERSTRING:
         printf("appmain::wndProc----->UM_FOLDERSTRING\n");
         m_pStatusLine->setCellText( (int)mp2,( char *) mp1 );
         return 0;
    case UM_IMAGESTRING:
         printf("appmain::wndProc----->UM_IMAGESTRING\n");
         m_pStatusLine->setCellText( (int)1,( char *) mp1 );
         return 0;
    case WM_CHAR:
        return m_pSheet->handleKeyBoard(mp1,mp2);
     case WM_HELP:
         if (fHelpEnabled)
            DisplayHelpPanel(PANEL_HELP);
        return (MRESULT)0;
    case WM_COMMAND:
        handleCommand(mp1,mp2);
        return (MRESULT)1;
    case WM_SIZE:
        m_cx = (LONG)SHORT1FROMMP(mp2);
        m_cy = (LONG)SHORT2FROMMP(mp2);
        return layoutControls();
    case WM_BUTTON1DOWN:
        initTrackInfo(&Track);
        if (WinTrackRect(m_hwnd, NULLHANDLE, &Track))
        {
            WinCopyRect(mainhab,&m_rTree,&Track.rclTrack);
            layoutControls();
        }
        return (MRESULT)0;
    case WM_MOUSEMOVE:
        ptlPosn.x   = SHORT1FROMMP(mp1); /* @check */
        ptlPosn.y   = SHORT2FROMMP(mp1);
        if ( WinPtInRect((HAB)mainhab,
                         &m_rSplit,
                         &ptlPosn))
        {
            /*
            ** We are on the split and not on the statusline
            */
            WinSetPointer(HWND_DESKTOP,
                          WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE));
            m_bHit = TRUE;
        }
        else
        {
            if (m_bHit)
                WinSetPointer(HWND_DESKTOP,WinQuerySysPointer(HWND_DESKTOP,SPTR_ARROW,FALSE));            
            m_bHit = FALSE;
        }
        return (MRESULT)TRUE;
    case WM_PAINT:
        hps = WinBeginPaint (m_hwnd,(HPS)0,&rcl);
        //GpiErase(hps);
//        ulColor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0L));
        WinFillRect(hps,&m_rSplit,CLR_PALEGRAY);
        WinDrawBorder(hps, &m_rSplit,2L,2L,0L, 0L, DB_RAISED);
        if (m_pStatusLine)
            m_pStatusLine->paintCells (hps);
        WinEndPaint(hps);
        return (MRESULT)1;
    case UM_LETMEKNOW:
        strcpy(szDir,(char *)mp1);
        return m_pSheet->loadPicturesFromDir(szDir);
    }
    return (MRESULT)0;
}

appmain::appmain( )
{
   HMQ   hmq;
   QMSG  qmsg;
   int iPercent[2];

   static CHAR szApplClass[]    = "AppMainWindow";
   static ULONG FrameFlags = FCF_TITLEBAR     | FCF_SYSMENU       |
                             FCF_SIZEBORDER   | FCF_SHELLPOSITION | 
                             FCF_MINMAX       | FCF_MENU          | 
                             FCF_TASKLIST     | FCF_ACCELTABLE;

   mainhab = WinInitialize (0);
   hmq = WinCreateMsgQueue(mainhab,0);
   m_cx=0;
   m_cy=0;

   memset(&m_rSheet,0,sizeof(RECTL));
   memset(&m_rTree ,0,sizeof(RECTL));

   WinRegisterClass(mainhab,			                // Another block handle
                    (PSZ)szApplClass,               // Name of class being registered
                    (PFNWP)appMainWndProc,	        // Window procedure for class
                    CS_CLIPCHILDREN | CS_SIZEREDRAW,// Class style
                    sizeof( void *));			    // Extra bytes to reserve

   pAppCreate = this;
   m_pSheet   = NULL;

   m_hwndFrame = WinCreateStdWindow (HWND_DESKTOP,
                                     WS_VISIBLE,
                                     &FrameFlags,
                                     (PSZ)szApplClass,
                                     (PSZ)"PMSheet",
                                      0,
                                     (HMODULE)0L,
                                     ID_RESOURCE,
                                     &m_hwnd);


   m_pTree  = new dirCtrl(mainhab,m_hwnd,CNR_ID, CV_MINI,UM_LETMEKNOW);

   m_pTool  = new tbar(mainhab , m_hwnd, m_hwnd );

   m_pSheet = new sheet(mainhab, m_hwnd);
   iPercent[0] = 50;
   iPercent[1] = 50;
   m_pStatusLine = new statline(m_hwnd,iPercent,2 ); // array of percentages ( sum == 100%) and nr of cells in icount.

   layoutControls();

   fHelpEnabled = InitHelp(mainhab, m_hwndFrame,m_hwnd);

   while (WinGetMsg (mainhab, &qmsg, 0L,0,0))
      WinDispatchMsg (mainhab,&qmsg);

   delete m_pSheet;
   delete m_pTree;
   delete m_pTool;
   delete m_pStatusLine;

   WinDestroyWindow (m_hwndFrame);
   WinDestroyMsgQueue ( hmq);
   WinTerminate (mainhab);
}



int main(void)
{
    appmain * myApplication;

    myApplication = new appmain();

    return 0;
}
