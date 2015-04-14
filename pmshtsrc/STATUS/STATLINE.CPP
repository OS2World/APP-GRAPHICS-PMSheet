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

#include "statline.hpp"

#define DB_RAISED    0x0400	    /* the undocumented value's */
#define DB_DEPRESSED 0x0800     /* drawbox.                 */

statline::statline (HWND hwnd, int *iPercent, int iCells ) // array of percentages ( sum == 100%) and nr of cells in icount.
{
    int i;
    int sum;
    m_cx     = 0;
    m_cy     = 0;
    m_xPos   = 0;
    m_yPos   = 0;
    m_iCells = 0;
    m_pCell  = NULL;    /* array of cells */
    m_bSumError = FALSE;
    m_hParent = hwnd;

    if (iCells > 0 && iCells < MAX_CELLS )
    {
        sum = 0;
        for (i = 0; i < iCells; i++)
            sum += iPercent[i];

        if ( sum != 100 )
        {
            m_bSumError = TRUE;
            return;
        }
    }

    if (iCells > 0 && iCells < MAX_CELLS )
    {
        m_iCells = iCells;
        m_pCell = (celldef *)calloc(sizeof(celldef) * m_iCells,1);

        for (i = 0; i < m_iCells; i++)
            m_pCell[i].iPercentage = iPercent[i];
    }

}

statline::~statline ()
{
    if (m_pCell)
        free(m_pCell);
}

void statline::setSize( int cx, int cy )
{
    int i;
    int xPos = 0;
    int yPos = 0;
    int iPerc;
    RECTL *prcl;

    if (m_bSumError)
        return;

    m_cx     = cx;
    m_cy     = cy;

    for (i =0; i < m_iCells; i++)
    {
        prcl = &m_pCell[i].rcl;
        iPerc= m_pCell[i].iPercentage;

        prcl->xLeft  = xPos;
        prcl->xRight = xPos + ( (iPerc * cx)/100 );
        prcl->yBottom= 0;
        prcl->yTop   = m_cy;
        xPos += ( (iPerc * cx)/100 );
    }
}

void statline::paintCells ( HPS hps )
{
    int i;
    int xPos = 0;
    int yPos = 0;
    int iPerc;
    RECTL *prcl;
    RECTL rclText;

    if (m_bSumError)
        return;

    WinSetPresParam(m_hParent,PP_FONTNAMESIZE,(ULONG)strlen("8.Helv")+1,(void *)"8.Helv");
    for (i =0; i < m_iCells; i++)
    {
        prcl = &m_pCell[i].rcl;
        WinFillRect(hps,prcl,CLR_PALEGRAY);
        WinDrawBorder(hps, prcl,1L,1L,0L, 0L, DB_DEPRESSED);
        if (m_pCell[i].szText[0])
        {
            rclText = m_pCell[i].rcl;
            rclText.xLeft += 10;
            WinDrawText(hps,-1,(PCH)m_pCell[i].szText,&rclText,CLR_BLACK,CLR_PALEGRAY,
                        DT_LEFT | DT_VCENTER );
        }
    }
}

void statline::setCellText( int iCell, char *pszText )
{
    if (iCell > m_iCells || !pszText)
        return;

    printf("statline::setCellText %d %s\n",iCell,pszText );
    strcpy(m_pCell[iCell].szText,pszText);
    WinInvalidateRect(m_hParent,&m_pCell[iCell].rcl,TRUE);
}
