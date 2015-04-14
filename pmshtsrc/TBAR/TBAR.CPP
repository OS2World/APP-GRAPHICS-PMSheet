/*---------------------------------------------------------------------------*/
/*  Name: tbar.cpp                                                           */
/*                                                                           */
/*  Description : Toolbar                                                    */
/*                                                                           */
/*                                                                           */
/*-sc---date-----------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#define INCL_WINDIALOGS
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "..\resource.h"  /* resource id's            */
#include "tbutton.h"
#include "tbar.hpp"
#include "tip.h"

#define ID_TBAR 1024

static char pszButtonClass[] = "ImageBtn";
static char psztBarClass[]   = "toolbar";

static tbar *tbarCreate;
/*
**---------------------------- Main toolbar-------------------------------
*/
struct                      /* ibtn */
{                           /* Size:  16        Bytes                     */
   LONG        cb;          /* Structure        Size                      */
   LONG        idBitmap;    /* Bitmap ID        : Normal or Up Position   */
   LONG        idBitmapDown;/* Bitmap ID        : Down Position           */
   LONG        idBitmapDisabled; /* Bitmap ID        : Disabled           */
   CHAR        szTipText[MAX_TIPTEXT];/* TOOLltip text      */
   LONG        idButton;    /* WINDOW ID used in the WM_COMMAND. Button does not used this...only used here  */
   HWND        hButton;     /* WINDOW handle of the Button.            "" only here   */
} imgbtn[]=
{
{16,IDB_PROJECT,   IDB_PROJECT, IDB_PROJECT,"", IDBTN_PROJECT, NULLHANDLE}, /* SLIDE SHOW*/
{16,IDB_ROTRIGHT,   IDB_ROTRIGHT,  IDB_ROTRIGHT, "",IDBTN_ROTRIGHT,  NULLHANDLE}, /* Rotate left*/
{16,IDB_ROTLEFT,   IDB_ROTLEFT,  IDB_ROTLEFT, "",IDBTN_ROTLEFT,  NULLHANDLE}, /* Rotate left*/
{16,IDB_SORTSIZE,  IDB_SORTSIZE,IDB_SORTSIZE,"",IDBTN_SORTSIZE,  NULLHANDLE}, /* Sort on size */
{16,IDB_SORTNAME,  IDB_SORTNAME,IDB_SORTNAME,"",IDBTN_SORTNAME,  NULLHANDLE}, /* SortName*/
{16,IDB_ZOOM,      IDB_ZOOM,    IDB_ZOOM,   "", IDBTN_ZOOM,    NULLHANDLE}, /* ZOOM      */
{16,IDB_DELETE,    IDB_DELETE,  IDB_DELETE, "", IDBTN_DELETE,  NULLHANDLE}, /* DELETE    */
{16,IDB_SMALL ,    IDB_SMALL ,  IDB_SMALL , "", IDBTN_SMALL ,  NULLHANDLE}, /* small     */
{16,IDB_MEDIUM,    IDB_MEDIUM,  IDB_MEDIUM, "", IDBTN_MEDIUM,  NULLHANDLE}, /* medium    */
{16,IDB_LARGE ,    IDB_LARGE ,  IDB_LARGE , "", IDBTN_LARGE ,  NULLHANDLE}, /* large     */
{0,0,0,0,0,NULLHANDLE}};

