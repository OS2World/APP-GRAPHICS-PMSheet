
/* Program name:   ImageBtn.C  Title: Demystifying Custom Controls   */
/* OS/2   Developer Magazine, Issue:  Winter '93, page 120                */
/* Author:  Mark Benge    Phone:   (919) 469-6446 Fax: (919) 469-6025   */
/*       Matt Smith    Phone:   (416) 363-2292 Fax: (416) 363-8452   */
/* Description:    Custom   controls demystified.  Sample image button   */
/*       with text.  Can be used as the   basis for other      */
/*       custom   controls.               */
/* Program Requirements:  OS/2 2.0, IBM   C Set/2   or WATCOM C 386/9.0   */
/*           OS/2 Toolkit               */

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
/*--ch------------------------description-------------------------------*/
/*  1                         Text on and below button disabled!        */
/*  2    160598   Disabled button should be white.                      */
/*----------------------------------------------------------------------*/
#define   INCL_DOS         /* Include OS/2 DOS Kernal      */
#define   INCL_GPI         /* Include OS/2 PM GPI Interface   */
#define   INCL_WIN         /* Include OS/2 PM Windows Interface   */

#include <os2.h>
#include <string.h>
#include <stdio.h>
#include "tbutton.h"
#include "tip.h"

#define DB_RAISED    0x0400	    /* the undocumented value's */
#define DB_DEPRESSED 0x0800     /* drawbox.                 */

/* This   module contains   an example installable control that can   be used   */
/* by any OS/2 2.0 Presentation   Manager   application.  The control   */
/* demonstrates   the principles of creating a custom control such   */
/* that   new custom controls can   be created using this as a model.   */
/*                           */
/* The control can be compiled using the IBM C Set/2 Version 1.0 with   */
/* the following:                     */
/*                           */
/*   Icc -G3e- -O+ -Rn -W3 -C -FoImageBtn.Obj ImageBtn.C      */
/*                           */
/* The control can be compiled using the WATCOM   C 386 Version 9.0 with   */
/* the following:                     */
/*                           */
/*   Wcl386 -ms   -3s -bd   -s -ox -zp4 -w3   -c -fo=ImageBtn.Obj ImageBtn.C   */

/* Filename:   ImageBtn.C                  */

/*  Version:   1.00                     */
/*  Created:   1992-06-02                  */
/*  Revised:   1992-08-30                  */
/* Released:   1993-01-27                  */

/* Routines:   static VOID CalcImagePos(PIMGBTN   pimgbtn, PRECTL   prcl,   */
/*               PPOINTL   pptl, LONG cx, LONG cy,   */
/*               LONG xShift, LONG yShift);   */
/*          static VOID SizeButton(HWND hWnd, LONG x, LONG y,   */
/*                  LONG cx, LONG cy);      */
/*          static VOID LoadBitmaps(PIMAGEBUTTON pib,      */
/*                   PIMGBTN pimgbtn,   HMODULE   hmod);   */
/*          static LONG lGetPresParam(HWND hWnd, ULONG ulID1,   */
/*                ULONG ulID2, LONG lDefault);   */
/*          static VOID SetDefaultColours(HWND hWnd,         */
/*                    PIMGBTN pimgbtn);      */
/*          MRESULT EXPENTRY   ImageBtnWndProc(HWND hWnd, ULONG msg,   */
/*                  MPARAM mp1, MPARAM mp2);*/


/* Copyright ¸ International Business Machines Corp., 1991,1992,1993.   */
/* Copyright ¸ 1989-1993  Prominare Inc.  All Rights Reserved.      */

/* --------------------------------------------------------------------   */


/* --- Internal   Structures --------------------------------------------   */

typedef   struct _IMGBTN         /* imgbtn */
   {
   ULONG    id;            /* ID Value            */
   ULONG    flStyle;         /* Style            */
   BOOL       fFocus;         /* Focus Flag         */
   RECTL    rclText;         /* Text Limits Rectangle      */
   RECTL    rcl;         /* Frame Rectangle         */
   RECTL    rclHitTest;         /* Hit Test Rectangle      */
   RECTL    rclSrc;         /* Source Rectangle   - Up      */
   RECTL    rclSrcDown;         /* Source Rectangle   - Down      */
   RECTL    rclSrcDisabled;      /* Source Rectangle   - Disabled   */
   POINTL   ptlDest;         /* Destination Point   - Up      */
   POINTL   ptlDestDown;      /* Destination Point   - Down      */
   POINTL   ptlDestDisabled;      /* Destination Point   - Disabled   */
   HBITMAP  hbm;         /* Bitmap Handle         */
   HBITMAP  hbmDown;         /* Bitmap Handle         */
   HBITMAP  hbmDisabled;      /* Bitmap Handle         */
   POINTL   aptlFocus[5];      /* Focus Emphasis Polyline Array   */
   POINTL   aptlOutline[8];      /* Outline Polyline Array      */
   POINTL   aptlShadow[12];      /* Shadow Polyline Array      */
   HWND       hwndOwner;         /* Owner Window Handle      */
   HWND       hwndParent;         /* Parent Window Handle      */
   ULONG    flState;         /* Button State         */
   ULONG    cx;            /* Bitmap Width  - Up      */
   ULONG    cy;            /* Bitmap Height - Up      */
   ULONG    cxDown;         /* Bitmap Width  - Down      */
   ULONG    cyDown;         /* Bitmap Height - Down      */
   ULONG    cxDisabled;         /* Bitmap Width  - Disabled      */
   ULONG    cyDisabled;         /* Bitmap Height - Disabled      */
   HPOINTER hptrArrow;         /* Arrow Pointer         */
   CHAR       szText[256];      /* Button Text         */
   CHAR     szTipText[MAX_TIPTEXT];   /* tooltip text      */
   ULONG    aClr[7];         /* Presentation Colours Array   */
   ULONG    lButtonShadow;      /* Button Shadow Colour      */
   ULONG    lButtonFace;      /* Button Face Colour      */
   ULONG    lButtonReflect;      /* Button Reflection   Edge Colour   */
   } IMGBTN ;

typedef   IMGBTN *PIMGBTN;

/* --- CTLDATA Structure Definition -----------------------------------   */

/************************************************************************/
/*                           */
/*     NOTE:   ID's for the bitmaps must be 0 < id < 65535              */
/*          This limitation is due to OS/2 PM Version 2.0      */
/*                           */
/*          OS/2 2.0   requires the first element of the CTLDATA   */
/*          structure to be the size   of the structure and this   */
/*          value must be less than 64 KB            */
/*                           */
/************************************************************************/

typedef   struct _IMAGEBUTTON      /* ibtn */
   {               /* Size:  16   Bytes         */
   LONG   cb;            /* Structure   Size         */
   LONG   idBitmap;         /* Bitmap ID   : Normal or Up Position   */
   LONG   idBitmapDown;         /* Bitmap ID   : Down Position      */
   LONG   idBitmapDisabled;      /* Bitmap ID   : Disabled      */
   char   szTipText[MAX_TIPTEXT];        /* tooltip */
   } IMAGEBUTTON ;

typedef   IMAGEBUTTON *PIMAGEBUTTON;

/* --- Module Prototype   Definitions -----------------------------------   */

static VOID CalcImagePos(PIMGBTN pimgbtn, PRECTL prcl, PPOINTL pptl, LONG cx, LONG cy, LONG xShift, LONG yShift);
static VOID SizeButton(HWND hWnd, LONG x, LONG y, LONG cx, LONG   cy);
static VOID LoadBitmaps(PIMAGEBUTTON pib, PIMGBTN pimgbtn, HMODULE hmod);
static LONG lGetPresParam(HWND hWnd, ULONG ulID1, ULONG   ulID2, LONG lDefault);
static VOID SetDefaultColours(HWND hWnd, PIMGBTN pimgbtn);

MRESULT   EXPENTRY ImageBtnWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);

static HMODULE hmodDLL;         /* DLL Module Handle         */



