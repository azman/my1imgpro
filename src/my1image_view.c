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
	view->aspect = 0;
	view->draw_more = 0x0;
	view->draw_more_data = 0x0;
	view->dodraw = 0x0;
	view->image = 0x0;
	image_init(&view->buff);
}
/*----------------------------------------------------------------------------*/
void image_view_free(my1image_view_t* view)
{
	/* canvas stuff will be taken care of by gtk? */
	image_free(&view->buff);
}
/*----------------------------------------------------------------------------*/
gboolean on_draw_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	image_view_draw(view,view->image);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
/*
gboolean on_canvas_resize(GtkWidget *widget, GtkAllocation *allocation,
	gpointer user_data)
{
	my1image_view_t* view = (my1image_view_t*) user_data;
	printf("-- Resize: %d x %d\n",allocation->width,allocation->height);
	return TRUE;
}
*/
/*----------------------------------------------------------------------------*/
void image_view_make(my1image_view_t* view, my1image_t* that)
{
	if (!view->canvas)
	{
		view->canvas = gtk_drawing_area_new();
		/* connect event handlers */
		g_signal_connect(G_OBJECT(view->canvas),"expose-event",
			G_CALLBACK(on_draw_expose),(gpointer)view);
/*
		g_signal_connect(G_OBJECT(view->canvas),"size-allocate",
			G_CALLBACK(on_canvas_resize),(gpointer)view);
*/
	}
	view->image = that;
}
/*----------------------------------------------------------------------------*/
void image_view_draw(my1image_view_t* view, my1image_t* that)
{
	GtkAllocation alloc;
	GdkPixbuf *dotemp;
	int chkh, chkw;
	my1image_t mods, temp, *show;
	my1image_area_t area;
	/* check if assigned new image */
	if (that) view->image = that;
	/* must have image - and canvas! */
	if (!view->image||!view->canvas) return;
	show = view->image;
	image_init(&mods);
	/* check canvas size and the need to resize */
	gtk_widget_get_allocation(view->canvas,&alloc);
	chkw = alloc.width;
	chkh = alloc.height;
	/**printf("DoDraw: %d x %d\n",chkw,chkh);*/
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
		/**printf("Dosize: %d x %d (%p)\n",show->cols,show->rows,show);*/
		image_free(&temp);
	}
	/* colormode abgr32 for gdk function */
	image_make(&view->buff,show->rows,show->cols);
	image_copy_color2bgr(&view->buff,show);
	show = &view->buff;
	/* draw! */
	view->dodraw = gdk_cairo_create(view->canvas->window);
	dotemp = gdk_pixbuf_new_from_data((const guchar*)show->data,
		GDK_COLORSPACE_RGB,TRUE,8,show->cols,show->rows,show->cols<<2,0x0,0x0);
	/*printf("Redraw: %d x %d (%p)\n",show->cols,show->rows,show);*/
	gdk_cairo_set_source_pixbuf(view->dodraw,dotemp,0,0);
	cairo_paint(view->dodraw);
	if (view->draw_more) view->draw_more((void*)view);
	cairo_destroy(view->dodraw);
	image_free(&mods);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_VIEWC__ */
/*----------------------------------------------------------------------------*/
