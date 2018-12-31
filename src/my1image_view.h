/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_VIEWH__
#define __MY1IMAGE_VIEWH__
/*----------------------------------------------------------------------------*/
#include "my1image.h"
#include "my1image_util.h"
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
/*----------------------------------------------------------------------------*/
#define HISTSIZE_WIDTH 640
#define HISTSIZE_HEIGHT 480
#define HISTSIZE_BORDER 30
#define HISTSIZE_BCHART 1
#define HISTSIZE_BCHART_SKIP 1
/*----------------------------------------------------------------------------*/
typedef void (*pdraw_more)(void*);
/*----------------------------------------------------------------------------*/
typedef struct _my1image_view_t
{
	GtkWidget *window, *canvas, *domenu;
	GtkWidget *donext, *dohist, *dostat, *dostxt;
	guint idstat, idstxt, idtime; /* gtk stuffs */
	int width, height; /* current view size */
	int gohist, doquit; /* histogram/quit request flag */
	int gofull, isfull; /* full screen request/status flag */
	int aspect; /* flag to keep aspect ratio! */
	pdraw_more draw_more; /* user draw function */
	void* draw_more_data; /* data for user draw function */
	cairo_t *dodraw; /* cairo drawing, needed by draw_more! */
	my1image_t *image; /* pure pointer to image object */
	my1image_t buff; /* local buffer coz gdk uses abgr, not argb! */
	my1image_t size; /* local buffer for fullscreen! */
	my1image_histogram_t hist; /* histogram */
}
my1image_view_t;
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* iview);
void image_view_free(my1image_view_t* iview);
void image_view_make(my1image_view_t* iview, my1image_t* that);
void image_view_draw(my1image_view_t* iview, my1image_t* that);
void image_view_name(my1image_view_t* iview, const char* name);
void image_view_make_hist(my1image_view_t* iview); /* to enable histogram */
void image_view_show_hist(my1image_view_t* iview);
void image_view_stat_show(my1image_view_t* iview, const char* mesg);
void image_view_stat_time(my1image_view_t* iview, const char* mesg, int secs);
guint image_view_stat_push(my1image_view_t* iview, const char* mesg);
void image_view_stat_pop(my1image_view_t* iview);
void image_view_stat_remove(my1image_view_t* iview, guint mesg_id);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