/* --- CalcImagePos -----------------------------------   [ Private ] ---   */
/*                           */
/*     This function is   used to   calculate the bitmap display      */
/*     co-ordinates such that the image   is properly placed within   */
/*     the display area.  It may be necessary to clip the image   to   */
/*     fit the button area if the image   is too large for the width   */
/*     or the height.                     */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     PIMGBTN pimgbtn;   = Image   Button Structure Pointer      */
/*     PRECTL  prcl;   = Source Rectangle Pointer         */
/*     POINTL  pptl;   = Target Display Point Pointer         */
/*     LONG    cx;   = Button Width               */
/*     LONG    cy;   = Button Height               */
/*     LONG    xShift;   = x Shift for Down Press         */
/*     LONG    yShift;   = y Shift for Down Press         */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     Nothing                        */
/*                           */
/* --------------------------------------------------------------------   */

static VOID CalcImagePos(PIMGBTN pimgbtn, PRECTL prcl, PPOINTL pptl,
          LONG cx, LONG cy, LONG   xShift,   LONG yShift)

{
             /* Determine if the bitmap image   width is   */
             /* smaller than the button width   and if the case   */
             /* determine the   best way of centering the image   */
             /* horizontally within the button rectangle   */

if ( (pimgbtn->rcl.xRight - pimgbtn->rcl.xLeft)   < cx )
   {
   prcl->xRight   = (prcl->xLeft = (cx -
        (pimgbtn->rcl.xRight - pimgbtn->rcl.xLeft)) /   2L) +
        (pimgbtn->rcl.xRight - pimgbtn->rcl.xLeft) - xShift;
   pptl->x = 0L   + xShift;
   }
else
   {
   pptl->x = (pimgbtn->rcl.xRight - pimgbtn->rcl.xLeft - cx) / 2L + 1L + xShift;
   prcl->xLeft   = 0;
   prcl->xRight   = cx - xShift;
   }
             /* Determine if the bitmap image   height is   */
             /* smaller than the button height and if   the   */
             /* case determine the best way of centering the   */
             /* image   vertically within the button rectangle   */

if ( ((pimgbtn->rcl.yTop - pimgbtn->rcl.yBottom) - (pimgbtn->rclText.yTop + 2L)) < cy )
   {
   prcl->yBottom = (cy - ((pimgbtn->rcl.yTop - pimgbtn->rcl.yBottom) -
           (pimgbtn->rclText.yTop + 2L))) /2L;
   prcl->yTop =   prcl->yBottom +
      ((pimgbtn->rcl.yTop - pimgbtn->rcl.yBottom) -
       (pimgbtn->rclText.yTop   + 2L));
   pptl->y = pimgbtn->rclText.yTop + 2L;
   }
else
   {
   pptl->y = pimgbtn->rclText.yTop + 2L;
   prcl->yBottom = 0;
   prcl->yTop    = cy;
   }
             /* Check   to see if the image should be shifted   */
             /* which   is usually done   for the   image when the   */
             /* button is pressed.  The shifting of the image   */
             /* gives   a greater 3D impression   of movement.   */
if ( yShift )
   prcl->yBottom += yShift;
}

/* --- SizeButton -------------------------------------   [ Private ] ---   */
/*                           */
/*     This function is   used to   calculate various elements of that   */
/*     make up the button.                  */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     HWND hWnd; = Window Handle               */
/*     LONG x;     = x Starting Point               */
/*     LONG y;     = y Starting Point               */
/*     LONG cx;     = Button Width               */
/*     LONG cy;     = Button Height               */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     Nothing                        */
/*                           */
/* --------------------------------------------------------------------   */

static VOID SizeButton(HWND hWnd, LONG x, LONG y, LONG cx, LONG   cy)

{
	HPS   hPS;            /* Presentation Space Handle      */
	PIMGBTN   pimgbtn;         /* Image Button Structure Pointer   */
	LONG   yStart;            /* Button Start Co-ordinate      */

				 /* Get the address of the private control data   */
				 /* structure               */

	pimgbtn   = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

				 /* Save the bounding rectangle of the control   */

	pimgbtn->rcl.xRight  = (pimgbtn->rcl.xLeft   = x) + cx;
	pimgbtn->rcl.yTop    = (pimgbtn->rcl.yBottom = y) + cy;

				 /* Set the display area for the text of the   */
				 /* control               */
	if (pimgbtn->szText[0] && (pimgbtn->flStyle &   IS_TEXTONBTN) )    /*@pr1*/
	{
		pimgbtn->rclText = pimgbtn->rcl;
		pimgbtn->rclText.xLeft   += 2L;
		pimgbtn->rclText.xRight   -= 2L;
		pimgbtn->rclText.yBottom = 4L;
	}
             /* Determine the   positioning of the text   by   */
             /* finding its drawing rectangle         */
             /* @pr1 Not more text on or below the button....   */

if (pimgbtn->szText[0] && (pimgbtn->flStyle &   IS_TEXTONBTN) )     /*@pr1*/
WinDrawText(hPS   = WinGetPS(hWnd), -1, (PCH)pimgbtn->szText, &pimgbtn->rclText,
       SYSCLR_OUTPUTTEXT, SYSCLR_BACKGROUND,
       DT_MNEMONIC   | DT_CENTER | DT_BOTTOM   | DT_QUERYEXTENT);

WinReleasePS(hPS);
             /* Calculate the   focus emphasis polyline   array   */
             /* points.  They   are as follows:         */
             /*                  */
             /*     1      2            */
             /*      +------+            */
             /*      | Text |            */
             /*    0 +------+            */
             /*      4      3            */
             /*                  */
             /* The emphasis array is   based on the actual   */
             /* display rectangle of the text.      */

pimgbtn->aptlFocus[0].x   = pimgbtn->rclText.xLeft   - 1L;
pimgbtn->aptlFocus[0].y   = pimgbtn->rclText.yBottom - 1L;
pimgbtn->aptlFocus[1].x   = pimgbtn->rclText.xLeft   - 1L;
pimgbtn->aptlFocus[1].y   = pimgbtn->rclText.yTop      + 1L;
pimgbtn->aptlFocus[2].x   = pimgbtn->rclText.xRight  + 1L;
pimgbtn->aptlFocus[2].y   = pimgbtn->rclText.yTop      + 1L;
pimgbtn->aptlFocus[3].x   = pimgbtn->rclText.xRight  + 1L;
pimgbtn->aptlFocus[3].y   = pimgbtn->rclText.yBottom - 1L;
pimgbtn->aptlFocus[4].x   = pimgbtn->rclText.xLeft   - 1L;
pimgbtn->aptlFocus[4].y   = pimgbtn->rclText.yBottom - 1L;

             /* Save the bounding rectangle of the control   */

pimgbtn->rcl.xRight  = (pimgbtn->rcl.xLeft   = x) + cx;
pimgbtn->rcl.yTop    = (pimgbtn->rcl.yBottom = y) + cy;

pimgbtn->rclHitTest = pimgbtn->rcl;

             /* Set the starting point for the button   image   */
             /* based   on its style in   relation to the   text   */
             /* that is to be   displayed for the control   */

if ( pimgbtn->flStyle &   IS_TEXTONBTN )
   pimgbtn->rclHitTest.yBottom = yStart   = 0L;
else
   pimgbtn->rclHitTest.yBottom = yStart   = pimgbtn->rclText.yTop   + 2L;

             /* Calculate the   control   outline   polyline array   */
             /* points.  They   are as follows:         */
             /*                  */
             /*      2     3            */
             /*    1 +------+ 4            */
             /*      |     |            */
             /*    0 +------+ 5            */
             /*      7     6            */

pimgbtn->aptlOutline[0].x = 0L;
pimgbtn->aptlOutline[0].y = yStart + 1L;
pimgbtn->aptlOutline[1].x = 0L;
pimgbtn->aptlOutline[1].y = pimgbtn->rcl.yTop -   1L;
pimgbtn->aptlOutline[2].x = 1L;
pimgbtn->aptlOutline[2].y = pimgbtn->rcl.yTop;
pimgbtn->aptlOutline[3].x = pimgbtn->rcl.xRight   - 1L;
pimgbtn->aptlOutline[3].y = pimgbtn->rcl.yTop;
pimgbtn->aptlOutline[4].x = pimgbtn->rcl.xRight;
pimgbtn->aptlOutline[4].y = pimgbtn->rcl.yTop -   1L;
pimgbtn->aptlOutline[5].x = pimgbtn->rcl.xRight;
pimgbtn->aptlOutline[5].y = yStart + 1L;
pimgbtn->aptlOutline[6].x = pimgbtn->rcl.xRight   - 1L;
pimgbtn->aptlOutline[6].y = yStart;
pimgbtn->aptlOutline[7].x = 1L;
pimgbtn->aptlOutline[7].y = yStart;

             /* Set the starting point for the button   image   */
             /* based   on its style in   relation to the   text   */
             /* that is to be   displayed for the control   */

if ( pimgbtn->flStyle &   IS_TEXTONBTN )
   yStart = 1L;
else
   yStart = pimgbtn->rclText.yTop + 2L;

             /* Calculate the   control   chamfer   polyline array   */
             /* points.  They   are as follows:         */
             /*                  */
             /*     1          2         */
             /*      +----------+            */
             /*      |+--------+|   8         */
             /*      ||4      3 ||            */
             /*      ||       9||            */
             /*      ||5        ||            */
             /*      || 11   10||            */
             /*      |+--------+|            */
             /*    0 +----------+            */
             /*      6          7         */

pimgbtn->aptlShadow[ 0].x = 1L;
pimgbtn->aptlShadow[ 0].y = yStart;
pimgbtn->aptlShadow[ 1].x = 1L;
pimgbtn->aptlShadow[ 1].y = pimgbtn->rcl.yTop -   1;
pimgbtn->aptlShadow[ 2].x = pimgbtn->rcl.xRight   - 1;
pimgbtn->aptlShadow[ 2].y = pimgbtn->rcl.yTop -   1;
pimgbtn->aptlShadow[ 3].x = pimgbtn->rcl.xRight   - 2;
pimgbtn->aptlShadow[ 3].y = pimgbtn->rcl.yTop -   2;
pimgbtn->aptlShadow[ 4].x = 2L;
pimgbtn->aptlShadow[ 4].y = pimgbtn->rcl.yTop -   2;
pimgbtn->aptlShadow[ 5].x = 2L;
pimgbtn->aptlShadow[ 5].y = yStart + 1L;

pimgbtn->aptlShadow[ 6].x = 2L;
pimgbtn->aptlShadow[ 6].y = yStart;
pimgbtn->aptlShadow[ 7].x = pimgbtn->rcl.xRight   - 1L;
pimgbtn->aptlShadow[ 7].y = yStart;
pimgbtn->aptlShadow[ 8].x = pimgbtn->rcl.xRight   - 1L;
pimgbtn->aptlShadow[ 8].y = pimgbtn->rcl.yTop -   2L;
pimgbtn->aptlShadow[ 9].x = pimgbtn->rcl.xRight   - 2L;
pimgbtn->aptlShadow[ 9].y = pimgbtn->rcl.yTop -   3L;
pimgbtn->aptlShadow[10].x = pimgbtn->rcl.xRight   - 2L;
pimgbtn->aptlShadow[10].y = yStart + 1L;
pimgbtn->aptlShadow[11].x = 2L;
pimgbtn->aptlShadow[11].y = yStart + 1L;

             /* Determine if the bitmap image   width is   */
             /* smaller than the button width   and if the case   */
             /* determine the   best way of centering the image   */
             /* horizontally within the button rectangle   */

CalcImagePos(pimgbtn, &pimgbtn->rclSrc,   &pimgbtn->ptlDest,
        pimgbtn->cx, pimgbtn->cy, 0L, 0L);
CalcImagePos(pimgbtn, &pimgbtn->rclSrcDown, &pimgbtn->ptlDestDown,
        pimgbtn->cxDown, pimgbtn->cyDown, 2L, 2L);
CalcImagePos(pimgbtn, &pimgbtn->rclSrcDisabled,   &pimgbtn->ptlDestDisabled,
        pimgbtn->cxDisabled, pimgbtn->cyDisabled, 0L, 0L);
}