/*---------------------------------------------------------------------------*/
MRESULT EXPENTRY tbarWndProc(HWND hwnd,ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
   static long cx,cy;
   HPS hps;
   RECTL rcl;
   HBITMAP hbm;
   POINTL ptl[2];
   tbar *pTbar;

   if (tbarCreate)
      pTbar = tbarCreate;
   else
      pTbar = (tbar *)WinQueryWindowPtr(hwnd,0);
   

    switch (ulMsg)
    {
      case WM_CREATE:
         WinSetWindowPtr(hwnd,0,tbarCreate);
         tbarCreate = NULL;
         return 0;

    case WM_SIZE:
        pTbar->m_cx = (LONG)SHORT1FROMMP(mp2);
        pTbar->m_cy = (LONG)SHORT2FROMMP(mp2);
        return 0;

       case WM_PAINT:
          hps = WinBeginPaint (hwnd,(HPS)0,&rcl);
//          GpiCreateLogColorTable (hps, LCOL_RESET,LCOLF_RGB,0,0,NULL);
//          WinFillRect(hps,&rcl,0x009CD6FF);
          ptl[0].x = 0;
          ptl[0].y = 0;
          ptl[1].x = pTbar->m_cx;
          ptl[1].y = pTbar->m_cy; 
          if (!pTbar->m_hbm)
             tbarCreate->m_hbm = GpiLoadBitmap(hps,NULL,IDB_TBAR,34,34);
          WinDrawBitmap(hps,pTbar->m_hbm,NULL,ptl,CLR_NEUTRAL,CLR_BACKGROUND,DBM_STRETCH);
          WinEndPaint(hps);
          return (MRESULT)0;
   }
   return (MRESULT)WinDefWindowProc(hwnd,ulMsg,mp1,mp2);
}
/*---------------------------------------------------------------------------*/
/* createToolBar.                                                            */
/*---------------------------------------------------------------------------*/
tbar::tbar(HAB hab , HWND hOwner, HWND hParent )
{
   int i;
   HPS hps;
   WinRegisterClass(hab,(PSZ)pszButtonClass,(PFNWP)ImageBtnWndProc,
                    CS_PARENTCLIP | CS_SYNCPAINT  | CS_SIZEREDRAW,
                    (ULONG)USER_RESERVED);

   WinRegisterClass(hab,(PSZ)psztBarClass,(PFNWP)tbarWndProc,
                    CS_PARENTCLIP | CS_SYNCPAINT  | CS_SIZEREDRAW,
                    (ULONG)USER_RESERVED);

   registerTooltip(hab);
   /*
   ** Create the main toolbar
   */
   m_hTbar =  WinCreateWindow(hParent,psztBarClass, "",
                            WS_VISIBLE,
                            0,0,
                            0,0,
                            hOwner,               /* owner window      */
                            HWND_BOTTOM,
                            ID_TBAR,
                            (void *)0,            /* user data         */
                            NULL);                /* press params      */

   if (!m_hTbar)
      return;

   tbarCreate = this;

   for (i = 0; i < MAXBUTTONS; i++)
   {
       if (imgbtn[i].idBitmap)
       {
          WinLoadString((HAB)0,(HMODULE)0,imgbtn[i].idBitmap,MAX_TIPTEXT,(PSZ)imgbtn[i].szTipText);

          imgbtn[i].hButton = WinCreateWindow(m_hTbar,pszButtonClass,"",
                                              WS_VISIBLE,
                                              0,0,
                                              0,0,
                                              hOwner,
                                              HWND_BOTTOM,
                                              imgbtn[i].idButton,
                                              (void *)&imgbtn[i],
                                              NULL); /* press params */
       }
   }
   return;
}
/*---------------------------------------------------------------------------*/
/* destroyToolBar                                                            */
/*---------------------------------------------------------------------------*/
tbar::~tbar()
{
   int i;


   for (i = 0; imgbtn[i].idBitmap; i++)
   {
      if (imgbtn[i].hButton)
         WinDestroyWindow(imgbtn[i].hButton);
   }
   WinDestroyWindow(m_hTbar);
}
/*---------------------------------------------------------------------------*/
/* posToolBar                                                                */
/*---------------------------------------------------------------------------*/
void tbar::sizepos(long x, long y, long cx, long cy)
{
    int i; 
    int xPos;
    long cyButton;
    if (!m_hTbar)
        return;
   
   WinSetWindowPos(m_hTbar,HWND_TOP,x,y,cx,cy,SWP_SIZE | SWP_MOVE | SWP_ZORDER);

   m_x         = x;
   m_y         = y;

   xPos        = TBAR_SPACE;
   cyButton = cy - 4;

   for (i = 0; i < MAXBUTTONS; i++)
   {
      if (imgbtn[i].cb)
      {
          if (imgbtn[i].idButton == IDBTN_DELETE || imgbtn[i].idButton == IDBTN_SMALL)
                xPos += (CXBUTTON/2);

          WinSetWindowPos(imgbtn[i].hButton,
                          HWND_TOP,
                          xPos,
                          2,
                          CXBUTTON,
                          cyButton,
                          SWP_SIZE | SWP_MOVE | SWP_ZORDER);
      }
      xPos += (CXBUTTON + TBAR_SPACE);
   }
}
