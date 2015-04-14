#define INCL_GPI
#define INCL_WIN
#include <os2.h>


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>


#include "standard.h"
#include "gbm.h"
#include "gbmerr.h"
#include "gbmht.h"
#include "gbmhist.h"
#include "gbmmcut.h"
#include "gbmmir.h"
#include "gbmtrunc.h"
#include "shtimg.h"
#include "Shtpic.h"

static int StrideOf(MOD *mod)
   {
   return ( ( mod->gbm.w * mod->gbm.bpp + 31 ) / 32 ) * 4;
   }
/*...e*/
/*...sAllocateData:0:*/

static BOOL AllocateData(MOD *mod)
   {
   int stride = StrideOf(mod);
   if ( (mod->pbData = calloc((stride * mod->gbm.h),1)) == NULL )
      return FALSE;
   return TRUE;
   }

static int Hbitmap2Mod(MOD *mod, pImage pImg)
{
   USHORT usColors;

   mod->gbm.w   = pImg->pbmp2->cx;
   mod->gbm.h   = pImg->pbmp2->cy;
   mod->gbm.bpp = pImg->pbmp2->cBitCount;

   if (mod->gbm.bpp == 24)
      usColors = 0;
   else
      usColors = (1<<mod->gbm.bpp);

   if ( mod->gbm.bpp != 24 )
   {
      int i;
      RGB2  *pRGB2;
      unsigned char *data;

      data = (unsigned char *)pImg->pbmp2;
      pRGB2 = (RGB2 *)(data + sizeof(BITMAPINFOHEADER2));

      for ( i = 0; i < usColors; i++ )
      {
         mod->gbmrgb[i].r = pRGB2->bRed;
         mod->gbmrgb[i].g = pRGB2->bGreen;
         mod->gbmrgb[i].b = pRGB2->bBlue;
         pRGB2->fcOptions = 0;
         pRGB2++;
      }
   }
   mod->pbData = pImg->ImgData;
   return 0;
}

static MOD_ERR ModCopy(MOD *mod, MOD *modNew)
   {
   modNew->gbm.w   = mod->gbm.w;
   modNew->gbm.h   = mod->gbm.h;
   modNew->gbm.bpp = mod->gbm.bpp;
   if ( mod->gbm.bpp != 24 )
      memcpy(modNew->gbmrgb, mod->gbmrgb, sizeof(GBMRGB) << mod->gbm.bpp);

   if ( !AllocateData(modNew) )
      return MOD_ERR_MEM;
   memcpy(modNew->pbData, mod->pbData, StrideOf(mod) * mod->gbm.h);
   return MOD_ERR_OK;
   }


static MOD_ERR ModTranspose(MOD *mod, MOD *modNew)
   {
   modNew->gbm.w   = mod->gbm.h;
   modNew->gbm.h   = mod->gbm.w;
   modNew->gbm.bpp = mod->gbm.bpp;
   if ( mod->gbm.bpp != 24 )
      memcpy(modNew->gbmrgb, mod->gbmrgb, sizeof(GBMRGB) << mod->gbm.bpp);
   if ( !AllocateData(modNew) )
      return MOD_ERR_MEM;
   gbm_transpose(&(mod->gbm), mod->pbData, modNew->pbData);
   return MOD_ERR_OK;
   }

/*...sModReflectHorz:0:*/
static MOD_ERR ModReflectHorz(MOD *mod, MOD *modNew)
   {
   MOD_ERR mrc;
   if ( (mrc = ModCopy(mod, modNew)) != MOD_ERR_OK )
      return mrc;
   if ( !gbm_ref_horz(&(modNew->gbm), modNew->pbData) )
      {
      free(modNew->pbData);
      return MOD_ERR_MEM;
      }
   return MOD_ERR_OK;
   }

static MOD_ERR ModReflectVert(MOD *mod, MOD *modNew)
   {
   MOD_ERR mrc;
   if ( (mrc = ModCopy(mod, modNew)) != MOD_ERR_OK )
      return mrc;
   if ( !gbm_ref_vert(&(modNew->gbm), modNew->pbData) )
      {
      free(modNew->pbData);
      return MOD_ERR_MEM;
      }
   return MOD_ERR_OK;
   }

