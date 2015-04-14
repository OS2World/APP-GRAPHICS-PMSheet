/*
 * drwrdbmp.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to read input images in Microsoft "BMP"
 * format (MS Windows 3.x, OS/2 1.x, and OS/2 2.x flavors).
 * Currently, only 8-bit and 24-bit images are supported, not 1-bit or
 * 4-bit (feeding such low-depth images into JPEG would be silly anyway).
 * Also, we don't support RLE-compressed files.
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume input from
 * an ordinary stdio stream.  They further assume that reading begins
 * at the start of the file; start_input may need work if the
 * user interface has already read some data (e.g., to determine that
 * the file is indeed BMP format).
 *
 * This code contributed by James Arthur Boucher.
 */
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <memory.h>
#include <io.h>
#include <stdio.h>
#include "jpeglib.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#define JMAKE_MSG_TABLE
#include "cderror.h"		/* create message string table */
#include "jversion.h"		/* for version message */

#include <ctype.h>		/* to declare isupper(),tolower(),isprint() */
#include "..\resource.h"
#include "shtimg.h"

#ifdef BMP_SUPPORTED


/* Macros to deal with unsigned chars as efficiently as compiler allows */

#ifdef HAVE_UNSIGNED_CHAR
typedef unsigned char U_CHAR;
#define UCH(x)	((int) (x))
#else /* !HAVE_UNSIGNED_CHAR */
#ifdef CHAR_IS_UNSIGNED
typedef char U_CHAR;
#define UCH(x)	((int) (x))
#else
typedef char U_CHAR;
#define UCH(x)	((int) (x) & 0xFF)
#endif
#endif /* HAVE_UNSIGNED_CHAR */


#define UM_JPGERROR         WM_USER+32 

/* Private version of data source object */

typedef struct _bmp_source_struct * bmp_source_ptr;

typedef struct _bmp_source_struct 
{
  struct cjpeg_source_struct pub; /* public fields                         */
  j_compress_ptr cinfo;		  /* back link saves passing separate parm */
  JSAMPARRAY colormap;		  /* BMP colormap (converted to my format) */
  jvirt_sarray_ptr whole_image;	  /* Needed to reverse row order           */
  JDIMENSION source_row;	  /* Current source row number             */
  JDIMENSION row_width;		  /* Physical width of scanlines in file   */
  int bits_per_pixel;		  /* remembers 8- or 24-bit format         */
  pImage    pSaveImg;
} bmp_source_struct;

static pImage pSave;
static BOOL bError;
static char filename[255];
static HWND thisClient;

static void sht_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  char *szError;

  if (!bError)
  {
     bError = TRUE;

  if (thisClient)
  {
     szError = (char *)calloc(JMSG_LENGTH_MAX+150,1);
  }
  else
     return;

  /* Create the message */
  (*cinfo->err->format_message) (cinfo, buffer);

  /* Send it to stderr, adding a newline */
  sprintf(szError,"Could not write:%s\nError code %s",filename,buffer);
  WinPostMsg(thisClient,UM_JPGERROR,(MPARAM)szError,(MPARAM)0);
  }
}

static void sht_exit (j_common_ptr cinfo)
{
  /* Always display the message */
  (*cinfo->err->output_message) (cinfo);

  /* Let the memory manager delete any temp files before we die */
  jpeg_destroy(cinfo);

  DosExit(EXIT_THREAD,EXIT_FAILURE);
}
/*-----------------------------------------------[ private ]-----------------*/
/* Name : getColorMap.                                                       */
/*                                                                           */
/* Read the colormap from a BMP file                                         */
/*---------------------------------------------------------------------------*/
static void getColorMap(bmp_source_ptr sinfo, int iColors, RGB2 *pRGB2)
{
  int i;
  /*
  ** RGB2 struct contains four bytes. RGB0
  ** BGR format (occurs in OS/2 files) 
  */
  for (i = 0; i < iColors; i++) 
  {
    sinfo->colormap[2][i] = (JSAMPLE) pRGB2->bBlue;
    sinfo->colormap[1][i] = (JSAMPLE) pRGB2->bGreen;
    sinfo->colormap[0][i] = (JSAMPLE) pRGB2->bRed;
    pRGB2++;
  }
}
/*---------------------------------------------------------------------------*/
/* get_8bit_row.  This version is for reading 8-bit colormap indexes         */
/*                                                                           */
/* Description  : Read one row of pixels. The image has been read into the   */
/*                whole_image array, but is otherwise unprocessed.  We must  */
/*                read it out in top-to-bottom row order, and if it is an    */
/*                8-bit image, we must expand colormapped pixels to 24bit    */
/*                format.                                                    */
/*---------------------------------------------------------------------------*/
JDIMENSION get_8bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  register JSAMPARRAY colormap = source->colormap;
  JSAMPARRAY image_ptr;
  register int t;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;

  /* Fetch next row from virtual array */
  source->source_row--;
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->whole_image, source->source_row,(JDIMENSION) 1, FALSE);

  /* Expand the colormap indexes to real data */
  inptr = image_ptr[0];
  outptr = source->pub.buffer[0];
  for (col = cinfo->image_width; col > 0; col--) {
    t = GETJSAMPLE(*inptr++);
    *outptr++ = colormap[0][t];	/* can omit GETJSAMPLE() safely */
    *outptr++ = colormap[1][t];
    *outptr++ = colormap[2][t];
  }
  return 1;
}
/*---------------------------------------------------------------------------*/
/* get_24bit_row.             This version is for reading 24-bit pixels      */
/*---------------------------------------------------------------------------*/
JDIMENSION get_24bit_row (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  /* 
  ** Fetch next row from virtual array 
  */
  source->source_row--;
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, source->whole_image, source->source_row,(JDIMENSION) 1, FALSE);
  /* 
  ** Transfer data.  Note source values are in BGR order
  ** (even though Microsoft's own documents say the opposite).
  */
  inptr  = image_ptr[0];
  outptr = source->pub.buffer[0];

  for (col = cinfo->image_width; col > 0; col--) 
  {
    outptr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
    outptr[1] = *inptr++;
    outptr[0] = *inptr++;
    outptr   += 3;
  }
  return 1;
}
/*------------------------------------------------------------------------
 * This method loads the image into whole_image during the first call on
 * get_pixel_rows.  The get_pixel_rows pointer is then adjusted to call
 * get_8bit_row or get_24bit_row on subsequent calls.
 ------------------------------------------------------------------------*/
