/*----------------------------------------------------------------------------*/
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* iview)
{
	iview->window = 0x0;
	iview->canvas = 0x0;
	iview->domenu = 0x0;
	iview->donext = 0x0;
	iview->dohist = 0x0;
	iview->idstat = 0;
	iview->idmesg = 0;
	iview->idtime = 0;
	iview->width = -1;
	iview->height = -1;
	iview->gohist = 0;
	iview->image = 0x0;
	image_init(&iview->buff);
}
/*----------------------------------------------------------------------------*/
void image_view_free(my1image_view_t* iview)
{
	image_free(&iview->buff);
}
/*----------------------------------------------------------------------------*/
gboolean on_done_hist(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	gtk_widget_hide(view->donext);
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
gboolean on_draw_hist_expose(GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data)
{
	my1image_view_t* v = (my1image_view_t*) user_data;
	cairo_t *dodraw = gdk_cairo_create(v->dohist->window);
	/* white background */
	cairo_set_source_rgb(dodraw, 1.0, 1.0, 1.0);
	cairo_paint(dodraw);
	if (v->image->mask==IMASK_GRAY)
	{
		image_get_histogram(v->image,&v->hist);
	}
	/* test text - blue! */
	cairo_set_source_rgb(dodraw, 0.0, 0.0, 1.0);
	cairo_move_to(dodraw, 30, 30);
	cairo_set_font_size(dodraw, 15);
	cairo_show_text(dodraw, "HAHA");
	cairo_destroy(dodraw);
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
	gtk_window_set_resizable(GTK_WINDOW(iview->window),FALSE);
	/* container box */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(iview->window),vbox);
	/* create draw canvas - deprecated in gtk3 in favor of cairo? */
	iview->canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request(iview->canvas,iview->width,iview->height);
	gtk_box_pack_start(GTK_BOX(vbox),iview->canvas,TRUE,TRUE,0);
	gtk_widget_grab_focus(iview->canvas); /* just in case */
 	/* create status bar */
	iview->dostat = gtk_statusbar_new();
	iview->idstat = gtk_statusbar_get_context_id((GtkStatusbar*)iview->dostat,
		"MY1ImageViewStat");
	image_view_stat_push(iview,"MY1Image Viewer Status");
	gtk_box_pack_start(GTK_BOX(vbox),iview->dostat,FALSE,FALSE,0);
	/* connect event handlers */
	g_signal_connect(G_OBJECT(iview->window),"destroy",
		G_CALLBACK(gtk_main_quit),0x0);
	g_signal_connect(G_OBJECT(iview->canvas),"expose-event",
		G_CALLBACK(on_draw_expose),(gpointer)iview);
	/* show main window */
	gtk_widget_show_all(iview->window);
}
/*----------------------------------------------------------------------------*/
void image_view_draw(my1image_view_t* iview, my1image_t* that)
{
	/* check if assigned new image */
	if (that) iview->image = that;
	/* must have image - and canvas! */
	if (!iview->image||!iview->canvas) return;
	/* check the need to resize window */
	if (iview->width!=iview->image->width||iview->height!=iview->image->height)
	{
		gtk_widget_set_size_request(iview->canvas,
			iview->image->width,iview->image->height);
		iview->width = iview->image->width;
		iview->height = iview->image->height;
		image_make(&iview->buff,iview->height,iview->width);
	}
	/* colormode abgr32 for gdk function */
	image_copy_color2bgr(&iview->buff,iview->image);
	/* draw! */
	gdk_draw_rgb_32_image(iview->canvas->window,
		iview->canvas->style->fg_gc[GTK_STATE_NORMAL],
		0, 0, iview->buff.width, iview->buff.height, GDK_RGB_DITHER_NONE,
		(const guchar*)iview->buff.data, iview->buff.width*4);
	/* check histogram */
	image_view_show_hist(iview);
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
void image_view_make_hist(my1image_view_t* iview)
{
	/* skip if histogram window already created */
	if (iview->donext) return;
	/* create histogram window */
	iview->donext = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(iview->donext),"Histogram Viewer");
	gtk_window_set_default_size(GTK_WINDOW(iview->donext),
		HISTSIZE_W,HISTSIZE_H);
	gtk_window_set_resizable(GTK_WINDOW(iview->donext),FALSE);
	/* create draw canvas - deprecated in gtk3 in favor of cairo? */
	iview->dohist = gtk_drawing_area_new();
	gtk_widget_set_size_request(iview->dohist,HISTSIZE_W,HISTSIZE_H);
	gtk_container_add(GTK_CONTAINER(iview->donext),iview->dohist);
	/* connect event handlers */
	g_signal_connect(G_OBJECT(iview->donext),"delete-event",
		G_CALLBACK(on_done_hist),(gpointer)iview);
	g_signal_connect(G_OBJECT(iview->dohist),"expose-event",
		G_CALLBACK(on_draw_hist_expose),(gpointer)iview);
}
/*----------------------------------------------------------------------------*/
void image_view_show_hist(my1image_view_t* iview)
{
	/* skip if histogram window not available */
	if (!iview->donext) return;
	/* show histogram if requested and image is grayscale */
	if (iview->gohist&&iview->image->mask==IMASK_GRAY)
	{
		GdkRectangle rect;
		/* update histogram window position & show it! */
		gdk_window_get_frame_extents(iview->window->window,&rect);
		gtk_window_move(GTK_WINDOW(iview->donext),rect.x+rect.width,rect.y);
		gtk_widget_show_all(iview->donext);
	}
	else
	{
		gtk_widget_hide(iview->donext);
	}
}
/*----------------------------------------------------------------------------*/
void image_view_stat_push(my1image_view_t* iview, const char* stat)
{
	iview->idmesg = gtk_statusbar_push((GtkStatusbar*)iview->dostat,
		iview->idstat,stat);
}
/*----------------------------------------------------------------------------*/
void image_view_stat_pop(my1image_view_t* iview)
{
	gtk_statusbar_pop((GtkStatusbar*)iview->dostat,iview->idstat);
	iview->idmesg = 0;
}
/*----------------------------------------------------------------------------*/
void image_view_stat_remove(my1image_view_t* iview, guint mesg_id)
{
	gtk_statusbar_remove((GtkStatusbar*)iview->dostat,iview->idstat,mesg_id);
}
/*----------------------------------------------------------------------------*/
gboolean on_timer_status(gpointer data)
{
	my1image_view_t *view = (my1image_view_t*) data;
	if (view->idtime)
	{
		image_view_stat_remove(view,view->idtime);
		view->idtime = 0;
	}
	return 0; /* a one-shot */
}
/*----------------------------------------------------------------------------*/
void image_view_stat_time(my1image_view_t* iview, const char* stat, int secs)
{
	image_view_stat_push(iview,stat);
	iview->idtime = iview->idmesg;
	g_timeout_add_seconds(secs,on_timer_status,(gpointer)iview);
}
/*----------------------------------------------------------------------------*/
