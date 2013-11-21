#ifdef __cplusplus
      extern "C" {
#endif

BOOL InitHelp(HAB hab,HWND hwndFrm,HWND hwndClnt);
VOID HelpUsingHelp(MPARAM mp2);
VOID HelpGeneral(MPARAM mp2);
VOID HelpKeys(MPARAM mp2);
VOID HelpIndex(MPARAM mp2);
VOID HelpTutorial(MPARAM mp2);
VOID HelpProdInfo(MPARAM mp2);
VOID DisplayHelpPanel(SHORT nPanelId);
VOID DestroyHelpInstance(VOID);
VOID ShowDlgHelp(HWND hwnd);

#ifdef __cplusplus
}
#endif

#define PANEL_HELP                          2910
#define PANEL_HELPHELPFORHELP               2920
#define PANEL_HELPEXTENDED                  2930
#define PANEL_HELPKEYS                      2940
#define PANEL_HELPINDEX                     2950
#define PANEL_HELPABOUT                     2960

#define PANEL_TEXTOBJECT                    3000
