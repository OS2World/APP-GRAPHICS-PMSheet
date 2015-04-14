/*---------------------------------------------------------------------------*/
/*  Name: tip.h                                                              */
/*                                                                           */
/*  Description : Contains c-wrapper functions for generic tooltip.          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#define MAX_TIPTEXT    50

#ifdef __cplusplus
      extern "C" {
#endif

MRESULT EXPENTRY ToolTipWndProc(HWND,ULONG,MPARAM,MPARAM);
void delTip( void );
BOOL newTip(HWND hParent,PSZ pszText);
BOOL isTipActive(void);
void registerTooltip(HAB hab);

#ifdef __cplusplus
}
#endif
