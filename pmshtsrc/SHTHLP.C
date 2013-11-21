/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
#define  INCL_WIN
#include <os2.h>
#include <string.h>
#include "resource.h"
#define STYLE_HELP_TABLE                    1000
#define HELPLIBRARYNAMELEN  20

/*  Global variables                                            */
static HWND hwndHelpInstance;
static CHAR szLibName[HELPLIBRARYNAMELEN];
static CHAR szWindowTitle[HELPLIBRARYNAMELEN];
void fnErrMsgBox(HWND hwnd, ULONG ErrId);
void DisplayHelpPanel(SHORT idPanel);
BOOL fHelpEnabled;
static HWND hwndFrame,hwndClient;
static HAB  habMain;
/*------------------------------------------------------------------------*/
/*  Name: InitHelp                                                        */
/*                                                                        */
/*  Description : Initializes the IPF help facility                       */
/*                                                                        */
/*  Concepts : Initializes the HELPINIT structure and creates the         */
/*             help instance.  If successful, the help instance           */
/*             is associated with the main window                         */
/*                                                                        */
/*  API's : WinCreateHelpInstance                                         */
/*          WinAssociateHelpInstance                                      */
/*          WinLoadString                                                 */
/*                                                                        */
/*  Parameters : None                                                     */
/*                                                                        */
/*  Returns:  None                                                        */
/*                                                                        */
/*------------------------------------------------------------------------*/
BOOL InitHelp(HAB hab, HWND hwndFrm,HWND hwndClnt)
{
  HELPINIT helpInit;

  /* if we return because of an error, Help will be disabled                */
  fHelpEnabled = FALSE;

  hwndFrame = hwndFrm;
  hwndClient= hwndClnt;
  habMain   = hab;

  /* inititalize help init structure                                        */
  helpInit.cb = sizeof(HELPINIT);
  helpInit.ulReturnCode = 0L;

  helpInit.pszTutorialName = (PSZ)NULL;   /* if tutorial added,             */
                                          /* add name here                  */
  helpInit.phtHelpTable = (PHELPTABLE)MAKELONG(STYLE_HELP_TABLE,
                           0xFFFF);
  helpInit.hmodHelpTableModule = (HMODULE)0;
  helpInit.hmodAccelActionBarModule = (HMODULE)0;
  helpInit.idAccelTable = 0;
  helpInit.idActionBar = 0;

  if (!WinLoadString(habMain, (HMODULE)0, IDS_HELPWINDOWTITLE,
                    HELPLIBRARYNAMELEN, (PSZ)szWindowTitle))
  {
     fnErrMsgBox(hwndClient, IDS_CANNOTLOADSTRING);
     return FALSE;
  }
  helpInit.pszHelpWindowTitle = (PSZ)szWindowTitle;
  helpInit.fShowPanelId = CMIC_HIDE_PANEL_ID;

  if (!WinLoadString(habMain, (HMODULE)0, IDS_HELPLIBRARYNAME,
       HELPLIBRARYNAMELEN, (PSZ)szLibName))
  {
     fnErrMsgBox(hwndClient, IDS_CANNOTLOADSTRING);
     return FALSE;
  }

  helpInit.pszHelpLibraryName = (PSZ)szLibName;

  /* creating help instance                                                 */
  hwndHelpInstance = WinCreateHelpInstance(habMain, &helpInit);

  if (!hwndHelpInstance || helpInit.ulReturnCode)
  {
     fnErrMsgBox(hwndFrame, IDS_HELPLOADERROR);
     return FALSE;
  }

  /* associate help instance with main frame                                */
  if (!WinAssociateHelpInstance(hwndHelpInstance, hwndFrame))
  {
     fnErrMsgBox(hwndFrame, IDS_HELPLOADERROR);
     return FALSE;
  }

  /* help manager is successfully initialized so set flag to TRUE           */
  fHelpEnabled = TRUE;
  return TRUE;

}   /*                                          End of InitHelp()           */

/*********************************************************************
 *  Name: HelpUsingHelp
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Using Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Using Help menu item
 *             is selected.  Sends an HM_DISPLAY_HELP message to the
 *             help instance so that the default Using Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpUsingHelp(MPARAM mp2)
{
/*    this just displays the system help for help panel                       */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_DISPLAY_HELP, NULL, NULL))
    {
          fnErrMsgBox(hwndFrame, IDS_HELPLOADERROR);
    }
  }
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.
  */
  mp2;
}   /* End of HelpUsingHelp()                                                */
/*---------------------------------------------------------------------------*/
VOID  HelpGeneral(MPARAM mp2)
{
/*   this just displays the system General help panel                       */
   if (fHelpEnabled)
   {
     if ((BOOL)WinSendMsg(hwndHelpInstance, HM_EXT_HELP, NULL, NULL))
     {
        fnErrMsgBox(hwndFrame, IDS_HELPGENERALERROR);
     }
   }
   /*
    * This routine currently doesn't use the mp2 parameter but
    * it is referenced here to prevent an 'Unreferenced Parameter'
    * warning at compile time.
    */
    mp2;

}       /* End of HelpGeneral()                                      */

/*********************************************************************
 *  Name: HelpKeys
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Keys Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Keys Help menu item
 *             is selected.  Sends an HM_KEYS_HELP message to the
 *             help instance so that the default Extended Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpKeys(MPARAM mp2)

{
/* this just displays the system keys help panel                              */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_KEYS_HELP, NULL, NULL))
    {
      fnErrMsgBox(hwndFrame, IDS_HELPKEYSERROR);
    }
  }
