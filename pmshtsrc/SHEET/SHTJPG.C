/*------------------------------------------------------------------------*/
/*  Name: drwjpg.c                                                        */
/*                                                                        */
/*  Description : JPEG filehandling. Reads an jpeg imagefile and inserts  */
/*                the data into a drawit image segment.                   */
/*                                                                        */
/*  Functions  :                                                          */
/*                                                                        */
/*------------------------------------------------------------------------*/
#define INCL_WIN
#define INCL_GPI
#define INCL_DOS

#include <os2.h>
#include <stdlib.h>
#include <memory.h>
#include <io.h>
#include <stdio.h>
#include "jpeglib.h"
//#include "jmorecfg.h"
#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#define JMAKE_MSG_TABLE
#include "cderror.h"		/* create message string table */
#include "jversion.h"		/* for version message */

#include <ctype.h>		/* to declare isupper(),tolower(),isprint() */
#include "..\resource.h"

#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif

#ifdef USE_SETMODE
#include <fcntl.h>		/* to declare setmode()'s parameter macros */
/* If you have setmode() but not <io.h>, just delete this line: */
#include <io.h>			/* to declare setmode() */
#endif

#define  ROUNDTODWORD(b)   (((b+31)/32)*4)

#define UM_JPGERROR         WM_USER+32 
static const char * progname;	/* program name for error messages */
static char * outfilename;	/* for -outfile switch */


#include "shtimg.h"

static pImage thisImg;
static HWND   thisClient;
static BOOL   bSuccess,bError;

static char filename[255];
/*
**
** Called on error.
*/
HWND getClientWindow(void)
{
   return thisClient;
}

void sheet_message (j_common_ptr cinfo)
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
  sprintf(szError,"Could not load:%s\nError code %s",filename,buffer);
  WinPostMsg(thisClient,UM_JPGERROR,(MPARAM)szError,(MPARAM)0);
  }
}

void sheet_exit (j_common_ptr cinfo)
{
  /* Always display the message */
  (*cinfo->err->output_message) (cinfo);

  /* Let the memory manager delete any temp files before we die */
  jpeg_destroy(cinfo);

  DosExit(EXIT_THREAD,EXIT_FAILURE);
}



static pImage getImageObject(void)
{
   return thisImg;
}
/*
 * To support 12-bit JPEG data, we'd have to scale output down to 8 bits.
 * This is not yet implemented.
 */

/*
 * Since BMP stores scanlines bottom-to-top, we have to invert the image
 * from JPEG's top-to-bottom order.  To do this, we save the outgoing data
 * in a virtual array during put_pixel_row calls, then actually emit the
 * BMP file during finish_output.  The virtual array contains one JSAMPLE per
 * pixel if the output is grayscale or colormapped, three if it is full color.
 */

/* Private version of data destination object */

typedef struct {
  struct djpeg_dest_struct pub;	/* public fields */

  boolean is_os2;		/* saves the OS2 format request flag */

  jvirt_sarray_ptr whole_image;	/* needed to reverse row order */
  JDIMENSION data_width;	/* JSAMPLEs per row */
  JDIMENSION row_width;		/* physical width of one row in the BMP file */
  int pad_bytes;		/* number of padding bytes needed per row */
  JDIMENSION cur_output_row;	/* next row# to write to virtual array */
} bmp_dest_struct;

typedef bmp_dest_struct * bmp_dest_ptr;



/*
 * Write some pixel data.
 * In this module rows_supplied will always be 1.
 */

void put_pixel_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
		JDIMENSION rows_supplied)
