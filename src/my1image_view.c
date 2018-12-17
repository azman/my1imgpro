/*----------------------------------------------------------------------------*/
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
void image_view_init(my1image_view_t* iview)
{
	iview->window = 0x0;
	iview->canvas = 0x0;
	iview->domenu = 0x0;
	iview->width = -1;
	iview->height = -1;
	iview->image = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_view_free(my1image_view_t* iview)
{
	/** nothing to free */
}
/*----------------------------------------------------------------------------*/
void image_view_make(my1image_view_t* iview)
{
	/* nothing to do if window/canvas already created */
	if (iview->window||iview->canvas) return;
	/* must have image */
	if (!iview->image) return;
	/* check default size */
	if (iview->width<0||iview->height<0)
	{
		iview->width = iview->image->width;
		iview->height = iview->image->height;
	}
	/* create window */
	iview->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(iview->window),"MY1Image Viewer");
	gtk_window_set_default_size(GTK_WINDOW(iview->window),
		iview->width,iview->height);
	gtk_window_set_position(GTK_WINDOW(iview->window),GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(iview->window),FALSE);
	/* create draw canvas - deprecated in gtk3 in favor of cairo? */
	iview->canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request(iview->canvas,iview->width,iview->height);
	gtk_container_add(GTK_CONTAINER(iview->window),iview->canvas);
	/* show me! */
	gtk_widget_show_all(iview->window);
}
/*----------------------------------------------------------------------------*/
void image_view_draw(my1image_view_t* iview)
{
	/* must have image - and canvas! */
	if (!iview->image||!iview->canvas) return;
	/* check the need to resize window */
	if (iview->width!=iview->image->width||iview->height!=iview->image->height)
	{
		gtk_widget_set_size_request(iview->canvas,
			iview->image->width,iview->image->height);
		iview->width = iview->image->width;
		iview->height = iview->image->height;
	}
	/* colormode abgr32 for gdk function */
	image_color2bgr(iview->image);
	/* draw! */
	gdk_draw_rgb_32_image(iview->canvas->window,
		iview->canvas->style->fg_gc[GTK_STATE_NORMAL],
		0, 0, iview->image->width, iview->image->height, GDK_RGB_DITHER_NONE,
		(const guchar*)iview->image->data, iview->image->width*4);
}
/*----------------------------------------------------------------------------*/
void image_view_name(my1image_view_t* iview,const char *name)
{
	/* must have window! */
	if (!iview->window) return;
	/* set title */
	gtk_window_set_title(GTK_WINDOW(iview->window),name);
}
/*----------------------------------------------------------------------------*/
