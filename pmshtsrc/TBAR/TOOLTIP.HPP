/*---------------------------------------------------------------------------*/
/*  Name: tooltip.hpp                                                        */
/*                                                                           */
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
/*---------------------------------------------------------------------------*/
#define DLY_TOOLTIP    750              // timer delay
#define DLY_TIPDELETE  6000

class toolTip
{

friend MRESULT EXPENTRY ToolTipWndProc(HWND,ULONG,MPARAM,MPARAM);

public:
  toolTip(HWND hParent, char *text);
  virtual ~toolTip();
  void showToolTip(void);
  void ToolTip_OnPaint(void);
  HWND getWnd(void) { return m_hTip; }
  HWND getParent(void) { return m_hParent; }
private:
  HWND m_hTip;     /* Window handle of the tooltip window.        */
  HWND m_hParent;  /* parentwindow handle.                        */
  char *m_pszText;
  int  m_cx,m_cy;  /* width and hight of the tooltip.             */
  ULONG m_hfont;   /* font to be used. Can be NULL.               */
};