/* This version is for writing 24-bit pixels */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

  /*
  ** Access next row in virtual array (source data).
  */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /*
  ** Transfer data.  Note destination values must be in BGR order
  ** (even though Microsoft's own documents say the opposite).
  ** inptr points to the jpeg imagedata.
  ** outptr points to the buffer which will finally hold he OS/2 bitmap
  ** data. Both buffers are allocated by the jpeg library.
  */

  inptr  = dest->pub.buffer[0];
  outptr = image_ptr[0];

  for (col = cinfo->output_width; col > 0; col--)
  {

    outptr[2] = *inptr++;	/* can omit GETJSAMPLE() safely */
    outptr[1] = *inptr++;
    outptr[0] = *inptr++;
    outptr += 3;
  }
  /*
  ** Zero out the pad bytes.
  */
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}

void put_gray_rows (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo,
	       JDIMENSION rows_supplied)
/* This version is for grayscale OR quantized color output */
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW inptr, outptr;
  register JDIMENSION col;
  int pad;

  /* Access next row in virtual array */
  image_ptr = (*cinfo->mem->access_virt_sarray)
    ((j_common_ptr) cinfo, dest->whole_image,
     dest->cur_output_row, (JDIMENSION) 1, TRUE);
  dest->cur_output_row++;

  /* Transfer data. */
  inptr = dest->pub.buffer[0];
  outptr = image_ptr[0];
  for (col = cinfo->output_width; col > 0; col--) {
    *outptr++ = *inptr++;	/* can omit GETJSAMPLE() safely */
  }

  /* Zero out the pad bytes. */
  pad = dest->pad_bytes;
  while (--pad >= 0)
    *outptr++ = 0;
}

/*
** Startup: normally writes the file header.
** In this module we may as well postpone everything until finish_output.
*/

