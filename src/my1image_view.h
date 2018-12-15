/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_VIEWH__
#define __MY1IMAGE_VIEWH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
/*----------------------------------------------------------------------------*/
typedef struct _my1image_view_t
{
	GtkWidget *window, *canvas; /* gtk stuffs */
	int width, height; /* current view size */
	my1image_t *image; /* pure pointer to image object */
}
my1image_view_t;
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* object);
void image_view_free(my1image_view_t* object);
void image_view_make(my1image_view_t* object);
void image_view_draw(my1image_view_t* object);
void image_view_name(my1image_view_t* object, const char *name);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
