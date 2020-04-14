/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_GRAB_C__
#define __MY1IMAGE_GRAB_C__
/*----------------------------------------------------------------------------*/
#include "my1image_grab.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
/*----------------------------------------------------------------------------*/
void igrab_init(my1image_grab_t* igrab)
{
	igrab->flag = 0;
	igrab->pick = 0x0;
	image_init(&igrab->buff);
	igrab->grab = 0x0;
	appw_handler_make(&igrab->do_grab,0x0,0x0);
}
/*----------------------------------------------------------------------------*/
void igrab_free(my1image_grab_t* igrab)
{
	image_free(&igrab->buff);
}
/*----------------------------------------------------------------------------*/
void igrab_grab(my1image_grab_t* igrab)
{
	if (igrab->do_grab.task)
		igrab->do_grab.task((void*)igrab);
	else
		igrab->flag |= IGRAB_FLAG_TASK_ERROR;
}
/*----------------------------------------------------------------------------*/
void igrab_grab_default(my1image_grab_t* igrab)
{
	if (!igrab->do_grab.task)
		igrab->do_grab.task = igrab_grab_image_file;
}
/*----------------------------------------------------------------------------*/
void igrab_grab_image_file(void* itask)
{
	my1image_grab_t* igrab = (my1image_grab_t*) itask;
	if (!igrab->grab) igrab->grab = &igrab->buff;
	if (!igrab->pick)
	{
		igrab->flag |= IGRAB_FLAG_FILE_ERROR;
		return;
	}
	if (image_load(igrab->grab,igrab->pick)<0)
		igrab->flag |= IGRAB_FLAG_LOAD_ERROR;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_GRAB_C__ */
/*----------------------------------------------------------------------------*/
