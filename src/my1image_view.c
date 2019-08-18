/*----------------------------------------------------------------------------*/
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* iview)
{
	iview->window = 0x0;
	iview->canvas = 0x0;
	iview->domenu = 0x0;
	iview->dostat = 0x0;
	iview->idstat = 0;
	iview->idtime = 0;
	iview->width = -1;
	iview->height = -1;
	iview->doquit = 0;
	iview->goquit = 0;
	iview->gofull = 0;
	iview->draw_more = 0x0;
	iview->draw_more_data = 0x0;
	iview->dodraw = 0x0;
	iview->image = 0x0;
	iview->ishow = 0x0;
	image_init(&iview->buff);
	image_init(&iview->size);
}
/*----------------------------------------------------------------------------*/
void image_view_free(my1image_view_t* iview)
{
	image_free(&iview->buff);
	image_free(&iview->size);
}
/*----------------------------------------------------------------------------*/
void image_view_full(my1image_view_t* iview, int full)
{
	if (full) gtk_window_fullscreen(GTK_WINDOW(iview->window));
	else gtk_window_unfullscreen(GTK_WINDOW(iview->window));
}
/*----------------------------------------------------------------------------*/
gboolean on_done_all(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	if (view->goquit)
		gtk_main_quit();
	else
		view->doquit = 1;
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gboolean on_draw_expose(GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data)
{
	my1image_view_t* view = (my1image_view_t*) user_data;
	image_view_draw(view,view->image);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gboolean on_canvas_resize(GtkWidget *widget, GtkAllocation *allocation,
	gpointer user_data)
{
/**
	my1image_view_t* view = (my1image_view_t*) user_data;
	printf("Resize: %d x %d\n",allocation->width,allocation->height);
*/
	return TRUE;
}
/*----------------------------------------------------------------------------*/
void image_view_make(my1image_view_t* iview, my1image_t* that)
{
	GtkWidget* vbox;
	/* nothing to do if window/canvas already created */
	if (iview->window||iview->canvas) return;
	/* check if assigned new image */
	if (that) iview->image = that;
	/* must have image */
	if (!iview->image) return;
	/* check default size */
	if (iview->width<0||iview->height<0)
	{
		iview->width = iview->image->width;
		iview->height = iview->image->height;
		image_make(&iview->buff,iview->height,iview->width);
	}
	/* create window */
	iview->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(iview->window),"MY1Image Viewer");
	gtk_window_set_default_size(GTK_WINDOW(iview->window),
		iview->width,iview->height);
	gtk_window_set_position(GTK_WINDOW(iview->window),GTK_WIN_POS_CENTER);
	/**gtk_window_set_resizable(GTK_WINDOW(iview->window),FALSE);*/
	/* check if we need to go full screen */
	image_view_full(iview,iview->gofull);
	/* container box */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(iview->window),vbox);
	/* create draw canvas - deprecated in gtk3 in favor of cairo? */
	iview->canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request(iview->canvas,iview->width,iview->height);
	gtk_box_pack_start(GTK_BOX(vbox),iview->canvas,TRUE,TRUE,0);
	gtk_widget_grab_focus(iview->canvas); /* just in case */
 	/* create status bar - if NOT starting in fullscreen mode! */
 	if (!iview->gofull)
 	{
		iview->dostat = gtk_statusbar_new();
		iview->idstat = gtk_statusbar_get_context_id(
			(GtkStatusbar*)iview->dostat,"MY1ImageViewStat");
		gtk_box_pack_start(GTK_BOX(vbox),iview->dostat,FALSE,FALSE,0);
	}
	/* connect event handlers */
	g_signal_connect(G_OBJECT(iview->window),"delete-event",
		G_CALLBACK(on_done_all),(gpointer)iview);
	g_signal_connect(G_OBJECT(iview->canvas),"expose-event",
		G_CALLBACK(on_draw_expose),(gpointer)iview);
	g_signal_connect(G_OBJECT(iview->canvas),"size-allocate",
		G_CALLBACK(on_canvas_resize),(gpointer)iview);
	/* show main window */
	gtk_widget_show_all(iview->window);
}
/*----------------------------------------------------------------------------*/
void image_view_draw(my1image_view_t* iview, my1image_t* that)
{
	GdkPixbuf *dotemp;
	int chkh, chkw;
	/* check if assigned new image */
	if (that) iview->image = that;
	/* must have image - and canvas! */
	if (!iview->image||!iview->canvas) return;
	/* colormode abgr32 for gdk function */
	image_copy_color2bgr(&iview->buff,iview->image);
	iview->ishow = &iview->buff;
	/* check canvas size and the need to resize */
	chkw = iview->canvas->allocation.width;
	chkh = iview->canvas->allocation.height;
	if (chkw!=iview->ishow->width||chkh!=iview->ishow->height)
		iview->ishow = image_size_this(iview->ishow,&iview->size,chkh,chkw);
	/* draw! */
	iview->dodraw = gdk_cairo_create(iview->canvas->window);
	dotemp = gdk_pixbuf_new_from_data((const guchar*)iview->ishow->data,
		GDK_COLORSPACE_RGB,TRUE,8,iview->ishow->width,iview->ishow->height,
		iview->ishow->width<<2,0x0,0x0);
	gdk_cairo_set_source_pixbuf(iview->dodraw,dotemp,0,0);
	cairo_paint(iview->dodraw);
	if (iview->draw_more) iview->draw_more((void*)iview);
	cairo_destroy(iview->dodraw);
}
/*----------------------------------------------------------------------------*/
void image_view_name(my1image_view_t* iview,const char* name)
{
	/* must have window! */
	if (!iview->window) return;
	/* set title */
	gtk_window_set_title(GTK_WINDOW(iview->window),name);
}
/*----------------------------------------------------------------------------*/
void image_view_stat_show(my1image_view_t* iview, const char* mesg)
{
	if (!iview->dostat) return;
	/* remove all previous */
	gtk_statusbar_remove_all((GtkStatusbar*)iview->dostat,iview->idstat);
	/* show new */
	gtk_statusbar_push((GtkStatusbar*)iview->dostat,iview->idstat,mesg);
}
/*----------------------------------------------------------------------------*/
gboolean on_timer_status(gpointer data)
{
	my1image_view_t *iview = (my1image_view_t*) data;
	iview->idtime = 0;
	gtk_statusbar_pop((GtkStatusbar*)iview->dostat,iview->idstat);
	return 0; /* a one-shot */
}
/*----------------------------------------------------------------------------*/
void image_view_stat_time(my1image_view_t* iview, const char* mesg, int secs)
{
	if (!iview->dostat) return;
	if (iview->idtime)
	{
		g_source_remove(iview->idtime);
		gtk_statusbar_pop((GtkStatusbar*)iview->dostat,iview->idstat);
	}
	gtk_statusbar_push((GtkStatusbar*)iview->dostat,iview->idstat,mesg);
	iview->idtime = g_timeout_add_seconds(secs,on_timer_status,(gpointer)iview);
}
/*----------------------------------------------------------------------------*/
guint image_view_stat_push(my1image_view_t* iview, const char* mesg)
{
	if (!iview->dostat) return 0;
	return gtk_statusbar_push((GtkStatusbar*)iview->dostat,iview->idstat,mesg);
}
/*----------------------------------------------------------------------------*/
void image_view_stat_pop(my1image_view_t* iview)
{
	if (!iview->dostat) return;
	gtk_statusbar_pop((GtkStatusbar*)iview->dostat,iview->idstat);
}
/*----------------------------------------------------------------------------*/
void image_view_stat_remove(my1image_view_t* iview, guint mesg_id)
{
	if (!iview->dostat) return;
	gtk_statusbar_remove((GtkStatusbar*)iview->dostat,iview->idstat,mesg_id);
}
/*----------------------------------------------------------------------------*/