JDIMENSION preload_image (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  register JSAMPROW out_ptr;
  JSAMPARRAY image_ptr;
  JDIMENSION row, col;
  PBITMAPINFO2   pbmi; /* is infoheader2 + pointer to rgb table    */
  unsigned char *pbScan;
  unsigned short usColors;
  
  pbmi = (PBITMAPINFO2)source->pSaveImg->pbmp2;
 
  usColors = 1 << ( pbmi->cBitCount * pbmi->cPlanes );

  pbScan  = source->pSaveImg->ImgData;

  /*
  ** Read the data into a virtual array in input-file row order. 
  */
  for (row = 0; row < cinfo->image_height; row++) 
  {
    image_ptr = (*cinfo->mem->access_virt_sarray)
                 ((j_common_ptr) cinfo, source->whole_image, row, (JDIMENSION) 1, TRUE);

    out_ptr = image_ptr[0];

    for (col = source->row_width; col > 0; col--) 
    {
      *out_ptr = *pbScan;
      out_ptr++;
      pbScan++;

    }
  }
  /* 
  ** Set up to read from the virtual array in top-to-bottom order 
  */
  switch (source->bits_per_pixel) 
  {
  case 8:
    source->pub.get_pixel_rows = get_8bit_row;
    break;
  case 24:
    source->pub.get_pixel_rows = get_24bit_row;
    break;
  default:
    ERREXIT(cinfo, JERR_BMP_BADDEPTH);
  }
  source->source_row = cinfo->image_height;
  /* 
  ** And read the first row 
  */
  return (*source->pub.get_pixel_rows) (cinfo, sinfo);
}
/*---------------------------------------------------------------------------*/
/* Read the file header; return image size and component count.              */
/*---------------------------------------------------------------------------*/
void start_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  bmp_source_ptr source = (bmp_source_ptr) sinfo;
  INT32 headerSize;
  INT32 biWidth = 0;		
  INT32 biHeight = 0;
  unsigned int biPlanes;
  INT32 biCompression;
  INT32 biXPelsPerMeter,biYPelsPerMeter;
  INT32 biClrUsed = 0;
  int mapentrysize = 0;		/* 0 indicates no colormap */
  JDIMENSION row_width;
  PBITMAPINFO2 pbmi;            /* is infoheader2 + pointer to rgb table    */
  PRGB2        pRGB2;

  pbmi = (PBITMAPINFO2)source->pSaveImg->pbmp2;


  biWidth                = pbmi->cx;
  biHeight               = pbmi->cy;
  biPlanes               = pbmi->cPlanes;
  biXPelsPerMeter        = 0;
  biYPelsPerMeter        = 0;
  biClrUsed              = 1 << ( pbmi->cBitCount * pbmi->cPlanes );

  source->bits_per_pixel = pbmi->cBitCount;

  /* Read the colormap, if any */
  if (biClrUsed > 0 && biClrUsed <= 256) 
  {
     if (biClrUsed <= 0)
        biClrUsed = 256;       /* assume it's 256 */

    pRGB2 = &pbmi->argbColor[0];     /* Color definition record */
    /* 
    ** Allocate space to store the colormap 
    */
    source->colormap = (*cinfo->mem->alloc_sarray) 
                        ((j_common_ptr)cinfo, 
                        JPOOL_IMAGE, 
                        (JDIMENSION)biClrUsed,      /* # of palette entries */
                        (JDIMENSION)4);             /* # bytes per color    */
    /*
    ** and read it from memory.
    */
    getColorMap(source,(int)biClrUsed,pRGB2);
  }

  /*
  ** Compute row width in file, including padding to 4-byte boundary 
  */
  if (source->bits_per_pixel == 24)
    row_width = (JDIMENSION) (biWidth * 3);
  else
    row_width = (JDIMENSION) biWidth;
  while ((row_width & 3) != 0) row_width++;
  source->row_width = row_width;

  /*
  ** Allocate space for inversion array, prepare for preload pass 
  */
  source->whole_image = (*cinfo->mem->request_virt_sarray)
                         ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
                         row_width, (JDIMENSION) biHeight, (JDIMENSION) 1);

  source->pub.get_pixel_rows = preload_image;

  /* 
  ** Allocate one-row buffer for returned data 
  */
  source->pub.buffer = (*cinfo->mem->alloc_sarray)
                        ((j_common_ptr) cinfo, JPOOL_IMAGE,
                        (JDIMENSION) (biWidth * 3), (JDIMENSION) 1);


  source->pub.buffer_height = 1;

  cinfo->in_color_space   = JCS_RGB;
  cinfo->input_components = 3;
  cinfo->data_precision   = 8;
  cinfo->image_width      = (JDIMENSION) biWidth;
  cinfo->image_height     = (JDIMENSION) biHeight;
}
/*---------------------------------------------------------------------------*/
/* Finish up at the end of the file.                                         */
/*---------------------------------------------------------------------------*/
void finish_input_bmp (j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  /*---------------- no work ----------------*/
}
/*-----------------------------------------------[ public ]------------------*/
/* jinit_read_bmp       The module selection routine for BMP format input.   */
/*---------------------------------------------------------------------------*/
cjpeg_source_ptr jinit_read_bmp (j_compress_ptr cinfo)
{
  bmp_source_ptr source;

  /* Create module interface object */
  source = (bmp_source_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_source_struct));

  source->cinfo = cinfo;	/* make back link for subroutines */
  /* 
  ** Fill in method ptrs, except get_pixel_rows which start_input sets 
  */
  source->pub.start_input  = start_input_bmp;
  source->pub.finish_input = finish_input_bmp;
  source->pSaveImg         = pSave; /* Pick up the image structure. */

  return (cjpeg_source_ptr) source;
}
/*---------------------------------------------------------------------------*/
BOOL writejpeg(HWND hwndClient,char *pszFilename, pImage pImg)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  int file_index;
  cjpeg_source_ptr src_mgr;
  FILE * input_file;
  FILE * output_file;
  JDIMENSION num_scanlines;
  char *progname = "Pipo";  /* in case C library doesn't provide it */

  pSave = pImg;
  /* 
  ** Initialize the JPEG compression object with default error handling. 
  */
  memset((void *)&cinfo,0,sizeof(struct jpeg_compress_struct));

  cinfo.err = jpeg_std_error(&jerr);

  jerr.output_message         = sht_message;
  jerr.error_exit             = sht_exit;

  jpeg_create_compress(&cinfo);
  /* 
  ** Add some application-specific error messages (from cderror.h) 
  */
