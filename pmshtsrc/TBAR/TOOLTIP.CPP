/*---------------------------------------------------------------------------*/
/*  Name: tooltip.cpp                                                        */
/*---------------------------------------------------------------------------*/
/* DISCLAIMER OF WARRANTIES:                  */
/* -------------------------                  */
/* The following [enclosed] code is sample code   created   by IBM      */
/* Corporation and Prominare Inc.  This   sample code is not part   of any   */
/* standard IBM   product   and is provided   to you solely for the purpose   */
/* of assisting   you in the development of your applications.  The code   */
/* is provided "AS IS",   without   warranty of any   kind.  Neither IBM nor   */
/* Prominare shall be liable for any damages arising out of your   */
/* use of the sample code, even   if they   have been advised of the   */
/* possibility of such damages.                  */
/*                                               */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>

extern "C"
{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
}

#define DB_RAISED    0x0400
#define DB_DEPRESSED 0x0800
#define UM_DESTROY   WM_USER

#include "tooltip.hpp" /* class definition        */
#include "tip.h"       /* extern c function def's */

#define IDT_TOOLTIP    25           // timer id


static char  szClass[] = "tltip";
static char  szFont[200]; /* font used in the tooltip. */
static long  cxScreen;
static long  cyScreen;
static HWND  hTip;        /* One tooltip in the whole app at the same time */
/*---------------------------------------------------------------------------*/
/*  toolTip::showToolTip.                                                    */
/*                                                                           */
/*  Description : Shows the tooltip 10 pixels below the hotspot of the       */
/*                mouse pointer.                                             */
/*                                                                           */
/*  Parameters  : none.                                                      */
/*                                                                           */
/*  Returns     : none.                                                      */
/*---------------------------------------------------------------------------*/
void toolTip::showToolTip(void)
{
    POINTL pt,ptOrg;
    RECTL  r;

    if (m_hTip)
    {
        /*
        ** A one shot timer!
        */
        WinStopTimer((HAB)0,m_hTip,IDT_TOOLTIP);
      

        WinQueryPointerPos(HWND_DESKTOP,&pt);
        ptOrg = pt;
        /* map from desktop to client window */
        WinMapWindowPoints(HWND_DESKTOP,m_hParent,&pt,1);

        WinQueryWindowRect(m_hParent,&r);
        if (!WinPtInRect((HAB)0,&r,&pt))
        {
           WinShowWindow(m_hTip,FALSE);
           return;
        }
        // -- center the tip 10 pixels below the mouse hotspot
        ptOrg.y -= ((LONG)WinQuerySysValue(HWND_DESKTOP,SV_CYPOINTER) + 5);

        if (ptOrg.y < 0)
        {
           ptOrg.y  = 0;
           ptOrg.x += 5;
        }

        ptOrg.x = max(0,ptOrg.x);
        ptOrg.y = max(0,ptOrg.y);

        if ((ptOrg.x + m_cx ) > cxScreen)
           ptOrg.x -= ((ptOrg.x + m_cx ) - cxScreen);

        if ((ptOrg.y + m_cy ) > cyScreen)
           ptOrg.y -= ((ptOrg.y + m_cy ) - cyScreen);

        WinSetWindowPos(m_hTip,HWND_TOP, ptOrg.x, ptOrg.y, 0, 0,
                        SWP_MOVE | SWP_SHOW | SWP_ACTIVATE | SWP_ZORDER);
    }
}
/*---------------------------------------------------------------------------*/
/*  toolTip::toolTip.                                                        */
/*                                                                           */
/*  Description : Constructor for the tooltip window. Creates the window     */
/*                with the size calculated by DrawText. Starts a timer which */
/*                notifies the parent window.                                */
/*                                                                           */
/*  Parameters  : HWND hParent - window handle of the parent window.         */
/*                char *pszText- pointer to a null terminating string which  */
/*                is shown when the tooltip is shown.                        */
/*                                                                           */
/*  Returns     : none.                                                      */
/*---------------------------------------------------------------------------*/
toolTip::toolTip(HWND hParent, char *pszText)
{
   HPS   hps;
   RECTL r;
   char  *p;
   PPOINTL aptl;
   POINTL  nptl[5];        /* QueryTextbox needs 5 points  */

   aptl = nptl;

   // -- remember our parent.
   m_hParent = hParent;
   m_hTip    = (HWND)0;
   m_pszText = NULL;
   m_cx = 0;
   m_cy = 0;

   // -- display tooltip if not empty
   if (pszText && *pszText)
   {
      m_pszText = new char [strlen(pszText)+1];

      memset(m_pszText,0,strlen(pszText)+1);

      //strcpy(m_pszText, pszText);
      p = m_pszText;
      do    
      {
         if (*pszText != '~')
         {
          *p = *pszText;
          p++;
         }
         pszText++;
      } while (*pszText);


      if (!szFont[0])
      {
         // -- calculate nice size.

         hps = WinGetPS(m_hParent);
         GpiQueryTextBox(hps,(LONG)strlen(m_pszText),m_pszText,TXTBOX_COUNT,aptl);
         WinReleasePS(hps);
         m_cx = nptl[TXTBOX_TOPRIGHT].x - nptl[TXTBOX_TOPLEFT].x;
         m_cy = nptl[TXTBOX_TOPRIGHT].y - nptl[TXTBOX_BOTTOMRIGHT].y;
         m_cx += 6;
         m_cy += 4;
      }
      m_hTip = WinCreateWindow(HWND_DESKTOP,szClass,
                               NULL,0,    /* invisible */
                               0,0,
                               m_cx,m_cy,
                               hParent,
                               HWND_TOP,
                               6000,
                               (void *)this,
                               NULL);

      WinSetWindowPtr(m_hTip, 0, (void *)this);

      if (szFont[0])
      {
         WinSetPresParam(m_hTip,PP_FONTNAMESIZE,
                         (ULONG)(strlen(szFont)+1),(PVOID)szFont);

         hps = WinGetPS(m_hTip);
         GpiQueryTextBox(hps,(LONG)strlen(m_pszText),m_pszText,TXTBOX_COUNT,aptl);
         WinReleasePS(hps);
         m_cx = nptl[TXTBOX_TOPRIGHT].x - nptl[TXTBOX_TOPLEFT].x;
         m_cy = nptl[TXTBOX_TOPRIGHT].y - nptl[TXTBOX_BOTTOMRIGHT].y;
         m_cx += 6;
         m_cy += 4;
         WinSetWindowPos(m_hTip,NULLHANDLE,0,0,m_cx,m_cy,SWP_SIZE);
      }
      if (m_hTip)
      {
         /*
         ** Signal when it is showtime
         */
         WinStartTimer((HAB)0,m_hTip,IDT_TOOLTIP, DLY_TOOLTIP);
         /*
         ** Capture the mouse so we can cancel when the mouse moves away
         */
         WinSetCapture(HWND_DESKTOP,m_hParent);
      }
   }
}
/*---------------------------------------------------------------------------*/
/*  toolTip::~toolTip.                                                       */
/*                                                                           */
/*  Description : Destructor.                                                */
/*                The final destroy is posted as a message. This to be sure  */
/*                that the correct thread initiates the Win-calls to destroy */
/*                the tooltip. Needed for instance for the GFP toolpalette   */
/*                which runs in another thread.                              */
/*---------------------------------------------------------------------------*/
toolTip::~toolTip()
{
   if (m_pszText) delete [] m_pszText;
   WinSetCapture(HWND_DESKTOP,NULLHANDLE);
   WinPostMsg(m_hTip,UM_DESTROY,(MPARAM)0,(MPARAM)0);
}
/*---------------------------------------------------------------------------*/
/*  toolTip::ToolTip_OnPaint.                                                */
/*                                                                           */
/*  Description : Handles the WM_PAINT message for the tooltip window.       */
/*---------------------------------------------------------------------------*/
void toolTip::ToolTip_OnPaint()
{
   HPS hps;
   RECTL rcl;
   LONG  lBackClr = 0x00FFFF80;

   WinQueryWindowRect(m_hTip,&rcl);
   hps = WinBeginPaint (m_hTip,(HPS)0,&rcl);
   GpiCreateLogColorTable (hps, LCOL_RESET,LCOLF_RGB,0,0,NULL);
   WinFillRect( hps, &rcl,lBackClr);


   WinDrawBorder(hps,&rcl,1L,1L,0L, 0L, DB_RAISED);

   if (m_pszText)
      WinDrawText(hps,strlen(m_pszText), m_pszText,&rcl,
                  0L,0L,DT_VCENTER | DT_CENTER | DT_TEXTATTRS);

   WinEndPaint(hps);
   return;
}
/*---------------------------------------------------------------------------*/
/*  ToolTipWndProc.                                                          */
/*                                                                           */
/*  Description : Window Procedure for the ToolTip popup.                    */
/*---------------------------------------------------------------------------*/
MRESULT EXPENTRY ToolTipWndProc(HWND hwnd,ULONG ulMsg,MPARAM mp1,MPARAM mp2)
{
    toolTip *pTip;

    // -- make DGROUP addressable

    pTip = (toolTip *)WinQueryWindowPtr(hwnd,0);

    switch (ulMsg)
    {
       case WM_TIMER:
          pTip->showToolTip();
          break;
       case WM_PAINT:
          pTip->ToolTip_OnPaint();
          return 0;
       case UM_DESTROY:
           WinStopTimer((HAB)0,pTip->getParent(),IDT_TOOLTIP);
           WinDestroyWindow(hwnd);
           return 0;
    }
    return WinDefWindowProc(hwnd,ulMsg,mp1,mp2);
}
/*-----------------------------------------------[ public ]------------------*/
/*  deleteTip.                                                               */
/*                                                                           */
/*  Description : Deletes the tooltip with the given window handle.          */
/*                                                                           */
/*  Parameters  : HWND hTip : Window handle of the tooltip window to be del..*/
/*                                                                           */
/*  Returns     : None.                                                      */
/*---------------------------------------------------------------------------*/
void delTip( void )
{
   toolTip *pTip;

   if (!hTip)
      return;

   pTip = (toolTip *)WinQueryWindowPtr(hTip,0);

   if (pTip)
      delete pTip;

   hTip = (HWND)0;

   return;
}
/*-----------------------------------------------[ public ]------------------*/
/*  newTip.                                                                  */
/*                                                                           */
/*  Description : Creates a new tooltip.                                     */
/*                                                                           */
/*  Parameters  : HWND hParent : Parent window handle.                       */
/*                PSZ pszText  : String to display.                          */
/*                                                                           */
/*  Returns     : BOOL - TRUE on success.                                    */
/*---------------------------------------------------------------------------*/
BOOL newTip(HWND hParent,PSZ pszText)
{
   HWND hwndAct;
   HWND hwnd;   

   toolTip *pTip;

   if (hTip)
   {
      /*
      ** This may never occur. But since accessible via owi
      ** our custommers have access too.
      */
      delTip();
   }
//   hwndAct = WinQueryActiveWindow(HWND_DESKTOP);
//   for (hwnd=hParent; hwnd && hwnd != hwndAct;
//       hwnd=WinQueryWindow(hwnd,QW_PARENT))
//       NULL;

//   if (!hwnd)
//      return FALSE;


   pTip = new toolTip(hParent,pszText);
   hTip = pTip->getWnd();
   return (BOOL)hTip;
}

