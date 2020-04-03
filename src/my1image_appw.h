/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_APPWH__
#define __MY1IMAGE_APPWH__
/*----------------------------------------------------------------------------*/
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
#define MAIN_WINDOW_TITLE "MY1Image Viewer"
#define MAIN_WINDOW_CLASS "my1image-appw"
/*----------------------------------------------------------------------------*/
#define CLASSNAME_SIZE 32
#define STATUS_TIMEOUT 5
/*----------------------------------------------------------------------------*/
#define REDRAW 0x0
/*----------------------------------------------------------------------------*/
typedef struct _my1image_appw_t
{
	GtkWidget *window, *domenu, *dostat;
	guint idstat, idtime; /* gtk stuffs */
	int doquit; /* quit request flag */
	int goquit; /* flag to call gtk_quit_main in on_done_all */
	int gofull; /* full screen request/status flag  */
	int doshow; /* flag to use external image */
	char classname[CLASSNAME_SIZE]; /* classname */
	my1image_t buff, main, *show; /* buffered image data */
	my1image_view_t view; /* single image view */
}
my1image_appw_t;
/*----------------------------------------------------------------------------*/
void image_appw_init(my1image_appw_t* appw);
void image_appw_free(my1image_appw_t* appw);
void image_appw_full(my1image_appw_t* appw, int full);
void image_appw_make(my1image_appw_t* appw, my1image_t* that);
void image_appw_draw(my1image_appw_t* appw, my1image_t* that);
void image_appw_name(my1image_appw_t* appw, const char* name);
void image_appw_name_class(my1image_appw_t* appw,const char* name);
void image_appw_stat_show(my1image_appw_t* appw, const char* mesg);
void image_appw_stat_time(my1image_appw_t* appw, const char* mesg, int secs);
guint image_appw_stat_push(my1image_appw_t* appw, const char* mesg);
void image_appw_stat_pop(my1image_appw_t* appw);
void image_appw_stat_remove(my1image_appw_t* appw, guint mesg_id);
void image_appw_domenu(my1image_appw_t* appw);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_APPWH__ */
/*----------------------------------------------------------------------------*/