//  jerr.addon_message_table = addon_message_table;
//  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
//  jerr.last_addon_message  = JMSG_LASTADDONCODE;

    /* Initialize JPEG parameters.
    * Much of this may be overridden later.
    * In particular, we don't yet know the input file's color space,
    * but we need to provide some value for jpeg_set_defaults() to work.
    */

    cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
    jpeg_set_defaults(&cinfo);

    /* Scan command line to find file names.
    * It is convenient to use just one switch-parsing routine, but the switch
    * values read here are ignored; we will rescan the switches after opening
    * the input file.
    */

//  file_index = parse_switches(&cinfo, argc, argv, 0, FALSE);

//  outfilename = pszFilename;

  /* Open the input file. */

  /* 
  ** Open the output file. 
  */
  if (pszFilename != NULL) {
      strcpy(filename,pszFilename);
      thisClient = hwndClient;
    if ((output_file = fopen(pszFilename,"wb")) == NULL) {
      return FALSE;
    }
  }


  /* 
  ** Figure out the input file format, and set up to read it. 
  */
  src_mgr = jinit_read_bmp(&cinfo);

  src_mgr->input_file = input_file;
  /* 
  ** Read the input file header to obtain file size & colorspace. 
  */
  (*src_mgr->start_input) (&cinfo, src_mgr);
  /* 
  ** Now that we know input colorspace, fix colorspace-dependent defaults 
  */
  jpeg_default_colorspace(&cinfo);
  /* 
  ** Adjust default compression parameters by re-parsing the options 
  */
//  file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);
  /* 
  ** Specify data destination for compression 
  */
  jpeg_stdio_dest(&cinfo, output_file);
  /* 
  ** Start compressor 
  */
  jpeg_start_compress(&cinfo, TRUE);


  /* Process data */
    while (cinfo.next_scanline < cinfo.image_height) 
    {
        num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);

        (void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
    }

    /* Finish compression and release memory */
    (*src_mgr->finish_input) (&cinfo, src_mgr);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(output_file);

    /* All done. */
    return TRUE;			/* suppress no-return-value warnings */
}


#endif /* BMP_SUPPORTED */
