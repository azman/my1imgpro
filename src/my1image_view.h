/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_VIEWH__
#define __MY1IMAGE_VIEWH__
/*----------------------------------------------------------------------------*/
#include "my1image_crgb.h"
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
/*----------------------------------------------------------------------------*/
typedef struct _my1image_view_t
{
	GtkWidget *canvas;
	cairo_surface_t *dosurf; /* drawing surface! */
	cairo_t *dodraw; /* cairo drawing, needed by draw_more! */
	my1dotask_t domore; /* user custom draw function */
	my1image_t *image; /* pure pointer to image object */
	my1image_t buff; /* local buffer coz gdk uses abgr, not argb! */
	int aspect; /* request to maintain aspect ratio? */
}
my1image_view_t;
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* view);
void image_view_free(my1image_view_t* view);
void image_view_make(my1image_view_t* view, my1image_t* that);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_VIEWH__ */
/*----------------------------------------------------------------------------*/
