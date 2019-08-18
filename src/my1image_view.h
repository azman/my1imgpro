/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_VIEWH__
#define __MY1IMAGE_VIEWH__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
/*----------------------------------------------------------------------------*/
typedef void (*pdraw_more)(void*);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_view_t
{
	GtkWidget *window, *canvas, *domenu, *dostat;
	guint idstat, idtime; /* gtk stuffs */
	int width, height; /* current view size */
	int doquit; /* quit request flag */
	int goquit; /* flag to call gtk_quit_main in on_done_all */
	int gofull; /* full screen request/status flag  */
	pdraw_more draw_more; /* user draw function */
	void* draw_more_data; /* data for user draw function */
	cairo_t *dodraw; /* cairo drawing, needed by draw_more! */
	my1image_t *image; /* pure pointer to image object */
	my1image_t *ishow; /* pointer to image on display */
	my1image_t buff; /* local buffer coz gdk uses abgr, not argb! */
	my1image_t size; /* local buffer for fullscreen! */
}
my1image_view_t;
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* iview);
void image_view_free(my1image_view_t* iview);
void image_view_full(my1image_view_t* iview, int full);
void image_view_make(my1image_view_t* iview, my1image_t* that);
void image_view_draw(my1image_view_t* iview, my1image_t* that);
void image_view_name(my1image_view_t* iview, const char* name);
void image_view_stat_show(my1image_view_t* iview, const char* mesg);
void image_view_stat_time(my1image_view_t* iview, const char* mesg, int secs);
guint image_view_stat_push(my1image_view_t* iview, const char* mesg);
void image_view_stat_pop(my1image_view_t* iview);
void image_view_stat_remove(my1image_view_t* iview, guint mesg_id);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
