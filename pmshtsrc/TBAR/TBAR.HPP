#define CXBUTTON      42
#define CYBUTTON      42
#define TBAR_SPACE    5
#define MAXBUTTONS    10    /* Number of buttons in main bar.      */

#define IDBTN_SELECT  400
#define IDBTN_ROTLEFT 405
#define IDBTN_ROTRIGHT 406
#define IDBTN_ZOOM    410
#define IDBTN_DELETE  415
#define IDBTN_SMALL   420
#define IDBTN_MEDIUM  425
#define IDBTN_LARGE   430
#define IDBTN_SORTSIZE 435
#define IDBTN_SORTNAME 440
#define IDBTN_PROJECT  445

typedef struct _imgbtn      /* ibtn */
{                           /* Size:  16        Bytes                     */
   LONG        cb;          /* Structure        Size                      */
   LONG        idBitmap;    /* Bitmap ID        : Normal or Up Position   */
   LONG        idBitmapDown;/* Bitmap ID        : Down Position           */
   LONG        idBitmapDisabled; /* Bitmap ID        : Disabled           */
   LONG        idButton;    /* WINDOW ID used in the WM_COMMAND.          */
   HWND        hButton;     /* WINDOW handle of the Button.               */
} iButton;

class tbar
{
public:
    tbar(HAB hab , HWND hOwner, HWND hParent );
    ~tbar();

    void sizepos(long x,long y,long cx, long cy);


//    iButton m_imgbtn[MAXBUTTONS];
    HWND m_hTbar;             /* Main toolbar window handle           */
    long m_x;
    long m_y;
    long m_cy;
    long m_cx;
    HBITMAP m_hbm;
};


