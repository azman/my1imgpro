/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_GRAB_H__
#define __MY1IMAGE_GRAB_H__
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
/*----------------------------------------------------------------------------*/
#define IGRAB_FLAG_NEW_FRAME 0x01
#define IGRAB_FLAG_ERROR (~(~0U>>1))
#define IGRAB_FLAG_FILE_ERROR (IGRAB_FLAG_ERROR|0x02)
#define IGRAB_FLAG_LOAD_ERROR (IGRAB_FLAG_ERROR|0x04)
#define IGRAB_FLAG_GRAB_ERROR (IGRAB_FLAG_ERROR|0x08)
#define IGRAB_FLAG_TASK_ERROR (IGRAB_FLAG_ERROR|0x10)
/*----------------------------------------------------------------------------*/
typedef struct _my1image_grab_t
{
	int flag;
	char *pick;
	my1image_t buff; /* internal buffer */
	my1image_t *grab; /* pure pointer - will use image is not assigned */
	my1dotask_t do_grab;
}
my1image_grab_t;
/*----------------------------------------------------------------------------*/
void igrab_init(my1image_grab_t* igrab);
void igrab_free(my1image_grab_t* igrab);
void igrab_grab(my1image_grab_t* igrab);
void igrab_grab_default(my1image_grab_t* igrab);
/*----------------------------------------------------------------------------*/
int igrab_grab_image_file(void* data, void* that, void* xtra);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_GRAB_H__ */
/*----------------------------------------------------------------------------*/