/*
 * This routine currently doesn't use the mp2 parameter but
 * it is referenced here to prevent an 'Unreferenced Parameter'
 * warning at compile time.
 */
  mp2;
}   /*   End of HelpKeys()                                                    */

/*********************************************************************
 *  Name: HelpIndex
 *
 *  Description : Processes the WM_COMMAND message posted
 *                by the Index Help item of the Help menu
 *
 *  Concepts : Called from MainCommand when the Index Help menu item
 *             is selected.  Sends an HM_INDEX_HELP message to the
 *             help instance so that the default Extended Help is
 *             displayed.
 *
 *  API's : WinSendMsg
 *
 *  Parameters :  mp2 - Message parameter 2
 *
 *  Returns: None
 *
 ****************************************************************/
VOID  HelpIndex(MPARAM mp2)
{
/* this just displays the system help index panel                             */
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_HELP_INDEX, NULL, NULL))
    {
       fnErrMsgBox(hwndClient, IDS_HELPINDEXERROR);
    }
  }
 /*
  * This routine currently doesn't use the mp2 parameter but
  * it is referenced here to prevent an 'Unreferenced Parameter'
  * warning at compile time.

  */
  mp2;
}   /* End of HelpIndex()*/
/*********************************************************************
 *  Name: DestroyHelpInstance
 *
 *  Description : Destroys the help instance for the application
 *
 *  Concepts : Called after exit from message loop. Calls

 *             WinDestroyHelpInstance() to destroy the
 *             help instance.
 *
 *  API's : WinDestroyHelpInstance
 *
 *  Parameters : None

 * None
 *  Returns:
 *
 ****************************************************************/
VOID DestroyHelpInstance(VOID)
{
  if ((BOOL)hwndHelpInstance)
  {
     WinDestroyHelpInstance(hwndHelpInstance);
  }

}   /*        End of DestroyHelpInstance()                               */

/*-----------------------------------------------------------------------*/
/*  Name : ShowDlgHelp                                                   */
/*                                                                       */
/*  Description : Displays the help panel for the current selected       */
/*                item in the dialog window                              */
/*                                                                       */
/*  Concepts : Called each time a WM_HELP message is posted to a         */
/*             dialog gets the id value of the window and determine      */
/*             which help panel to display.  Then sends a message        */
/*             to the help instance to display the panel.  If the        */
/*             dialog or item is not included here, then the             */
/*             unknown dialog or unknown item panel is displayed.        */
/*                                                                       */
/*  Parameters : hwnd - Handle of the dialog box                         */
/*                                                                       */
/*  Returns: Void                                                        */
/*-----------------------------------------------------------------------*/
VOID ShowDlgHelp(HWND hwnd)
{
   USHORT idPanel, idDlg, idItem;
   HWND hwndFocus;

   idPanel = 0;
   /*
   * Get the id of the dialog box
   */
   idDlg = WinQueryWindowUShort(hwnd, QWS_ID);
   /*
    * Finds which window within the dialog has the focus and gets its id
    */
   hwndFocus = WinQueryFocus(HWND_DESKTOP);
   idItem = WinQueryWindowUShort(hwndFocus, QWS_ID);

   switch (idDlg)
   {
//      case IDD_TXT:
//         idPanel = PANEL_TEXTOBJECT;
//         break;
      default:
         idPanel = 0;
   }
  
   DisplayHelpPanel(idPanel);
}
/*-------------------------------------------------------------------------*/
/*  Name: DisplayHelp                                                      */
/*                                                                         */
/*  Description : Displays the help panel indicated                        */
/*                                                                         */
/*  Concepts : Displays the help panel whose id is passed to               */
/*             the routine.  Called whenever a help panel is               */
/*             desired to be displayed, usually from the                   */
/*             WM_HELP processing of the dialog boxes.                     */
/*                                                                         */
/*  API's : WinSendMsg                                                     */
/*                                                                         */
/*  Parameters : idPanel - Id of the halp panel to be displayed            */
/*                                                                         */
/*  Returns: None                                                          */
/*-------------------------------------------------------------------------*/
void DisplayHelpPanel(SHORT idPanel)
{
  if (fHelpEnabled)
  {
    if ((BOOL)WinSendMsg(hwndHelpInstance, HM_DISPLAY_HELP,
            MPFROM2SHORT(idPanel, NULL), MPFROMSHORT(HM_RESOURCEID)))
    {
      fnErrMsgBox(hwndFrame, IDS_HELPDISPLAYERROR);
    }
  }

}   /*    End of DisplayHelpPanel()                                   */

/*------------------------------------------------------------------------*/
/* Function : fnErrMsgBox                                                 */
/*                                                                        */
/*            Displays the errormessage which is loaded from the resource */
/*------------------------------------------------------------------------*/
void fnErrMsgBox(HWND hwnd, ULONG ErrId)
{
  char szErrmsg[50];

  WinLoadString(habMain,(HMODULE)0,ErrId,sizeof(szErrmsg),(PSZ)szErrmsg);
  WinMessageBox(HWND_DESKTOP,
                HWND_DESKTOP,
        	szErrmsg,
		"Error",
                0,
                MB_CUACRITICAL | MB_SYSTEMMODAL | MB_OK);
}
