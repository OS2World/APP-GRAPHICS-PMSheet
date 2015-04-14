#define UACT_SKIP           0x0001
#define UACT_OVERWRITE      0x0002
#define UACT_NEWITEM        0x0003

#define SEL_NONE            0x0000
#define SEL_FORWARD         0x0001
#define SEL_BACKWARD        0x0002

#define SORT_NAME           0x0001
#define SORT_SIZE           0x0002

class picturelist;
class hPicture;



typedef struct _loaderdef
{
   HWND hwndClient; /* for posting status */
   HAB  hab;
   char szPath[CCHMAXPATH];
   picturelist *pList;
   hPicture    *pPic;
   int cxSheet;
   int cySheet;
   int iPicSize;
}LOADERDEF;


typedef struct _dropcopyblock
{
    LOADERDEF loader;                   /* uptil here it is loaderdef!! */
    PDRAGINFO pDraginfo;
    unsigned  int *pAction; 
    char     *pszTargetFolder;
    int       iRot;
} dropcopyblock;

class hPicture
{
  friend class picturelist;
public:
   virtual ~hPicture();
   hPicture( char *pszfile,char *psz, int index );

   char * getFileName( void ) { return m_szFilename; }
   char * getShortName( void ) { return m_szShortname; }
   PBITMAPINFOHEADER2 getBitmapInfoheader( void );
   void setBitmapInfoheader( PBITMAPINFOHEADER2 p );
   void setImageData( PBYTE pcData );
   void setDetailBitmapInfoheader( PBITMAPINFOHEADER2 p );
   PBITMAPINFOHEADER2 getDetailBitmapInfoheader( void );

   void  setDetailImageData( PBYTE pcData );
   PBYTE getDetailImageData( void );

   void freeDetailImage(void);
   void freeImage(void);
   PBYTE getImageData( void );
   int  getIndex( void ) { return m_index; }
   void setIndex( int i) { m_index = i;    }
   void setImageRect(RECTL rcl);
   void getImageRect(RECTL *rcl);
   int  getHeight( void );
   int  getWidth ( void );
   int  getDetailWidth ( void );
   int  getDetailHeight( void );
   hPicture * pointInPicture(POINTL *pptl);
   void selectPicture(HWND hwnd,BOOL bRefresh, BOOL bSelect = TRUE);
   BOOL isSelected( void ){ return m_bSelected; }
   void loadPicRect(RECTL *prclImage);
   BOOL paint(HPS hps,RECTL *prclInvalidate, BOOL bShowName);
   void paintDetail(HPS hps,RECTL *prclWnd);
   HBITMAP pict2BitmapHandle( void );
   void setImageInfo(PBITMAPINFOHEADER2 pbmp2);
   static long m_lCharHeight;
   static HPS   m_hpsMem;

private:
   hPicture *next;
   hPicture *previous;
   char      m_szFilename[CCHMAXPATH];
   char      m_szShortname[CCHMAXPATH];
   PBITMAPINFOHEADER2 m_pbmp2;
   PBYTE    m_imgData;
   int      m_index;
   BOOL     m_bSelected;
   RECTL    m_rcl;
   /*
   ** following ptr's are only filled on detail view
   */
   PBITMAPINFOHEADER2 m_detailpbmp2;
   PBYTE              m_detailpImgData;
   /*
   ** For status info and sorting
   */
   unsigned int m_cxPixels;     /* show in props */
   unsigned int m_cyPixels;     /* show in props */
   unsigned int m_imgDataSize;  /* sort on size! */
  
};
/*------------------------------------------------------------------------*/
class picturelist
{
public:
      picturelist(void);
      virtual ~picturelist();
      void     addPicture(hPicture *pPicture);
      void     remPicture(hPicture *pPicture);
      BOOL     deletePicture( char *pszShortName );
      void delSelectedFromDisk(HWND hwnd);
      hPicture * getFirstPicture( void );
      hPicture * getNextPicture( hPicture *pPict );
      hPicture * getPrevPicture( hPicture *pPict );
      hPicture * selectPicture(HWND hwnd,hPicture *pCursor,POINTL *ptlMouse, BOOL bShift);
      hPicture * selectNextPicture(hPicture *pPict, HWND hwnd, BOOL bShift);
      hPicture * selectPrevPicture(hPicture *pPict, HWND hwnd, BOOL bShift);
      hPicture * selectNextRow(hPicture *pPict, HWND hwnd, BOOL bShift);
      hPicture * selectPrevRow(hPicture *pPict, HWND hwnd, BOOL bShift);

      hPicture * selectLastPicture(hPicture *pPict, HWND hwnd, BOOL bShift);          /* VK_END */
      hPicture * selectFirstPicture(hPicture *pPict, HWND hwnd, BOOL bShift);         /* VK_HOME*/
      hPicture * selectSinglePicture(HWND hwnd,hPicture *pCursor, POINTL *pptlMouse); /* on cntrl down */

      int countSelected( void );                     // used for init drag
      hPicture * getNextSelected(hPicture * hPict); // with null gives first
      void formatFolderString(HWND hwnd);            // On cancelation folderstring generated in sheet

      void     selectAllPicture(HWND hwnd,BOOL bSelect,BOOL bSilent);
      void     reCalcPictureRect(HWND hControl,int slideSize, int cxSheet, int cySheet);
      int      calcRowCount(int slideSize, int cxSheet);
      int      getRows( void ) { return m_Rows; }
      HBITMAP  pict2BitmapHandle( void );
      BOOL     loadImages(hPicture *pPict, HAB hab, HWND hwnd);
      hPicture *findPicture(char *pszName);
      BOOL     sortPicture(int iSort);
      void     deleteNonLoaded( void );

      void     rotateSelected(HWND hwnd, int degrees);                                    // RotateSelected imgages

private:
      BOOL sortOnSize(int iCount, hPicture **pArray );
      BOOL sortOnName(int iCount, hPicture **pArray );
      hPicture * pictureAtPoint(POINTL *pptlMouse);
      void selectFromPinTo(HWND hwnd, hPicture *pC);
      BOOL hasPrevRow(hPicture *pPict);
      BOOL hasNextRow(hPicture *pPict);
      hPicture *plistBase; /* Start of the list */
      hPicture *plistEnd;
      hPicture *pCursor;
      BOOL  m_bSortSize; /* ascending/descending */
      BOOL  m_bSortName; /* ascending/descending */
      int   m_Rows;
      int   m_iColumns;  /* just for cursor down/up movements */
      hPicture *m_pinPoint; /* multiselect anchor point */
      int   m_iDirection;

};

extern "C"
{
     void  _System picCreateList(LOADERDEF *pcdef );
     TID   createPictureList( HWND hwndClient , HAB hab, char *pszPath, int cx, int cy, int i );
     TID   continueOnError( HWND hwndClient,HAB hab, picturelist *pList );
     void _System continueList( LOADERDEF *pcdef );
     void _System loadImageDetail(LOADERDEF *pcdef );
     TID  loadDetailForImage( HWND hwndClient,HAB hab, hPicture *pPict );
     void _System dropCopyThread( dropcopyblock * p);
     void _System rotateSelectedThread (dropcopyblock *pBlock);
}

void cancelLoad( BOOL bCancel);
void cancelDetailLoad(BOOL bCancel);