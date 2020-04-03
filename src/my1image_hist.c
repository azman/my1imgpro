/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_HISTC__
#define __MY1IMAGE_HISTC__
/*----------------------------------------------------------------------------*/
#include "my1image_hist.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
void image_hist_init(my1image_hist_t* ihist, my1image_appw_t* appw)
{
	ihist->donext = 0x0;
	ihist->dohist = 0x0;
	ihist->dostxt = 0x0;
	ihist->idstxt = 0;
	ihist->dohide = 1;
	ihist->appwin = appw;
}
/*----------------------------------------------------------------------------*/
void image_hist_free(my1image_hist_t* ihist)
{
	/* nothing to do? */
}
/*----------------------------------------------------------------------------*/
gboolean on_done_hist(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	my1image_hist_t* hist = (my1image_hist_t*) data;
	gtk_widget_hide(hist->donext);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gboolean on_draw_hist_expose(GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data)
{
	int loop, temp;
	gchar *buff = 0x0;
	my1image_t gray;
	my1image_hist_t* v = (my1image_hist_t*) user_data;
	my1image_histogram_t* h = &v->hist;
	my1image_t* image = v->appwin->view.image;
	cairo_t *dodraw = gdk_cairo_create(v->dohist->window);
	/* declared as double in case we want to use scaling later! */
	double offs_x = (double)HISTSIZE_BORDER;
	double offs_y = (double)HISTSIZE_BORDER;
	double ends_x = (double)(HISTSIZE_WIDTH-HISTSIZE_BORDER);
	double ends_y = (double)(HISTSIZE_HEIGHT-HISTSIZE_BORDER);
	double bars_w = (double)HISTSIZE_BCHART;
	double bars_d = (double)HISTSIZE_BCHART_SKIP;
	double diff_y = ends_y-offs_y;
	double next_x, next_y, size_y, yscale;
	/* clear statusbar */
	gtk_statusbar_remove_all((GtkStatusbar*)v->dostxt,v->idstxt);
	/* white background */
	cairo_set_source_rgb(dodraw,1.0,1.0,1.0);
	cairo_paint(dodraw);
	/* test text - blue! */
	cairo_set_source_rgb(dodraw,0.0,0.0,1.0);
	cairo_move_to(dodraw,40,20);
	cairo_set_font_size(dodraw,12);
	cairo_show_text(dodraw,"HISTOGRAM CHART (Grayscale)");
	if (image)
	{
		image_init(&gray);
		if (image->mask!=IMASK_GRAY)
		{
			image_copy(&gray,image);
			image_grayscale(&gray);
			image = &gray;
		}
		image_get_histogram(image,h);
		histogram_get_threshold(h);
		/* scale drawing area */
		/**cairo_scale(dodraw,HISTSIZE_WIDTH,HISTSIZE_HEIGHT);*/
		/* x and y axis */
		cairo_set_source_rgb(dodraw,0.0,0.0,1.0);
		cairo_set_line_width(dodraw,1);
		cairo_move_to(dodraw,ends_x,ends_y);
		cairo_line_to(dodraw,offs_x,ends_y);
		cairo_line_to(dodraw,offs_x,offs_y);
		cairo_stroke(dodraw);
		/* draw data bars */
		cairo_set_source_rgb(dodraw,0.0,1.0,0.0);
		next_x = offs_x + bars_d;
		yscale = diff_y/h->maxvalue;
		for (loop=0;loop<GRAYLEVEL;loop++)
		{
			temp = h->count[loop];
			size_y = yscale*temp;
			next_y = ends_y-size_y;
			if (bars_w>1)
			{
				cairo_rectangle(dodraw,next_x,next_y,bars_w,size_y);
				cairo_fill(dodraw);
			}
			else
			{
				cairo_move_to(dodraw,next_x,ends_y);
				cairo_line_to(dodraw,next_x,next_y);
				cairo_stroke(dodraw);
			}
			if (loop==h->threshold)
			{
				cairo_set_source_rgb(dodraw,0.0,0.0,0.0);
				cairo_move_to(dodraw,next_x,ends_y);
				cairo_line_to(dodraw,next_x,ends_y+10);
				cairo_stroke(dodraw);
				cairo_set_source_rgb(dodraw,0.0,1.0,0.0);
			}
			next_x += bars_w+bars_d;
		}
		buff = g_strdup_printf("Len:%d,Max:%d(@%d),"
			"Min:%d(@%d),Chk:%d(@%d),Dif:%d,Th@%d",
			image->size,h->maxvalue,h->maxindex,
			h->minvalue,h->minindex,h->chkvalue,h->chkindex,
			(h->maxvalue-h->minvalue),h->threshold);
		gtk_statusbar_push((GtkStatusbar*)v->dostxt,v->idstxt,buff);
/*
		cairo_set_source_rgb(dodraw,0.0,0.0,1.0);
		cairo_move_to(dodraw,next_x,ends_y);
		cairo_line_to(dodraw,next_x,offs_y);
*/
		cairo_stroke(dodraw);
		image_free(&gray);
	}
	cairo_destroy(dodraw);
	if (buff) g_free(buff);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
void image_hist_make(my1image_hist_t* ihist)
{
	GtkWidget* vbox;
	/* skip if histogram window already created */
	if (ihist->donext) return;
	/* create histogram window */
	ihist->donext = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ihist->donext),"Histogram Viewer");
	gtk_window_set_default_size(GTK_WINDOW(ihist->donext),
		HISTSIZE_WIDTH,HISTSIZE_HEIGHT);
	gtk_window_set_resizable(GTK_WINDOW(ihist->donext),FALSE);
	/* container box */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(ihist->donext),vbox);
	/* create draw canvas - deprecated in gtk3 in favor of cairo? */
	ihist->dohist = gtk_drawing_area_new();
	gtk_widget_set_size_request(ihist->dohist,HISTSIZE_WIDTH,HISTSIZE_HEIGHT);
	gtk_box_pack_start(GTK_BOX(vbox),ihist->dohist,TRUE,TRUE,0);
	gtk_widget_grab_focus(ihist->dohist); /* just in case */
 	/* create status bar */
	ihist->dostxt = gtk_statusbar_new();
	ihist->idstxt = gtk_statusbar_get_context_id((GtkStatusbar*)ihist->dostxt,
		"MY1ImageHistStat");
	gtk_box_pack_start(GTK_BOX(vbox),ihist->dostxt,FALSE,FALSE,0);
	/* connect event handlers */
	g_signal_connect(G_OBJECT(ihist->donext),"delete-event",
		G_CALLBACK(on_done_hist),(gpointer)ihist);
	g_signal_connect(G_OBJECT(ihist->dohist),"expose-event",
		G_CALLBACK(on_draw_hist_expose),(gpointer)ihist);
}
/*----------------------------------------------------------------------------*/
void image_hist_show(my1image_hist_t* ihist)
{
	/* skip if histogram window not available */
	if (!ihist->donext) return;
	/* show histogram if requested and image is grayscale */
	if (ihist->dohide)
		gtk_widget_hide(ihist->donext);
	else
	{
		GdkRectangle rect;
		/* update histogram window position & show it! */
		gdk_window_get_frame_extents(ihist->appwin->window->window,&rect);
		gtk_window_move(GTK_WINDOW(ihist->donext),rect.x+rect.width,rect.y);
		gtk_widget_show_all(ihist->donext);
		gtk_widget_queue_draw(ihist->donext);
	}
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_HISTC__ */
/*----------------------------------------------------------------------------*/