static MOD_ERR ModRotate270(MOD *mod, MOD *modNew)
   {
   MOD_ERR mrc;
   MOD modTmp;

   if ( (mrc = ModReflectHorz(mod, &modTmp)) != MOD_ERR_OK )
      return mrc;
   mrc = ModTranspose(&modTmp, modNew);
   ModDelete(&modTmp);
   return mrc;
   }

static MOD_ERR ModRotate90(MOD *mod, MOD *modNew)
{
   MOD_ERR mrc;
   MOD modTmp;

   if ( (mrc = ModReflectVert(mod, &modTmp)) != MOD_ERR_OK )
      return mrc;
   mrc = ModTranspose(&modTmp, modNew);
   ModDelete(&modTmp);
   return mrc;
}


static MOD_ERR ModDelete(MOD *mod)
{
   free(mod->pbData);
   return MOD_ERR_OK;
}
/*
** Write image data to file.
*/
static MOD_ERR ModWriteToFile(MOD *mod,CHAR *szFn, CHAR *szOpt)
{
   GBM_ERR grc;
   int fd, ft, flag;
   GBMFT gbmft;

   if ( (grc = gbm_guess_filetype(szFn, &ft)) != GBM_ERR_OK )
      return grc;

   gbm_query_filetype(ft, &gbmft);
   switch ( mod->gbm.bpp )
   {
      case 1:      flag = GBM_FT_W1;   break;
      case 4:      flag = GBM_FT_W4;   break;
      case 8:      flag = GBM_FT_W8;   break;
      case 24:     flag = GBM_FT_W24;  break;
      }

   if ( (gbmft.flags & flag) == 0 )
      return MOD_ERR_SUPPORT;

   if ( (fd = open(szFn, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, S_IREAD|S_IWRITE)) == -1 )
      return MOD_ERR_CREATE;

   if ( (grc = gbm_write(szFn, fd, ft, &(mod->gbm), mod->gbmrgb, mod->pbData, szOpt)) != GBM_ERR_OK )
      {
      close(fd);
      unlink(szFn);
      return MOD_ERR_GBM(grc);
      }

   close(fd);

   return MOD_ERR_OK;
}


