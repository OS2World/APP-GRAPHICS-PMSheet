/*

MODEL.C  Work on the model used in GbmV2

*/
#ifdef __cplusplus
      extern "C" {
#endif

#ifndef MODEL_H
#define   MODEL_H

#include "gbm.h"

/* These needs to encompass all the GBM errors, plus some extras */
typedef int MOD_ERR;
#define   MOD_ERR_OK   ((MOD_ERR) 0)
#define   MOD_ERR_MEM   ((MOD_ERR) 1)
#define   MOD_ERR_OPEN   ((MOD_ERR) 2)
#define   MOD_ERR_CREATE   ((MOD_ERR) 3)
#define   MOD_ERR_SUPPORT   ((MOD_ERR) 4)
#define   MOD_ERR_HDC   ((MOD_ERR) 5)
#define   MOD_ERR_HPS   ((MOD_ERR) 6)
#define   MOD_ERR_HBITMAP   ((MOD_ERR) 7)
#define   MOD_ERR_HMF   ((MOD_ERR) 8)
#define   MOD_ERR_CLIP   ((MOD_ERR) 9)
#define   MOD_ERR_GBM(rc)   ((MOD_ERR) 0x8000+(rc))

typedef struct
   {
   GBM gbm;
   GBMRGB gbmrgb[0x100];
   BYTE *pbData;
   } MOD;

extern MOD_ERR ModCreate(
   int w, int h, int bpp, const GBMRGB gbmrgb[],
   MOD *modNew
   );

extern MOD_ERR ModDelete(MOD *mod);


extern MOD_ERR ModMove(MOD *mod, MOD *modNew);

extern MOD_ERR ModCreateFromFile(
   CHAR *szFn, CHAR *szOpt,
   MOD *modNew
   );

extern MOD_ERR ModWriteToFile(
   MOD *mod,
   CHAR *szFn,CHAR *szOpt
   );

extern BOOL imgRotate(IMAGE * pImg,int degrees);

/*...scolour adjustment mappings:0:*/
#define   CVT_I_TO_L   0
#define   CVT_I_TO_P   1
#define   CVT_L_TO_I   2
#define   CVT_L_TO_P   3
#define   CVT_P_TO_I   4
#define   CVT_P_TO_L   5
/*...e*/

extern MOD_ERR ModColourAdjust(
   const MOD *mod,
   int map, double gama, double shelf,
   MOD *modNew
   );

/*...sbpp palette and algorithm mappings:0:*/
#define   CVT_BW      0
#define   CVT_VGA      1
#define   CVT_8      2
#define   CVT_4G      3
#define   CVT_784      4
#define   CVT_666      5
#define   CVT_8G      6
#define   CVT_TRIPEL   7
#define   CVT_RGB      8
#define   CVT_FREQ   9
#define   CVT_MCUT   10

#define   CVT_NEAREST   0
#define   CVT_ERRDIFF   1
#define   CVT_HALFTONE   2
/*...e*/


#endif

#ifdef __cplusplus
}
#endif