/* --- LoadBitmaps ------------------------------------   [ Private ] ---   */
/*                           */
/*     This function is   used to   load the specified bitmaps from   the   */
/*     resources of the   calling   executable using the information   */
/*     provided   within a CTLDATA type structure.         */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     PIMAGEBUTTON pib;     = Image Button CTLDATA Pointer      */
/*     PIMGBTN       pimgbtn; = Internal   Data Pointer         */
/*     HMODULE       hmod;    = Module Handle            */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     Nothing                        */
/*                           */
/* --------------------------------------------------------------------   */

static VOID LoadBitmaps(PIMAGEBUTTON pib, PIMGBTN pimgbtn, HMODULE hmod)

{
BITMAPINFOHEADER bmpi;         /* Bitmap Information Header      */
HBITMAP       hbm;         /* Bitmap Handle         */
HMODULE       hmodBitmap;      /* Module Handle         */
HPS       hPS;         /* Presentation Space Handle      */
LONG       id;         /* ID Value            */

             /* Load each of the image bitmaps from the   */
             /* calling executable and create   a bitmap that   */
             /* will be shown   in each   button when the   ID for   */
             /* bitmaps are provided otherwise use the   */
             /* images contained within the control DLL   */

if ( pib && pib->idBitmap )
   {
             /* Set the module handle   for the   bitmap as the   */
             /* resources within the calling executable   */
   hmodBitmap =   hmod;
   id         =   pib->idBitmap;
   }
else
   {
             /* Set the module handle   for the   bitmap as the   */
             /* resources within DLL therefore get the   */
             /* handle of the   DLL            */

   hmodBitmap =   hmodDLL;
   id         =   IDB_DEFAULT;
   }
             /* Get a   temporary presentation space for the   */
             /* control to allow the creation   of the bitmaps   */
             /* for each of the buttons.  Then load the   */
             /* bitmap image in from the selected executable   */
             /* or DLL.  When   this fails, try   getting   the   */
             /* default image   from the resources of the DLL.   */


if ( hbm = GpiLoadBitmap(hPS = WinGetPS(HWND_DESKTOP), hmodBitmap,
          id, 0L, 0L) )
   {
   if (   pimgbtn->hbm )
       GpiDeleteBitmap(pimgbtn->hbm);
   pimgbtn->hbm   = hbm;
   }
else
   if (   hbm = GpiLoadBitmap(hPS, hmodDLL, IDB_DEFAULT, 0L, 0L) )
       {
       if ( pimgbtn->hbm )
      GpiDeleteBitmap(pimgbtn->hbm);
       pimgbtn->hbm = hbm;
       }
             /* Get the size of the bitmap that has been   */
             /* loaded to allow the proper positioning of it   */
             /* within the final button         */

GpiQueryBitmapParameters(pimgbtn->hbm, &bmpi);
pimgbtn->cx = bmpi.cx;
pimgbtn->cy = bmpi.cy;
             /* Get the down ID of the button         */

if ( pib && pib->idBitmapDown )
   id =   pib->idBitmapDown;
else
   id =   IDB_DEFAULTDOWN;
             /* Load the down   image from the resources   */

if ( hbm = GpiLoadBitmap(hPS, hmodBitmap, id, 0L, 0L) )
   {
   if (   pimgbtn->hbmDown )
       GpiDeleteBitmap(pimgbtn->hbmDown);
   pimgbtn->hbmDown = hbm;
   }
else
   if (   hbm = GpiLoadBitmap(hPS, hmodDLL, IDB_DEFAULTDOWN, 0L, 0L) )
       {
       if ( pimgbtn->hbmDown )
      GpiDeleteBitmap(pimgbtn->hbmDown);
       pimgbtn->hbmDown   = hbm;
       }
             /* Get the size of the bitmap that has been   */
             /* loaded to allow the proper positioning of it   */
             /* within the final button         */

GpiQueryBitmapParameters(pimgbtn->hbmDown, &bmpi);
pimgbtn->cxDown   = bmpi.cx;
pimgbtn->cyDown   = bmpi.cy;

             /* Get the disabled ID of the button      */

if ( pib && pib->idBitmapDisabled)
   id =   pib->idBitmapDisabled;
else
   id =   IDB_DEFAULTDISABLED;

             /* Load the disabled image from the resources   */

if ( hbm = GpiLoadBitmap(hPS, hmodBitmap, id, 0L, 0L) )
   {
   if (   pimgbtn->hbmDisabled )
       GpiDeleteBitmap(pimgbtn->hbmDisabled);
   pimgbtn->hbmDisabled   = hbm;
   }
else
   if (   hbm = GpiLoadBitmap(hPS, hmodDLL, IDB_DEFAULTDISABLED, 0L, 0L) )
       {
       if ( pimgbtn->hbmDisabled )
      GpiDeleteBitmap(pimgbtn->hbmDisabled);
       pimgbtn->hbmDisabled = hbm;
       }
             /* Get the size of the bitmap that has been   */
             /* loaded to allow the proper positioning of it   */
             /* within the final button         */

GpiQueryBitmapParameters(pimgbtn->hbmDisabled, &bmpi);
pimgbtn->cxDisabled = bmpi.cx;
pimgbtn->cyDisabled = bmpi.cy;

             /* Release the temporary   presentation space   */
WinReleasePS(hPS);
}

