
/* imagebtn.h	   Created:    1991-06-02  Revised:    1992-08-26	*/

/* Image Button	Custom Control Constants Header				*/

/* Copyright ¸ International Business Machines Corp., 1991,1992.	*/
/* Copyright ¸ 1989-1992  Prominare Inc.  All Rights Reserved.		*/

/* DISCLAIMER OF WARRANTIES:						*/
/* -------------------------						*/
/* The following [enclosed] code is sample code	created	by IBM		*/
/* Corporation and Prominare Inc.  This	sample code is not part	of any	*/
/* standard IBM	product	and is provided	to you solely for the purpose	*/
/* of assisting	you in the development of your applications.  The code	*/
/* is provided "AS IS",	without	warranty of any	kind.  Neither IBM nor	*/
/* Prominare shall be liable for any damages arising out of your	*/
/* use of the sample code, even	if they	have been advised of the	*/
/* possibility of such damages.						*/

#define	USER_RESERVED	   8	   /* Control Reserved Memory Size	*/

#define	QUCWP_WNDP (QWL_USER + 4)  /* Pointer to Internal Control Data	*/

#define	IDB_DEFAULT	     100   /* Default Bitmap ID			*/
#define	IDB_DEFAULTDOWN	     101   /* Default Bitmap ID			*/
#define	IDB_DEFAULTDISABLED  102   /* Default Bitmap ID			*/

#define	IBP_UP		       1   /* Button Position: Up		*/
#define	IBP_DOWN	       2   /* Button Position: Down		*/
#define	IBP_DISABLED	       3   /* Button Position: Disabled		*/

#define	IBN_SELECT  0x1000	   /* Button Pressed			*/
#define	IBN_DBLCLK  0x1001	   /* Button Double Clicked		*/
#define	IBN_DBLCLK2 0x1002	   /* Button Double Clicked		*/

#define	IBS_UP	       0x0001	   /* Button State:  Up			*/
#define	IBS_DOWN       0x0002	   /* Button State:  Down		*/
#define	IBS_DISABLED   0x0004	   /* Button State:  Disabled		*/
#define	IBS_SET	       0x0100	   /* Button State:  Set		*/
#define	IBS_CAPTURE    0x1000	   /* Button State:  Capture		*/

#define	IS_TEXTONBTN	0x00000001L/* Button Style: Text on Button	*/
#define	IS_TEXTBELOWBTN	0x00000002L/* Button Style: Text Below Button	*/

#define	IM_LOADBITMAPS	   (WM_USER + 500)
#define	IM_RESETBTN	   (WM_USER + 501)
#define	IM_SELECTBTN	   (WM_USER + 502)
#define	IM_BTNCLK	   (WM_USER + 503)
#define	IM_QUERYBITMAPS	   (WM_USER + 504)
#define	IM_SETBITMAP	   (WM_USER + 505)

/*
** Button ID which we use in the WM_COMMAND message
*/

#define SELECTBUTTON       400
#define TEXTBUTTON         401
#define CIRBUTTON          402
#define SQRBUTTON          403
#define LINBUTTON          404
#define FLINEBUTTON        405
#define TRIANGLEBUTTON     406
#define SPLINEBUTTON       407
#define FILBUTTON          408
#define FONTBUTTON         409
#define LAYERBUTTON        410
#define FORMCHANGE         411  /* used for reformatting existing fig  */
#define CHANGEOBJ          412  /* Put on object in front of the other */
#define ZOOMBUTTON         413
#define ROTATEBUTTON       414

/* end of the buttons */

extern MRESULT EXPENTRY ImageBtnWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);