static int ModCreateFromFile(CHAR *szFn, CHAR *szOpt,MOD *modNew)
{

GBM_ERR grc;
int fd, ft;

   if ( (grc = gbm_guess_filetype(szFn, &ft)) != GBM_ERR_OK )
      return MOD_ERR_GBM(grc);

   if ( (fd = open(szFn, O_RDONLY|O_BINARY)) == -1 )
      return MOD_ERR_OPEN;

   if ( (grc = gbm_read_header(szFn, fd, ft, &(modNew->gbm), szOpt)) != GBM_ERR_OK )
      {
      close(fd);
      return MOD_ERR_GBM(grc);
      }

   if ( (grc = gbm_read_palette(fd, ft, &(modNew->gbm), modNew->gbmrgb)) != GBM_ERR_OK )
      {
      close(fd);
      return MOD_ERR_GBM(grc);
      }

   if ( !AllocateData(modNew) )
      {
      close(fd);
      return MOD_ERR_MEM;
      }

   if ( (grc = gbm_read_data(fd, ft, &(modNew->gbm), modNew->pbData)) != GBM_ERR_OK )
      {
      free(modNew->pbData);
      close(fd);
      return MOD_ERR_GBM(grc);
      }

   close(fd);

   return MOD_ERR_OK;
}
/*---------------------------------------------------------------------------*/
static int ModMakeHBITMAP(MOD *mod, pImage pImg)
{
   USHORT usColors;
   ULONG  bytesperline,newdatasize;

   if (mod->gbm.bpp == 24)
      usColors = 0;
   else
      usColors = (1<<mod->gbm.bpp);


   pImg->pbmp2= (BITMAPINFOHEADER2 *)calloc(((ULONG)sizeof(BITMAPINFOHEADER2)+sizeof(RGB2)*usColors),
                                                        sizeof(char));

   pImg->pbmp2->cbFix    = sizeof(BITMAPINFOHEADER2);
   pImg->pbmp2->cx       = mod->gbm.w;
   pImg->pbmp2->cy       = mod->gbm.h;
   pImg->pbmp2->cBitCount= mod->gbm.bpp;
   pImg->pbmp2->cPlanes  = 1;
   pImg->pbmp2->cxResolution  = 0;
   pImg->pbmp2->cyResolution  = 0;
   pImg->pbmp2->usUnits       = BRU_METRIC;
   pImg->pbmp2->usRecording   = 0;
   pImg->pbmp2->usRendering   = 0;
   pImg->pbmp2->ulColorEncoding = BCE_RGB;
   pImg->pbmp2->cclrUsed      = usColors;
   pImg->pbmp2->cclrImportant = 0;

   if ( mod->gbm.bpp != 24 )
   {
      int i;
      RGB2  *pRGB2;
      unsigned char *data;

      data = (unsigned char *)pImg->pbmp2;
      pRGB2 = (RGB2 *)(data + sizeof(BITMAPINFOHEADER2));

      for ( i = 0; i < usColors; i++ )
      {
         pRGB2->bRed   = mod->gbmrgb[i].r;
         pRGB2->bGreen = mod->gbmrgb[i].g;
         pRGB2->bBlue  = mod->gbmrgb[i].b;
         pRGB2->fcOptions = 0;
         pRGB2++;
      }
   }
   pImg->ImgData = mod->pbData;
   bytesperline = ((pImg->pbmp2->cx * pImg->pbmp2->cBitCount + 31)/32) * 4 * pImg->pbmp2->cPlanes;
   newdatasize = bytesperline * pImg->pbmp2->cy;
   pImg->pbmp2->cbImage = newdatasize;
   pImg->ImageDataSize  = newdatasize;
   return 0;
}

BOOL readPicture(HWND hwndClient,char *pszFilename, pImage pImg)
{
   char *p;
   MOD mod;
   int mrc;
   struct stat bf;

   if (!pszFilename)
      return FALSE;

   if (stat(pszFilename,&bf) != 0)
      return FALSE;

   mrc = ModCreateFromFile(pszFilename, (char *)"",&mod);
   if (mrc != MOD_ERR_OK)
   {
      printf("ERROR:Loading %s\n",pszFilename);
      return FALSE;
   }

   ModMakeHBITMAP(&mod,pImg);
   return TRUE;
}

BOOL writePicture(HWND hwndClient,char *pszFilename, pImage pImg)
{
   char *p;
   MOD mod;
   int mrc;
   char szOpt[10];
   struct stat bf;

   if (!pszFilename)
      return FALSE;

   Hbitmap2Mod(&mod,pImg);
   strcpy(szOpt," ");
   mrc = ModWriteToFile(&mod,pszFilename,szOpt);

   return  (BOOL)(mrc == MOD_ERR_OK);
}
/*------------------------------------------------------------------------*/
/* ImgRotate.                                                             */
/*------------------------------------------------------------------------*/
BOOL imgRotate(IMAGE * pImg,int degrees)
{
  MOD    modNew,mod;
  int    mrc;
  float  fcyOrg,fOrg;
  
  if (!pImg)
     return FALSE;

  if (degrees != 90 && degrees != 270)  
      return FALSE;

  Hbitmap2Mod(&mod,pImg);

  if (degrees == 90)
     mrc = ModRotate90(&mod, &modNew);
  else
     mrc = ModRotate270(&mod, &modNew);

  if (mrc == MOD_ERR_OK)
  {
      pImg->pbmp2->cx = modNew.gbm.w;
      pImg->pbmp2->cy = modNew.gbm.h;
      pImg->ImgData   = modNew.pbData;
      return TRUE;
  }
  return FALSE;
}
