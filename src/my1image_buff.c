/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_BUFFC__
#define __MY1IMAGE_BUFFC__
/*----------------------------------------------------------------------------*/
#include "my1image_buff.h"
/*----------------------------------------------------------------------------*/
void buffer_init(my1ibuff_t* ibuff)
{
	image_area_make(&ibuff->region,0,0,0,0);
	image_area_make(&ibuff->select,0,0,0,0);
	ibuff->curr = &ibuff->main;
	ibuff->next = &ibuff->buff;
	/** ibuff->temp = 0x0; / * not necessary */
	ibuff->iref = 0x0;
	image_init(&ibuff->main);
	image_init(&ibuff->buff);
	image_init(&ibuff->xtra);
	ibuff->chk1 = 0;
	ibuff->chk2 = 0;
}
/*----------------------------------------------------------------------------*/
void buffer_free(my1ibuff_t* ibuff)
{
	image_free(&ibuff->main);
	image_free(&ibuff->buff);
	image_free(&ibuff->xtra);
}
/*----------------------------------------------------------------------------*/
void buffer_size(my1ibuff_t* ibuff, int height, int width)
{
	image_make(&ibuff->main,height,width);
	image_make(&ibuff->buff,height,width);
}
/*----------------------------------------------------------------------------*/
void buffer_size_all(my1ibuff_t* ibuff, int height, int width)
{
	image_make(&ibuff->main,height,width);
	image_make(&ibuff->buff,height,width);
	image_make(&ibuff->xtra,height,width);
}
/*----------------------------------------------------------------------------*/
void buffer_swap(my1ibuff_t* ibuff)
{
	ibuff->temp = ibuff->curr;
	ibuff->curr = ibuff->next;
	ibuff->next = ibuff->temp;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_BUFFC__ */
/*----------------------------------------------------------------------------*/