/*-----------------------------------------------[ private ]-----------------*/
/*  loadTooltipfont.                                                         */
/*                                                                           */
/*  Description : Loads the font used on the tooltip.                        */
/*                LogicalfontName : tooltip.                                 */
/*                                                                           */
/*  Post        : Static szFont contains the fontname on succes.             */
/*                <point>.<fontname> example 8.Courier                       */
/*                                                                           */
/*  Returns     : BOOL - TRUE on success.                                    */
/*---------------------------------------------------------------------------*/
static BOOL loadTooltipfont( void )
{
#if 0
   CHAR *pszPoints = NULL;
   CHAR szLogFont[200];
   char *pszProfile;    /* ini filename        */
   HINI hProfile;       /* ini filehandle      */
   HAB  hab;            /* Anchor block handle */
   ULONG ulSize;

   if ((hProfile = PrfOpenProfile(hab,pszProfile)) != NULLHANDLE)
   {
      ulSize = PrfQueryProfileString(hProfile,     /*in: ini filehandle     */
                                     "screen",     /*in: Application name   */
                                     "tooltip",      /*in: Key name.          */
                                     "",           /*in: Default string     */
                                     szLogFont,    /*out: Profile string    */
                                     200);         /*in: buffersize.        */
      PrfCloseProfile(hProfile);
   }
   else
      ulSize = 0;

   pszPoints = strchr(szLogFont,',');

   if (pszPoints==NULL)
       return FALSE;

   pszPoints[0] = '\0';
   pszPoints++;
   (strchr(pszPoints,','))[0] = '\0';

   strcpy(szFont,pszPoints);
   strcat(szFont,".");
   strcat(szFont,szLogFont);
#endif
   return TRUE;
}
/*-----------------------------------------------[ public ]------------------*/
/*  isTipActive                                                              */
/*                                                                           */
/*  Description : Checks if a tooltip is active.                             */
/*                                                                           */
/*  Parameters  : NONE.                                                      */
/*                                                                           */
/*  Returns     : BOOL - TRUE when tooltip is active, else FALSE.           */
/*---------------------------------------------------------------------------*/
BOOL isTipActive( void )
{
   return (BOOL)hTip;
}

/*-----------------------------------------------[ public ]------------------*/
/*  TipNotify                                                                */
/*                                                                           */
/*  Description : Registers the tooltip class.                               */
/*                                                                           */
/*  Parameters  : NONE                                                       */
/*                                                                           */
/*  Returns     : NONE.                                                      */
/*---------------------------------------------------------------------------*/
void registerTooltip(HAB hab)
{
   ULONG ulStyle;


   ulStyle = (CS_SIZEREDRAW | CS_SAVEBITS );
   WinRegisterClass(hab,(PSZ)szClass,(PFNWP)ToolTipWndProc,
                    ulStyle,4L);


   strcpy(szFont,"9.WarpSans");

   cxScreen = WinQuerySysValue(HWND_DESKTOP,	SV_CXSCREEN);
   cyScreen = WinQuerySysValue(HWND_DESKTOP,	SV_CYSCREEN);

}

