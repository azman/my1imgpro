/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILE_FMTH__
#define __MY1IMAGE_FILE_FMTH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
/** need FILE* definition */
#include <stdio.h>
/*----------------------------------------------------------------------------*/
/** a fancy way of setting msb to 1 */
#define FILE_ERROR_FLAG (~(~0U>>1))
#define FILE_ERROR_OPEN (0x00000001|FILE_ERROR_FLAG)
#define FILE_ERROR_FORMAT (0x00000002|FILE_ERROR_FLAG)
#define FILE_ERROR_SIZE (0x00000004|FILE_ERROR_FLAG)
#define FILE_ERROR_MALLOC (0x00000008|FILE_ERROR_FLAG)
#define FILE_ERROR_CORRUPT (0x00000010|FILE_ERROR_FLAG)
/*----------------------------------------------------------------------------*/
#define IMGFMT_NAMESIZE 16
/*----------------------------------------------------------------------------*/
typedef int (*pimgfile)(my1image_t*, FILE*);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_format_t
{
	unsigned flag; /* error flag id? */
	char extd[IMGFMT_NAMESIZE]; /* filename extension - for saving file? */
	pimgfile do_load, do_save;
	struct _my1image_format_t *next;
}
my1image_format_t;
typedef my1image_format_t my1imgfmt_t;
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