/* --- lGetPresParam ----------------------------------   [ Private ] ---   */
/*                           */
/*     This function is   used to   retrieve a presentation   parameter   */
/*     that may   be present.  If   the presentation parameter is not,   */
/*     the default colour passed to the   function will be used.      */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     HWND  hWnd;     = Window   Handle               */
/*     ULONG ulID1;    = Presentation Parameter   1 ID         */
/*     ULONG ulID2;    = Presentation Parameter   2 ID         */
/*     LONG  lDefault; = Default Colour               */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     lGetPresParam = Colour to Use               */
/*                           */
/* --------------------------------------------------------------------   */

static LONG lGetPresParam(HWND hWnd, ULONG ulID1, ULONG   ulID2, LONG lDefault)

{
HPS   hPS;            /* Presentation Space Handle      */
LONG  lClr;            /* Presentation Parameter Colour   */
ULONG ulID;            /* Presentation Parameter ID      */

if ( WinQueryPresParam(hWnd, ulID1, ulID2, &ulID, 4, (PVOID)&lClr,
             QPF_NOINHERIT | QPF_ID1COLORINDEX | QPF_ID2COLORINDEX | QPF_PURERGBCOLOR) )
   return(lClr);
else
             /* Check   to see if the colour requested as the   */
             /* default is a SYSCLR_*   which is a special   */
             /* colour index.    When the case,   use the      */
             /* specific routine to determine   the colour of   */
             /* the system colour index that is provided   */
             /* through OS/2               */

   if (   (lDefault >= SYSCLR_SHADOWHILITEBGND) &&
   (lDefault <= SYSCLR_HELPHILITE)   )
       return(WinQuerySysColor(HWND_DESKTOP, lDefault, 0L));
   else
       if ( (lClr = GpiQueryRGBColor(hPS = WinGetPS(hWnd),
                 LCOLOPT_REALIZED, lDefault)) == GPI_ALTERROR )
      {
      WinReleasePS(hPS);
      return(lDefault);
      }
       else
      {
      WinReleasePS(hPS);
      return(lClr);
      }
}

/* --- SetDefaultColours ------------------------------   [ Private ] ---   */
/*                           */
/*     This function is   used to   set the   default   colours   that the   */
/*     image button should use within the internal paint routines.   */
/*     The colour can either be   a presentation parameter that has   */
/*     been set   or it can be the default colour   as defined within   */
/*     control.                        */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     HWND    hWnd;   = Window Handle               */
/*     PIMGBTN pimgbtn;   = Image   Button Structure Pointer      */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     Nothing                        */
/*                           */
/* --------------------------------------------------------------------   */

static VOID SetDefaultColours(HWND hWnd, PIMGBTN pimgbtn)

{
HPS  hPS;            /* Presentation Space      */
LONG lClr;            /* Colour            */

             /* Set up the colours that will be used within   */
             /* the painting of the control.   The colour   */
             /* indices are:               */
             /*                  */
             /* 0 : Foreground (PP_FOREGROUND*)      */
             /* 1 : Background (PP_BACKGROUND*)      */
             /* 2 : Hilight Foreground (PP_HILITEFOREGROUND*)   */
             /* 3 : Hilight Background (PP_HILITEBACKGROUND*)   */
             /* 4 : Disabled Foreground (PP_DISABLEDFORE*)   */
             /* 5 : Disabled Foreground (PP_DISABLEDFORE*)   */
             /* 6 : Border (PP_BORDER*)         */

pimgbtn->aClr[0] = lGetPresParam(hWnd, PP_FOREGROUNDCOLOR,
             PP_FOREGROUNDCOLORINDEX,
             SYSCLR_OUTPUTTEXT);
pimgbtn->aClr[1] = lGetPresParam(hWnd, PP_BACKGROUNDCOLOR,
             PP_BACKGROUNDCOLORINDEX,
             SYSCLR_FIELDBACKGROUND);
pimgbtn->aClr[2] = lGetPresParam(hWnd, PP_HILITEFOREGROUNDCOLOR,
             PP_HILITEFOREGROUNDCOLORINDEX,
             SYSCLR_OUTPUTTEXT);
pimgbtn->aClr[3] = lGetPresParam(hWnd, PP_HILITEBACKGROUNDCOLOR,
             PP_HILITEBACKGROUNDCOLORINDEX,
             SYSCLR_BACKGROUND);
pimgbtn->aClr[4] = lGetPresParam(hWnd, PP_DISABLEDFOREGROUNDCOLOR,
             PP_DISABLEDFOREGROUNDCOLORINDEX,
             SYSCLR_OUTPUTTEXT);
pimgbtn->aClr[5] = lGetPresParam(hWnd, PP_DISABLEDBACKGROUNDCOLOR,
             PP_DISABLEDBACKGROUNDCOLORINDEX,
             SYSCLR_BACKGROUND);
pimgbtn->aClr[6] = lGetPresParam(hWnd, PP_BACKGROUNDCOLOR,
             PP_BACKGROUNDCOLORINDEX,
             SYSCLR_BUTTONDARK);

if ( (lClr = GpiQueryRGBColor(hPS = WinGetPS(hWnd),
               LCOLOPT_REALIZED,   SYSCLR_BUTTONDARK)) == GPI_ALTERROR )
   pimgbtn->lButtonShadow = SYSCLR_BUTTONDARK;
else
   pimgbtn->lButtonShadow = lClr;

if ( (lClr = GpiQueryRGBColor(hPS, LCOLOPT_REALIZED,
               SYSCLR_BUTTONMIDDLE)) == GPI_ALTERROR )
   pimgbtn->lButtonFace   = SYSCLR_BUTTONMIDDLE;
else
   pimgbtn->lButtonFace   = lClr;

if ( (lClr = GpiQueryRGBColor(hPS, LCOLOPT_REALIZED,
               SYSCLR_BUTTONLIGHT)) == GPI_ALTERROR )
   pimgbtn->lButtonReflect = SYSCLR_BUTTONLIGHT;
else
   pimgbtn->lButtonReflect = lClr;

WinReleasePS(hPS);
}
/* --- ImageBtnWndProc ------------------------------------------------   */
/*                           */
/*     This function is   used to   process   the messages for the image   */
/*     button control.                     */
/*                           */
/*     Upon Entry:                     */
/*                           */
/*     HWND   hWnd; = Window Handle               */
/*     ULONG  msg;  = PM Message               */
/*     MPARAM mp1;  = Message Parameter   1            */
/*     MPARAM mp2;  = Message Parameter   2            */
/*                           */
/*     Upon Exit:                     */
/*                           */
/*     ImageBtnWndProc = Message Handling Result         */
/*                           */
/* --------------------------------------------------------------------   */

