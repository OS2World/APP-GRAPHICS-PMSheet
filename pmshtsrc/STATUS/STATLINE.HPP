#define MAX_CELLS  10


typedef struct _celldef
{
    char szText[150];
    RECTL rcl;
    int   iPercentage;
} celldef;


class statline
{
public:
    statline (HWND hwnd, int *iPercent, int iCells ); // array of percentages ( sum == 100%) and nr of cells in icount.
    ~statline ();

    void setSize    ( int cx, int cy );
    void setPos     ( int x,  int y  ){};
    void setCellText( int iCell, char *pszText );
    void paintCells ( HPS hps );

private:
    int m_cx;
    int m_cy;
    int m_xPos;
    int m_yPos;
    int m_iCells;
    HWND m_hParent;
    BOOL m_bSumError;
    celldef *m_pCell;    /* array of cells */
};
