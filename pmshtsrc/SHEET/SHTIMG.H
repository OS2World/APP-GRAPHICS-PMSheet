/*------------------------------------------------------------------------*/
/* Image formats plus imagestruct definitions.                            */
/*------------------------------------------------------------------------*/

#define IFMT_UNKNOWN  0
#define IFMT_DIB      1
#define IFMT_X11      2
#define IFMT_META     3
#define IFMT_GIF      4
#define IFMT_ICO      5
#define IFMT_BMP2     6      // OS/2 2.0 Bitmaps, Icons & Pointers
#define IFMT_TIFF     7
#define IFMT_AUTO     9

#define  IMG_OK               0
#define  IMG_OUTOFMEMORY      1
#define  IMG_WRONGFORMAT      2
#define  IMG_NOTIMPLEMENTED   3

#define  SZIMG_START          "<IMAGE>"
#define  SZIMG_END            "<END>"


typedef struct _image
{
    USHORT   nColors;
    USHORT   uslayer;
    float    cx;
    float    cy;
    float    cxdest;
    float    cydest;
    float    x;
    float    y;
    ULONG    ImageDataSize;
    ULONG    filesize;
    PBITMAPINFOHEADER2 pbmp2;
    unsigned char * ImgData;

} IMAGE, *pImage,*pDrwImage;

/*
** drwjpg.c
*/




#ifdef DRWIMG

typedef int MOD_ERR;
#define	MOD_ERR_OK	((MOD_ERR) 0)
#define	MOD_ERR_MEM	((MOD_ERR) 1)
#define	MOD_ERR_OPEN	((MOD_ERR) 2)
#define	MOD_ERR_CREATE	((MOD_ERR) 3)
#define	MOD_ERR_SUPPORT	((MOD_ERR) 4)
#define	MOD_ERR_HDC	((MOD_ERR) 5)
#define	MOD_ERR_HPS	((MOD_ERR) 6)
#define	MOD_ERR_HBITMAP	((MOD_ERR) 7)
#define	MOD_ERR_HMF	((MOD_ERR) 8)
#define	MOD_ERR_CLIP	((MOD_ERR) 9)
#define	MOD_ERR_GBM(rc)	((MOD_ERR) 0x8000+(rc))

typedef struct
	{
	GBM gbm;
	GBMRGB gbmrgb[0x100];
	BYTE *pbData;
	} MOD;

int ModMakeHBITMAP(MOD *mod, POBJECT pObj);
int ModCreateFromFile(CHAR *szFn, CHAR *szOpt,MOD *modNew);

/*...sbpp palette and algorithm mappings:0:*/
#define	CVT_BW		0
#define	CVT_VGA		1
#define	CVT_8		2
#define	CVT_4G		3
#define	CVT_784		4
#define	CVT_666		5
#define	CVT_8G		6
#define	CVT_TRIPEL	7
#define	CVT_RGB		8
#define	CVT_FREQ	9
#define	CVT_MCUT	10

#define	CVT_NEAREST	0
#define	CVT_ERRDIFF	1
#define	CVT_HALFTONE	2
/*...e*/

#endif
