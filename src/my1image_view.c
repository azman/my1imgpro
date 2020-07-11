/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_VIEWC__
#define __MY1IMAGE_VIEWC__
/*----------------------------------------------------------------------------*/
#include "my1image_view.h"
#include "my1image_area.h"
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* view)
{
	view->canvas = 0x0;
	view->dosurf = 0x0;
	view->dodraw = 0x0;
	itask_make(&view->domore,0x0,0x0);
	view->image = 0x0;
	view->aspect = 0;
	image_init(&view->buff);
}
/*----------------------------------------------------------------------------*/
void image_view_free(my1image_view_t* view)
{
	/* canvas stuff will be taken care of by gtk? */
	image_free(&view->buff);
}
/*----------------------------------------------------------------------------*/
gboolean on_conf_callback(GtkWidget *widget, GdkEventConfigure *event,
	gpointer data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	if (view->dosurf) cairo_surface_destroy(view->dosurf);
	view->dosurf = gdk_window_create_similar_surface(
		gtk_widget_get_window(view->canvas), CAIRO_CONTENT_COLOR,
		gtk_widget_get_allocated_width(view->canvas),
		gtk_widget_get_allocated_height(view->canvas));
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gboolean on_draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	GdkPixbuf *dotemp;
	int chkh, chkw;
	my1image_t mods, temp, *show;
	my1image_view_t* view = (my1image_view_t*) data;
	my1image_area_t area;
	/* must have image - and canvas! */
	if (!view->image||!view->canvas) return FALSE;
	show = view->image;
	image_init(&mods);
	/* check canvas size and the need to resize */
	chkw = gtk_widget_get_allocated_width(view->canvas);
	chkh = gtk_widget_get_allocated_height(view->canvas);
	/**printf("-- DoDraw: %d x %d\n",chkw,chkh);*/
	if (chkw!=show->cols||chkh!=show->rows)
	{
		image_init(&temp);
		if (view->aspect)
		{
			image_area_make(&area,0,0,chkh,chkw);
			image_size_aspect(show,&area);
			if (area.hval!=chkh||area.wval!=chkw)
			{
				image_size_this(show,&mods,area.hval,area.wval);
				image_make(&temp,chkh,chkw);
				image_fill(&temp,encode_rgb(0,0,0));
				temp.mask = IMASK_COLOR;
				image_set_area(&temp,&mods,&area);
				show = &temp;
			}
		}
		show = image_size_this(show,&mods,chkh,chkw);
		/**printf("-- DoSize: %d x %d (%p)\n",show->cols,show->rows,show);*/
		image_free(&temp);
	}
	/* colormode abgr32 for gdk function */
	image_copy_color2bgr(&view->buff,show);
	show = &view->buff;
	/* draw! */
	dotemp = gdk_pixbuf_new_from_data((const guchar*)show->data,
		GDK_COLORSPACE_RGB,TRUE,8,show->cols,show->rows,show->cols<<2,0x0,0x0);
	/*printf("-- ReDraw: %d x %d (%p)\n",show->cols,show->rows,show);*/
	view->dodraw = cr; /* required by draw_more */
	gdk_cairo_set_source_pixbuf(view->dodraw,dotemp,0,0);
	cairo_paint(view->dodraw);
	itask_exec(&view->domore,(void*)view,0x0);
	image_free(&mods);
	/* enable user to resize canvas to a smaller size */
	gtk_widget_set_size_request(view->canvas,1,1);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
void image_view_make(my1image_view_t* view, my1image_t* that)
{
	if (!view->canvas)
	{
		view->canvas = gtk_drawing_area_new();
		/* connect event handlers */
		g_signal_connect(G_OBJECT(view->canvas),"draw",
			G_CALLBACK(on_draw_callback),(gpointer)view);
		g_signal_connect(G_OBJECT(view->canvas),"configure-event",
			G_CALLBACK(on_conf_callback),(gpointer)view);
	}
	/* to make sure we get full size on canvas */
	gtk_widget_set_size_request(view->canvas,that->cols,that->rows);
	view->image = that;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_VIEWC__ */
/*----------------------------------------------------------------------------*/
