/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_HISTH__
#define __MY1IMAGE_HISTH__
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
#include "my1image_stat.h"
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
/*----------------------------------------------------------------------------*/
#define HISTSIZE_WIDTH 640
#define HISTSIZE_HEIGHT 480
#define HISTSIZE_BORDER 30
#define HISTSIZE_BCHART 1
#define HISTSIZE_BCHART_SKIP 1
/*----------------------------------------------------------------------------*/
typedef struct _my1image_hist_t
{
	GtkWidget *donext, *dohist, *dostxt;
	guint idstxt, dohide;
	my1image_appw_t* appwin;
	my1image_histogram_t hist; /* histogram */
}
my1image_hist_t;
/*----------------------------------------------------------------------------*/
void image_hist_init(my1image_hist_t* ihist, my1image_appw_t* appw);
void image_hist_free(my1image_hist_t* ihist);
void image_hist_make(my1image_hist_t* ihist);
void image_hist_show(my1image_hist_t* ihist);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_HISTH__ */
/*----------------------------------------------------------------------------*/