MRESULT   EXPENTRY ImageBtnWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
BITMAPINFOHEADER bmpi;         /* Bitmap Info Header      */
CHAR       ch;         /* Character   Holder         */
HBITMAP       hbm;         /* Bitmap Handle         */
HPS       hPS;         /* Presentation Space Handle      */
LONG       lClr;         /* Presentation Parameter Colour   */
PCHAR       pch;         /* Character   String Pointer      */
PCREATESTRUCT    pcrst;         /* Create Structure Pointer      */
PIMAGEBUTTON    pib,p;         /* Image Button CTLDATA Pointer   */
PIMGBTN       pimgbtn;      /* Image Button Structure Pointer   */
POINTL       ptl;         /* Drawing Point         */
PWNDPARAMS    pwprm;         /* Window Parameters   Pointer      */
RECTL       rcl;         /* Rectangle   Holder         */
ULONG       ulID;         /* Presentation Parameter ID      */

switch ( msg )
   {

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   1: Control creation coding               */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control creation                     */
   /*********************************************************************/

   case   WM_CREATE :
             /* Get the address of the CTLDATA structure that   */
             /* may contain the bitmap information that the   */
             /* control can use during its creation instead   */
             /* of using messages to set the button images   */
       if ( pib   = (PIMAGEBUTTON)PVOIDFROMMP(mp1) )


             /* Check   to see that the   structure passed is   */
             /* what is expected and if not, return      */
             /* indicating that the control window should   */
             /* not be further created         */

             /*************************************************/
             /*  NOTE:   OS/2   2.0 requires the first element   */
             /*      of the CTLDATA structure to be the   */
             /*      size   of the structure and this   */
             /*      value must be less than 64 KB   */
             /*************************************************/

//      if (   (pib->cb != sizeof(IMAGEBUTTON)) )
//          return(MRFROMLONG(TRUE));

             /* Allocate memory for internal control data   */



       DosAllocMem((PPVOID)&pimgbtn, sizeof(IMGBTN),
         PAG_READ | PAG_WRITE   | PAG_COMMIT);

             /* Save the address of the internal control data   */
             /* in the control's reserved memory to allow it  */
             /* to be   referenced as required by the control   */

       WinSetWindowPtr(hWnd, QUCWP_WNDP, (PVOID)pimgbtn);

             /* Load each of the image bitmaps from the   */
             /* calling executable and create   a bitmap that   */
             /* will be shown   in each   button when the   ID for   */
             /* bitmaps are provided otherwise use the   */
             /* images contained within the control DLL   */

       LoadBitmaps(pib,   pimgbtn, (HMODULE)NULL);

             /* Get the control's creation structure address  */
             /* to copy the relevant information such   as the   */
             /* size,   position and text of the control into   */
             /* the internal control data         */

       pcrst = (PCREATESTRUCT)(void *)(mp2);

             /* Save the owner and parent of the control so   */
             /* notification messages   can be sent back to   */
             /* the proper locations within the owning   */
             /* application               */

       pimgbtn->hwndOwner  = pcrst->hwndOwner;
       pimgbtn->hwndParent = pcrst->hwndParent;

             /* Save the ID of the control and set the   */
             /* initial position of the control to up   and   */
             /* save the default style along with the   normal   */
             /* arrow   pointer   handle which will be used when   */
             /* the pointer passes over the control      */

       pimgbtn->id      = pcrst->id;
       pimgbtn->flState      = IBS_UP;
       pimgbtn->flStyle      = pcrst->flStyle;
       pimgbtn->hptrArrow  = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW,
                  FALSE);

             /* Get the text of the control and save within   */
             /* the internal control data         */

       strcpy(pimgbtn->szText,(char *)pcrst->pszText);

             /* Set up the colours that will be used within   */
             /* the painting of the control         */

       SetDefaultColours(hWnd, pimgbtn);

             /* Check   to see if the user provided font that   */
             /* should override the default font that   would   */
             /* be set               */

       if ( !WinQueryPresParam(hWnd, PP_FONTNAMESIZE, 0L, &ulID, 4,
                (PVOID)&lClr, QPF_NOINHERIT) )

             /* System indicates font   not set   since no data   */
             /* was returned,   therefore set default font for   */
             /* the control               */

      WinSetPresParam(hWnd, PP_FONTNAMESIZE, 7L, (PVOID)"8.Helv");

             /* Save the initial size   of the control      */

       SizeButton(hWnd,   0, 0, pcrst->cx, pcrst->cy);

       if (pib->szTipText[0]);
            strcpy(pimgbtn->szTipText,pib->szTipText);
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   2: Control text   and colour support coding         */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Process window parameters setting            */
   /*********************************************************************/

   case   WM_SETWINDOWPARAMS :

             /* Get the address for the windows parameters   */
             /* structure               */

       pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1);

             /* Check   to see if the text for the control is   */
             /* being   set               */

       if ( pwprm->fsStatus & WPM_TEXT )
      {
             /* Text being set, get the address of the text   */
             /* string stored   in the heap         */

      pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if any text is being set      */

      if (   pwprm->cchText )

             /* Check   to make   sure that the text that   is to   */
             /* be set is not   greater   than the memory      */
             /* allocated               */

          if ( pwprm->cchText > 255 )
         {
         strncpy(pimgbtn->szText, pwprm->pszText, 255);
         pimgbtn->szText[255]   = 0;
         }
          else
         strcpy(pimgbtn->szText,(char *)pwprm->pszText);
      else
             /* No text is being set,   clear any existing text   */

          pimgbtn->szText[0] = 0;
      }
       break;

   /*********************************************************************/
   /*  Process window parameters query               */
   /*********************************************************************/

   case   WM_QUERYWINDOWPARAMS :

             /* Get the address for the windows parameters   */
             /* structure               */

       pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1);

             /* Determine the   type of   query         */

       switch (   pwprm->fsStatus   )
      {
             /* Query   type:  get text            */

      case   WPM_TEXT :

             /* Text being asked for,   get the   address   of the   */
             /* text string stored in   the heap      */

          pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Copy the text   from the string   to the      */
             /* structure               */

          strcpy(pwprm->pszText,(char *)pimgbtn->szText);
          break;
             /* Query   type:  get text   length         */

      case   WPM_CCHTEXT :

             /* Text length being asked for, get the address   */
             /* of the text string stored in the heap      */

          pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Place   the length the string within the   */
             /* structure               */

          pwprm->cchText =   (SHORT)strlen(pimgbtn->szText);
          break;
             /* Query   type:  get control data   length      */

      case   WPM_CBCTLDATA :

             /* Set the control data length to zero      */

          pwprm->cbCtlData   = 0;
          break;

      default :
          return(WinDefWindowProc(hWnd, msg, mp1, mp2));
      }
       break;

   /*********************************************************************/
   /*  Presentation parameters changed,   record the change internally   */
   /*********************************************************************/

   case   WM_PRESPARAMCHANGED :

             /* Check   to see if an individual   presentation   */
             /* parameter has   changed   if so, get the new   */
             /* colour value for use by the painting routines   */

       if ( LONGFROMMP(mp1) && (LONGFROMMP(mp1)   < PP_FONTNAMESIZE) )
      {
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

      pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Get the new presentation parameter colour for   */
             /* the presentation parameter that has changed.   */
             /* Get the colour as a RGB value   so as to be   */
             /* able to get an exact value and not an      */
             /* approximation   which could happen if the   */
             /* presentation parameter was set as a RGB but   */
             /* queried as an   index.   When WinQueryPresParam   */
             /* returns a 0, it indicates that no      */
             /* presentation parameter set and the default   */
             /* colours should be used.         */

      switch ( LONGFROMMP(mp1) )
          {
          case PP_FOREGROUNDCOLOR :
          case PP_FOREGROUNDCOLORINDEX :
         pimgbtn->aClr[0] = lGetPresParam(hWnd, PP_FOREGROUNDCOLOR,
                      PP_FOREGROUNDCOLORINDEX,
                      SYSCLR_OUTPUTTEXT);
         break;

          case PP_BACKGROUNDCOLOR :
          case PP_BACKGROUNDCOLORINDEX :
         pimgbtn->aClr[1] = lGetPresParam(hWnd, PP_BACKGROUNDCOLOR,
                      PP_BACKGROUNDCOLORINDEX,
                      SYSCLR_BACKGROUND);
         break;

          case PP_HILITEFOREGROUNDCOLOR :
          case PP_HILITEFOREGROUNDCOLORINDEX :
         pimgbtn->aClr[2] = lGetPresParam(hWnd, PP_HILITEFOREGROUNDCOLOR,
                      PP_HILITEFOREGROUNDCOLORINDEX,
                      SYSCLR_OUTPUTTEXT);
         break;

          case PP_HILITEBACKGROUNDCOLOR :
          case PP_HILITEBACKGROUNDCOLORINDEX :
         pimgbtn->aClr[3] = lGetPresParam(hWnd, PP_HILITEBACKGROUNDCOLOR,
                      PP_HILITEBACKGROUNDCOLORINDEX,
                      SYSCLR_BACKGROUND);
         break;

          case PP_DISABLEDFOREGROUNDCOLOR :
          case PP_DISABLEDFOREGROUNDCOLORINDEX :
         pimgbtn->aClr[4] = lGetPresParam(hWnd, PP_DISABLEDFOREGROUNDCOLOR,
                      PP_DISABLEDFOREGROUNDCOLORINDEX,
                      SYSCLR_OUTPUTTEXT);
         break;

          case PP_DISABLEDBACKGROUNDCOLOR :
          case PP_DISABLEDBACKGROUNDCOLORINDEX :
         pimgbtn->aClr[5] = lGetPresParam(hWnd, PP_BACKGROUNDCOLOR,
                      PP_BACKGROUNDCOLORINDEX,
                      SYSCLR_BACKGROUND);
         break;

          case PP_BORDERCOLOR :
          case PP_BORDERCOLORINDEX   :
         pimgbtn->aClr[6] = lGetPresParam(hWnd, PP_BACKGROUNDCOLOR,
                      PP_BACKGROUNDCOLORINDEX,
                      SYSCLR_BUTTONDARK);
         break;
          }
             /* Invalidate the button   to force to use   the   */
             /* new colours just set or removed      */

          WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
      }
       else
             /* Determine if the Scheme Palette has forced a   */
             /* global scheme   update in which   case, check all   */
             /* of the presentation parameters to see   if they   */
             /* have been added or removed         */

      if (   LONGFROMMP(mp1)   == 0L )

             /* Set up the colours that will be used within   */
             /* the painting of the control.         */

          SetDefaultColours(hWnd,
             pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP));
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   3: Control size   and position coding            */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Size of the control changing               */
   /*********************************************************************/

   case   WM_SIZE   :
             /* Get the actual position of the control as a   */
             /* rectangle to allow it   to be saved within the   */
             /* internal control data            */

       WinQueryWindowRect(hWnd,   &rcl);

             /* Save the new control position   and size   */

       SizeButton(hWnd,   rcl.xLeft, rcl.yBottom,
        (rcl.xRight -   rcl.xLeft),
        (rcl.yTop   -   rcl.yBottom));
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   4: Control enable/disable and focus/defocus coding      */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control is being   activated or deactivated         */
   /*********************************************************************/

   case   WM_ACTIVATE :
             /* When the control is being activated or   */
             /* deactivated, make sure that the control   */
             /* responds properly to help requests when it   */
             /* has the focus               */

       if ( SHORT1FROMMP(mp1) )
      WinSendMsg(WinQueryHelpInstance(hWnd),
            HM_SET_ACTIVE_WINDOW,
            MPFROMHWND(WinQueryWindow(hWnd, QW_PARENT)),
            MPFROMHWND(WinQueryWindow(hWnd, QW_PARENT)));
       else
      WinSendMsg(WinQueryHelpInstance(hWnd),
            HM_SET_ACTIVE_WINDOW, 0L,   0L);
       break;

   /*********************************************************************/
   /*  Control is being   disable   or enabled            */
   /*********************************************************************/

   case   WM_ENABLE :

             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if the control is being disabled   */
             /* or enabled and set the state of the control   */
             /* within ther internal control data      */

       if ( (BOOL)SHORT1FROMMP(mp1) )
      pimgbtn->flState &= ~IBS_DISABLED;
       else
      pimgbtn->flState |= IBS_DISABLED;

             /* Force   the repainting of the control as its   */
             /* state   is changing            */

       WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
       break;

   /*********************************************************************/
   /*  Control is gaining or losing focus            */
   /*********************************************************************/

   case   WM_SETFOCUS :

             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Set the focus   state for the control based   */
             /* on the information from the message      */

       pimgbtn->fFocus = (BOOL)SHORT1FROMMP(mp2);

             /* Force   the repainting of the control as its   */
             /* state   is changing            */

       WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
       break;

   /*********************************************************************/
   /*  Control is gaining or losing selection            */
   /*********************************************************************/

   case   WM_SETSELECTION   :

             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Set the focus   state for the control based   */
             /* on the information from the message      */

       pimgbtn->fFocus = (BOOL)SHORT1FROMMP(mp1);

             /* Force   the repainting of the control as its   */
             /* state   is changing            */

       WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
       break;

   /*********************************************************************/
   /*  Control is gaining or losing selection            */
   /*********************************************************************/

   case   WM_QUERYDLGCODE   :
       return(MRFROMLONG(DLGC_PUSHBUTTON));

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   5: User   interaction coding for keyboard   and mouse      */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Check for key strokes                  */
   /*********************************************************************/

   case   WM_CHAR   :
             /* Check   for the   key up flag in which case the   */
             /* condition should be ignored         */

       if ( SHORT1FROMMP(mp1) &   KC_KEYUP )
      return(0L);
             /* Check   for virtual keys         */

       if ( SHORT1FROMMP(mp1) &   KC_VIRTUALKEY )
      switch ( SHORT2FROMMP(mp2) )
          {
          case VK_SPACE : //@ch3
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

         pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

         pimgbtn->flState = IBS_DOWN;
         WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);

             /* Post a message to the   main window using the   */
             /* command message value   to inform the control   */
             /* owner   that the button   has been selected   */

         WinSendMsg(pimgbtn->hwndOwner, WM_COMMAND,
               MPFROMSHORT(pimgbtn->id),
               MPFROM2SHORT(FALSE, CMDSRC_OTHER));

             /* Invalidate the button   image to update   it from   */
             /* down position               */

         pimgbtn->flState = IBS_UP;
         WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
         return(0L);

          case VK_TAB :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

         pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* TAB key pressed, determine which control is   */
             /* the next tab stop and   set the   focus on that   */
             /* control               */

         WinSetFocus(HWND_DESKTOP,
                WinEnumDlgItem(pimgbtn->hwndOwner, hWnd,
                     EDI_NEXTTABITEM));
         return(0L);

          case VK_RIGHT :
          case VK_DOWN :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

         pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Right   or down   arrow key pressed, determine   */
             /* which   control   is the next entry and set the   */
             /* focus   on that   control            */

         WinSetFocus(HWND_DESKTOP,
                WinEnumDlgItem(pimgbtn->hwndOwner, hWnd,
                     EDI_NEXTGROUPITEM));
         return(0L);

          case VK_BACKTAB :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

         pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Shift+TAB key   pressed, determine which   */
             /* control is the previous tab stop and set the   */
             /* focus   on that   control            */

         WinSetFocus(HWND_DESKTOP,
                WinEnumDlgItem(pimgbtn->hwndOwner, hWnd,
                     EDI_PREVTABITEM));
         return(0L);

          case VK_LEFT :
          case VK_UP :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

         pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Left or up arrow key pressed,   determine   */
             /* which   control   is the previous   entry and set   */
             /* the focus on that control         */

         WinSetFocus(HWND_DESKTOP,
                WinEnumDlgItem(pimgbtn->hwndOwner, hWnd,
                     EDI_PREVGROUPITEM));
         return(0L);
          }
             /* Virtual key not TAB or Shift+TAB, fall   */
             /* through to default window procedure      */

       return(WinDefWindowProc(hWnd, msg, mp1, mp2));

   /*********************************************************************/
   /*  Button 1   pressed                     */
   /*********************************************************************/

   case   WM_BUTTON1DOWN :
       delTip();
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if the button is   disabled and if   */
             /* the case, ignore the button request      */

       if ( pimgbtn->flState & IBS_DISABLED )
       return(0L);
       else
      {
             /* Post a message to the   main window using the   */
             /* command message value   to inform the control   */
             /* owner   that the button   has been selected   */

      ptl.x = (LONG)SHORT1FROMMP(mp1);
      ptl.y = (LONG)SHORT2FROMMP(mp1);
      if (   WinPtInRect((HAB)NULL, &pimgbtn->rclHitTest, &ptl) )
          {
             /* Since   the user has clicked the mouse pointer   */
             /* on the control, set the state   of the button   */
             /* as being down   and in capture mode in case the   */
             /* user moves the mouse outside of the area of   */
             /* control while   keeping   button 1 of the   mouse   */
             /* down                  */

          pimgbtn->flState   = IBS_DOWN | IBS_CAPTURE;

             /* Set the mouse   capture   to follow the movement   */
             /* of the mouse pointer until the mouse button   */
             /* is released               */

          WinSetCapture(HWND_DESKTOP, hWnd);

             /* Remove focus from the   control   if it has been   */
             /* selected               */

          pimgbtn->fFocus = FALSE;

             /* Force   the repainting of the control      */

          WinInvalidateRect(hWnd, NULL, TRUE);

             /* Since   the control is receiving the focus,   */
             /* make sure the   focus changes from the current   */
             /* control selected within the window or      */
             /* dialogue to this control         */

          WinSetFocus(HWND_DESKTOP, hWnd);
          }
      }
       break;

   /*********************************************************************/
   /*  Button 1   released                  */
   /*********************************************************************/

   case   WM_BUTTON1UP :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if the button is   disabled and if   */
             /* the case, ignore the button request      */

       if ( pimgbtn->flState & IBS_DISABLED )
      return(0L);
       else
      {
             /* Since   the user has clicked and released the   */
             /* mouse   pointer   on the control,   set the   state   */
             /* of the button   as being up and   remove the   */
             /* capture mode setting since interaction with   */
             /* the button only occurrs when the mouse   */
             /* button is depressed            */

      pimgbtn->flState = IBS_UP;

             /* Release the mouse capture since the user has   */
             /* released the mouse button         */

      WinSetCapture(HWND_DESKTOP, (HWND)NULL);

             /* Post a message to the   main window using the   */
             /* command message value   to inform the control   */
             /* owner   that the button   has been selected   */

      ptl.x = (LONG)SHORT1FROMMP(mp1);
      ptl.y = (LONG)SHORT2FROMMP(mp1);
      if (   WinPtInRect((HAB)NULL, &pimgbtn->rclHitTest, &ptl) )
          {
          WinPostMsg(pimgbtn->hwndOwner, WM_COMMAND,
           MPFROMSHORT(pimgbtn->id),
           MPFROM2SHORT(TRUE, CMDSRC_OTHER));

             /* Invalidate the button   image to update   it from   */
             /* down position               */

          WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);

             /* Since   the control is receiving the focus,   */
             /* make sure the   focus changes from the current   */
             /* control selected within the window or      */
             /* dialogue to this control         */

          WinSetFocus(HWND_DESKTOP, hWnd);
          }
      }
       break;

   /*********************************************************************/
   /* Process mouse movement over the button            */
   /*********************************************************************/

   case   WM_MOUSEMOVE :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       ptl.x = (LONG)SHORT1FROMMP(mp1);
       ptl.y = (LONG)SHORT2FROMMP(mp1);
       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if the mouse is in capture mode   */
             /* which   indicates that the user   has depressed   */
             /* button 1 of the mouse   and kept it down while   */
             /* moving the mouse pointer around      */

       if ( pimgbtn->flState & IBS_CAPTURE )
      {
             /* When in capture mode,   the control checks to   */
             /* see where the   pointer   is located such   that   */
             /* when the mouse pointer is outside the   control   */
             /* area,   the control's appearance is as though   */
             /* it is   in the up position.  When the pointer   */
             /* is within the   control   area, the down      */
             /* position of the control is shown.      */


             /* Check   to see where the mouse pointer is   */
             /* located within relationship to the control   */

      if (   !WinPtInRect((HAB)NULL,   &pimgbtn->rclHitTest, &ptl) )
          {
             /* The mouse is outside of the control area,   */
             /* check   to see if the control is currently   */
             /* shown   as down   and if the case, set it   to the   */
             /* up position internally and force it to be   */
             /* repainted in the up position         */

          if ( pimgbtn->flState & IBS_DOWN   )
         {
         pimgbtn->flState = IBS_UP | IBS_CAPTURE;
         WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
         }
          }
      else
             /* The mouse is inside of the control area,   */
             /* check   to see if the control is currently   */
             /* shown   as up and if the case, set it to the   */
             /* down position   internally and force it   to be   */
             /* repainted in the down   position      */

          if ( pimgbtn->flState & IBS_UP )
         {
         pimgbtn->flState = IBS_DOWN | IBS_CAPTURE;
         WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
         }
      }
             /* Send the WM_CONTROLPOINTER message to   the   */
             /* owner   of the control to allow   the owner of   */
             /* control to change the   pointer   shape from the   */
             /* current defined arrow   shape         */

       WinSetPointer(HWND_DESKTOP,
           (HPOINTER)WinSendMsg(pimgbtn->hwndOwner,
                 WM_CONTROLPOINTER,
                 MPFROMSHORT(pimgbtn->id),
                 MPFROMLONG(pimgbtn->hptrArrow)));

       if (!(pimgbtn->flState & IBS_CAPTURE) && !( pimgbtn->flState & IBS_DISABLED))
       {
          if (isTipActive())
          {
             if (!WinPtInRect((HAB)NULL, &pimgbtn->rclHitTest, &ptl))
             {
                delTip();
             }
          }
          else
          {
             newTip(hWnd,pimgbtn->szTipText);
          }
       }                                         /* @pr6 ends */
       break;

       break;

   /*********************************************************************/
   /*  DBCS character conversion requested            */
   /*********************************************************************/

   case   WM_QUERYCONVERTPOS :

             /* State   that conversion   should not occur   */

       return(MRFROMSHORT(QCP_NOCONVERT));

   /*********************************************************************/
   /*  Match mnemonic                     */
   /*********************************************************************/

   case   WM_MATCHMNEMONIC :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Check   to see if a mnemonic symbol contained   */
             /* within the control text         */

       if ( pch   = strchr(pimgbtn->szText, '~') )
      {
             /* Mnemonic symbol found   within the text, get   */
             /* the character   and see   if the character   */
             /* matches that typed by   the user after      */
             /* converting any lower case characters to   */
             /* upper   case               */
      ++pch;
      if (   (ch = CHAR1FROMMP(mp1))   > '`' )
          if ( ch < '{' )
         ch -= ' ';

             /* Check   to see if the mnemonic matches and   */
             /* if the case, indicate   a match   through   the   */
             /* return value which will cause   the BM_CLICK   */
             /* message to be   sent to   the control      */

      if (   *pch ==   ch )
          return(MRFROMLONG(TRUE));
      }
       break;

   /*********************************************************************/
   /*  Button Click                     */
   /*********************************************************************/

   case   BM_CLICK :
             /* The message is received only when the   control   */
             /* is processing   the mnemonic selection of the   */
             /* control.  The   message   is received since the   */
             /* control was designated as being a push button   */
             /* through the WM_QUERYDLGCODE.         */

             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

       pimgbtn->flState   = IBS_DOWN;
       WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);

             /* Post a message to the   main window using the   */
             /* command message value   to inform the control   */
             /* owner   that the button   has been selected   */

       WinSendMsg(pimgbtn->hwndOwner, WM_COMMAND,
        MPFROMSHORT(pimgbtn->id),
        MPFROM2SHORT(FALSE, CMDSRC_OTHER));

             /* Invalidate the button   image to update   it from   */
             /* down position               */

       pimgbtn->flState   = IBS_UP;
       WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   6: Control painting and   appearance coding         */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Erase background                     */
   /*********************************************************************/

   case   WM_ERASEBACKGROUND :

             /* Have OS/2 Presentation Manager perform the   */
             /* background erase on behalf of   the button   */

       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Paint the button                     */
   /*********************************************************************/

   case   WM_PAINT :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		 rcl = pimgbtn->rcl;
		 GpiCreateLogColorTable(hPS = WinBeginPaint(hWnd,   (HPS)NULL, (PRECTL)NULL),
               0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

             /* Get the presentation space for the control   */
             /* and fill the button area of the control with   */
             /* the colour for the button         */

       if ( pimgbtn->flState & IBS_DISABLED )                     /*@ch2*/
          WinFillRect(hPS,   &rcl, 0x00FFFFFF);   /*@ch2*/
       else                                                       /*@ch2*/
          WinFillRect(hPS,   &rcl, pimgbtn->lButtonFace);

             /* Get the destenation point and   the bitmap   */
             /* rectangle to allow the bitmap   to be correctly   */
             /* positioned within the   control         */
      rcl = pimgbtn->rcl;

             /* Check   the state of the control such that the   */
             /* control is correctly offset         */

      if ( pimgbtn->flState & IBS_DOWN   )
      {
		  ptl = pimgbtn->ptlDestDown;
		  hbm = pimgbtn->hbmDown;
                  WinDrawBitmap(hPS, hbm, NULL, (PPOINTL)&rcl, 0L, 0L, DBM_IMAGEATTRS | DBM_STRETCH); //@hack
		  WinDrawBorder(hPS, &rcl,1L,1L,0L, 0L, DB_DEPRESSED);
    }
      else
      {
	      if (   pimgbtn->flState & IBS_DISABLED   )
              {
			  hbm = pimgbtn->hbmDisabled;
			  ptl = pimgbtn->ptlDestDisabled;
              }
	      else
              {
			  hbm = pimgbtn->hbm;
			  ptl = pimgbtn->ptlDest;
              }
/*@prjdk      WinDrawBorder(hPS, &rcl,1L,1L,0L, 0L, DB_RAISED);*/
              WinDrawBitmap(hPS, hbm, NULL, (PPOINTL)&rcl, 0L, 0L, DBM_IMAGEATTRS | DBM_STRETCH); //@hack
      }
     /* Draw the bitmap within the control.  Use the   */
     /* appropriate bitmap depending on whether or   */
     /* not the bitmap is diabled.         */

//@HACK HACK       WinDrawBitmap(hPS, hbm, &rcl, &ptl, 0L, 0L, DBM_IMAGEATTRS);
      

       /* Release the presentation space      */
       WinEndPaint(hPS);
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   7: Control defined message coding            */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control Defined Message:    Load bitmaps            */
   /*********************************************************************/

   case   IM_LOADBITMAPS :
       if ( !(pib = (PIMAGEBUTTON)PVOIDFROMMP(mp1)) )
      return(MRFROMLONG(TRUE));

       LoadBitmaps((PIMAGEBUTTON)PVOIDFROMMP(mp1),
         (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP),
         (HMODULE)LONGFROMMP(mp2));
       WinQueryWindowRect(hWnd,   &rcl);

       SizeButton(hWnd,   rcl.xLeft, rcl.yBottom,
        (rcl.xRight -   rcl.xLeft),
        (rcl.yTop   -   rcl.yBottom));
       break;

   /*********************************************************************/
   /*  Control Defined Message:    Reset state            */
   /*********************************************************************/

   case   IM_RESETBTN :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

       if ( pimgbtn->flState & IBS_DISABLED )
      pimgbtn->flState = IBS_DISABLED;
       else
      pimgbtn->flState = 0;

       WinInvalidateRect(hWnd, NULL, FALSE);
       break;

   /*********************************************************************/
   /*  Control Defined Message:    Reset state            */
   /*********************************************************************/

   case   IM_SELECTBTN :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

       if ( pimgbtn->flState & IBS_DISABLED )
      pimgbtn->flState = IBS_DISABLED | SHORT1FROMMP(mp1);
       else
      pimgbtn->flState = SHORT1FROMMP(mp1);

       WinInvalidateRect(hWnd, NULL, FALSE);
       break;

   /*********************************************************************/
   /*  Control Defined Message:    Button   click simulation      */
   /*********************************************************************/

   case   IM_BTNCLK :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

       if ( pimgbtn->flState & IBS_DISABLED )
      return(0L);
       else
      {
      if (   pimgbtn->flState & IBS_SET )
          pimgbtn->flState   = IBS_UP;
      else
          pimgbtn->flState   = IBS_SET;

      WinSendMsg(pimgbtn->hwndOwner, WM_CONTROL,
            MPFROM2SHORT(pimgbtn->id,   IBN_SELECT),
            MPFROMSHORT(pimgbtn->flState & IBS_SET ? TRUE : FALSE));

             /* Invalidate the button   image to update   it from   */
             /* down position               */

      WinInvalidateRect(hWnd, (PRECTL)NULL, TRUE);
      }
       break;

   /*********************************************************************/
   /*  Control Defined Message:    Set Bitmap image         */
   /*********************************************************************/

   case   IM_SETBITMAP :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

       switch (   LONGFROMMP(mp1)   )
      {
      case   IBP_UP :
          if ( pimgbtn->hbm )
         GpiDeleteBitmap(pimgbtn->hbm);

          pimgbtn->hbm = (HBITMAP)LONGFROMMP(mp2);
          GpiQueryBitmapParameters(pimgbtn->hbm, &bmpi);
          pimgbtn->cx = bmpi.cx;
          pimgbtn->cy = bmpi.cy;
          break;

      case   IBP_DOWN :
          if ( pimgbtn->hbmDown )
         GpiDeleteBitmap(pimgbtn->hbmDown);

          pimgbtn->hbmDown   = (HBITMAP)LONGFROMMP(mp2);
          GpiQueryBitmapParameters(pimgbtn->hbmDown, &bmpi);
          pimgbtn->cxDown = bmpi.cx;
          pimgbtn->cyDown = bmpi.cy;
          break;

      case   IBP_DISABLED :
          if ( pimgbtn->hbmDisabled )
         GpiDeleteBitmap(pimgbtn->hbmDisabled);

          pimgbtn->hbmDisabled = (HBITMAP)LONGFROMMP(mp2);
          GpiQueryBitmapParameters(pimgbtn->hbmDisabled, &bmpi);
          pimgbtn->cxDisabled = bmpi.cx;
          pimgbtn->cyDisabled = bmpi.cy;
          break;

      default :
          return(MRFROMLONG(TRUE));
      }


       SizeButton(hWnd,   rcl.xLeft, rcl.yBottom,
        (rcl.xRight -   rcl.xLeft),
        (rcl.yTop   -   rcl.yBottom));
       break;

/************************************************************************/
/************************************************************************/
/*                           */
/* Part   8: Control destruction coding               */
/*                           */
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Window being destroyed, perform clean-up            */
   /*********************************************************************/

   case   WM_DESTROY :
             /* Get the address of the control info from the   */
             /* control's reserved memory                     */

       pimgbtn = (PIMGBTN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

             /* Delete the bitmap images of the buttons   */

       GpiDeleteBitmap(pimgbtn->hbm);
       GpiDeleteBitmap(pimgbtn->hbmDown);
       GpiDeleteBitmap(pimgbtn->hbmDisabled);

             /* Release the memory allocated for use by the   */
             /* control               */

       DosFreeMem((PVOID)pimgbtn);
       break;
             /* Default message processing         */
   default :
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}