void start_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  /* no work here */
}
/*---------------------------------------------------[ private ]----------*/
/*  setup_os2_header.                                                     */
/*                                                                        */
/*  Description : Sets the OS/2 bitmap header up.                         */
/*                                                                        */
/*------------------------------------------------------------------------*/
static long os2header (j_decompress_ptr cinfo, bmp_dest_ptr dest,pImage pimage)
{
  if (cinfo->quantize_colors)
  {
     /*
     ** Allocate space for the header plus the colormap
     */
     pimage->pbmp2=(BITMAPINFOHEADER2 *)calloc((ULONG)(sizeof(BITMAPINFOHEADER2) + sizeof(RGB2)*256),
                                                sizeof(char));
  }
  else
  {
     /*
     ** Allocate space for the header. True color image.
     */
     pimage->pbmp2=(BITMAPINFOHEADER2 *)calloc((ULONG)sizeof(BITMAPINFOHEADER2),sizeof(char));
  }
  pimage->pbmp2->cbFix = sizeof(BITMAPINFOHEADER2);
  pimage->pbmp2->ulCompression = BCA_UNCOMP;
  pimage->pbmp2->cPlanes       = 1;        /*????????????*/
  if (cinfo->density_unit == 2)  /* if have density in dots/cm, then */
  {
     pimage->pbmp2->cxResolution  = (LONG)(cinfo->X_density*100); /* XPels/M */
     pimage->pbmp2->cyResolution  = (LONG)(cinfo->X_density*100); /* XPels/M */
  }
  pimage->pbmp2->usUnits       = BRU_METRIC;
  pimage->pbmp2->usRecording   = 0;
  pimage->pbmp2->usRendering   = 0;
  pimage->pbmp2->ulColorEncoding = BCE_RGB;
  pimage->pbmp2->cclrImportant   = 0;
  pimage->pbmp2->cx      = cinfo->output_width;
  pimage->pbmp2->cy      = cinfo->output_height;
  pimage->pbmp2->cbImage = (INT32) dest->row_width * (INT32) cinfo->output_height;

  /* Compute colormap size and total file size */
  if (cinfo->out_color_space == JCS_RGB)
  {
    if (cinfo->quantize_colors)
    {
      /*
      ** Colormapped RGB
      */
      pimage->pbmp2->cBitCount     = 8;
      pimage->pbmp2->cclrUsed      = 256;

    }
    else
    {
      /*
      ** True color RGB
      */
      pimage->pbmp2->cBitCount = 24;
      pimage->pbmp2->cclrUsed  = 0;
    }
    return 1L;
  }

  if (pimage->pbmp2->cclrUsed)
  {
     JSAMPARRAY colormap = cinfo->colormap;
     int num_colors = cinfo->actual_number_of_colors;
     int i;
     PBYTE pData;
     PRGB2 pRGB2;

     if (colormap != NULL)
     {
        pData = (PBYTE)pimage->pbmp2;
        pRGB2 = (RGB2 *)(pData + sizeof(BITMAPINFOHEADER2));

        if (cinfo->out_color_components == 3)
        {
           /*
           ** Normal case with RGB colormap
           */
           for (i = 0; i < num_colors; i++)
           {
              pRGB2->bRed  = GETJSAMPLE(colormap[2][i]);
              pRGB2->bGreen= GETJSAMPLE(colormap[1][i]);
              pRGB2->bBlue = GETJSAMPLE(colormap[0][i]);
              pRGB2->fcOptions = 0;
              pRGB2++;
           }
        }
     }
  }
}
/*------------------------------------------------------------------------*/
/*  Name: finish_output_bmp                                               */
/*                                                                        */
/*  Description : Finaly writes the data into the drawit image obj.       */
/*                                                                        */
/*  Functions  :                                                          */
/*                                                                        */
/*------------------------------------------------------------------------*/
void finish_output_bmp (j_decompress_ptr cinfo, djpeg_dest_ptr dinfo)
{
  bmp_dest_ptr dest = (bmp_dest_ptr) dinfo;
  JSAMPARRAY image_ptr;
  register JSAMPROW data_ptr;
  JDIMENSION row;
  register JDIMENSION col;
  unsigned char *pData;
  cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;

  pImage pImg = getImageObject();
  /*
  ** setup the header and colormap
  */
  if (!os2header(cinfo, dest,pImg))
     return;

  if (!pImg->pbmp2->cbImage)
     pImg->pbmp2->cbImage = ROUNDTODWORD(pImg->pbmp2->cx * pImg->pbmp2->cBitCount)*pImg->pbmp2->cy;

  pImg->ImgData = (char *)calloc((ULONG)pImg->pbmp2->cbImage,sizeof(char));
  pData = (unsigned char *)pImg->ImgData;

  /* Write the file body from our virtual array */
  for (row = cinfo->output_height; row > 0; row--)
  {
    if (progress != NULL)
    {
      progress->pub.pass_counter = (long) (cinfo->output_height - row);
      progress->pub.pass_limit = (long) cinfo->output_height;
      (*progress->pub.progress_monitor) ((j_common_ptr) cinfo);
    }

    image_ptr = (*cinfo->mem->access_virt_sarray)
      ((j_common_ptr) cinfo, dest->whole_image, row-1,(JDIMENSION) 1, FALSE);
    data_ptr = image_ptr[0];

    for (col = dest->row_width; col > 0; col--)
    {
      *pData = *data_ptr;
      data_ptr++;
      pData++;
    }
  }
  if (progress != NULL)
    progress->completed_extra_passes++;
}
/*-----------------------------------------------[ private ]--------------*/
/*  Name: jinit_write_bmp.                                                */
/*                                                                        */
/*  Description : Fills in all the function pointers needed by the        */
/*                jpeg library for writing a jpeg file into a BMP.        */
/*                All functions which are filled in here can be found in  */
/*                this file.                                              */
/*                                                                        */
/*                                                                        */
/*------------------------------------------------------------------------*/
djpeg_dest_ptr jinit_write_bmp (j_decompress_ptr cinfo, boolean is_os2)
{
  bmp_dest_ptr dest;
  JDIMENSION row_width;

  /* Create module interface object, fill in method pointers */
  dest = (bmp_dest_ptr)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(bmp_dest_struct));
  dest->pub.start_output = start_output_bmp;
  dest->pub.finish_output = finish_output_bmp;
  dest->is_os2 = TRUE;

  if (cinfo->out_color_space == JCS_GRAYSCALE)
  {
    dest->pub.put_pixel_rows = put_gray_rows;
  }
  else if (cinfo->out_color_space == JCS_RGB)
  {
    if (cinfo->quantize_colors)
      dest->pub.put_pixel_rows = put_gray_rows;
    else
      dest->pub.put_pixel_rows = put_pixel_rows;
  }
  else
  {
    ERREXIT(cinfo, JERR_BMP_COLORSPACE);
  }

  /* Calculate output image dimensions so we can allocate space */
  jpeg_calc_output_dimensions(cinfo);

  /* Determine width of rows in the BMP file (padded to 4-byte boundary). */
  row_width = cinfo->output_width * cinfo->output_components;
  dest->data_width = row_width;
  while ((row_width & 3) != 0) row_width++;
  dest->row_width = row_width;
  dest->pad_bytes = (int) (row_width - dest->data_width);

  /* Allocate space for inversion array, prepare for write pass */
  dest->whole_image = (*cinfo->mem->request_virt_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, FALSE,
     row_width, cinfo->output_height, (JDIMENSION) 1);

  dest->cur_output_row = 0;

  if (cinfo->progress != NULL)
  {
    cd_progress_ptr progress = (cd_progress_ptr) cinfo->progress;
    progress->total_extra_passes++; /* count file input as separate pass */
  }

  /* Create decompressor output buffer. */
  dest->pub.buffer = (*cinfo->mem->alloc_sarray)
    ((j_common_ptr) cinfo, JPOOL_IMAGE, row_width, (JDIMENSION) 1);
  dest->pub.buffer_height = 1;

  return (djpeg_dest_ptr) dest;
}
/*------------------------------------------------------------------------*/
/*  Name: readjpeg.                                                       */
/*                                                                        */
/*  Description : Entry point of the jpeg image loader for drawit.        */
/*                                                                        */
/*  Parameters  : WINDOWINFO *pwi.                                        */
/*                                                                        */
/*  Returns     : BOOL TRUE on success.                                   */
/*------------------------------------------------------------------------*/
BOOL readjpeg(HWND hwndClient,char *pszFilename, pImage pImg)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  int file_index;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE *hfile,*ofile;
  JDIMENSION num_scanlines;

  progname = "sheet";	/* in case C library doesn't provide it */

  thisImg    = pImg;
  thisClient = hwndClient;
  bSuccess   = TRUE;
  bError     = FALSE;
  /*
  ** Initialize the JPEG decompression object with default error handling.
  */
  cinfo.err = jpeg_std_error(&jerr);


  jerr.output_message         = sheet_message;
  jerr.error_exit             = sheet_exit;

  cinfo.dct_method = JDCT_IFAST;
//  cinfo.desired_number_of_colors = 256;
  cinfo.quantize_colors = TRUE;

//  cinfo.dither_mode = JDITHER_ORDERED;

  jpeg_create_decompress(&cinfo);

  /*
  ** Add some application-specific error messages (from cderror.h)
  */
//  jerr.addon_message_table = addon_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message  = JMSG_LASTADDONCODE;

  if ( (hfile = fopen(pszFilename,"rb")) == NULL)
     return FALSE;
  /*
  ** Copy name in global buf to be used on error
  */
  strcpy(filename,pszFilename);
  /*
  ** Specify data source for decompression
  */
  jpeg_stdio_src(&cinfo, hfile);
  /*
  ** Read file header, set default decompression parameters
  */
  (void) jpeg_read_header(&cinfo, TRUE);

  /*
  ** Initialize the output module now to let it override any crucial
  ** option settings (for instance, GIF wants to force color quantization).
  */
  dest_mgr = jinit_write_bmp(&cinfo,TRUE);
  dest_mgr->output_file = hfile;

  jpeg_start_decompress(&cinfo);  /* Start decompressor */


  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height)
  {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }
  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  (*dest_mgr->finish_output) (&cinfo, dest_mgr);
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(hfile);
  return bSuccess; 
}
