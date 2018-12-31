/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
#include "my1image_util.h"
#include "my1image_math.h"
#include "my1image_work.h"
#include "my1image_view.h"
/*----------------------------------------------------------------------------*/
#define _PI_ 3.14159265
#define _FULL_PI_ (_PI_*2)
#define _HALF_PI_ (_PI_/2)
/*----------------------------------------------------------------------------*/
#define MAX_WIDTH 640
#define MAX_HEIGHT 480
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1image_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "Basic Image Tool Library"
#endif
/*----------------------------------------------------------------------------*/
#define ERROR_GENERAL 0
#define ERROR_NOFILE -1
/*----------------------------------------------------------------------------*/
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
typedef struct _my1image_test_t
{
	my1image_t currimage, *image;
	my1image_view_t view;
	my1image_buffer_t work;
	my1image_filter_t ifilter_gray, ifilter_laplace1, ifilter_laplace2,
		ifilter_sobelx, ifilter_sobely, ifilter_sobel,
		ifilter_gauss, ifilter_maxscale, ifilter_suppress,
		ifilter_threshold, ifilter_canny;
	my1image_filter_t *pfilter;
}
my1image_test_t;
/*----------------------------------------------------------------------------*/
void image_test_init(my1image_test_t* test)
{
	image_init(&test->currimage);
	test->image = 0x0;
	image_view_init(&test->view);
	buffer_init(&test->work);
	filter_init(&test->ifilter_gray,filter_gray,&test->work);
	filter_init(&test->ifilter_laplace1,filter_laplace_1,&test->work);
	filter_init(&test->ifilter_laplace2,filter_laplace_2,&test->work);
	filter_init(&test->ifilter_sobelx,filter_sobel_x,&test->work);
	filter_init(&test->ifilter_sobely,filter_sobel_y,&test->work);
	filter_init(&test->ifilter_sobel,filter_sobel,&test->work);
	filter_init(&test->ifilter_gauss,filter_gauss,&test->work);
	filter_init(&test->ifilter_maxscale,filter_maxscale,&test->work);
	filter_init(&test->ifilter_suppress,filter_suppress,&test->work);
	filter_init(&test->ifilter_threshold,filter_threshold,&test->work);
	filter_init(&test->ifilter_canny,filter_canny,&test->work);
	test->ifilter_sobel.data = (void*) &test->work.xtra;
	test->ifilter_suppress.data = (void*) &test->work.xtra;
	test->ifilter_canny.data = (void*) &test->work.xtra;
	test->pfilter = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_test_free(my1image_test_t* test)
{
	image_free(&test->currimage);
	image_view_free(&test->view);
	buffer_free(&test->work);
	filter_free(&test->ifilter_gray);
	filter_free(&test->ifilter_laplace1);
	filter_free(&test->ifilter_laplace2);
	filter_free(&test->ifilter_sobelx);
	filter_free(&test->ifilter_sobely);
	filter_free(&test->ifilter_sobel);
	filter_free(&test->ifilter_gauss);
	filter_free(&test->ifilter_canny);
}
/*----------------------------------------------------------------------------*/
void print_image_info(my1image_t* image)
{
	printf("Size: %d x %d, ",image->width,image->height);
	printf("Mask: %08X\n",image->mask);
}
/*----------------------------------------------------------------------------*/
void about(void)
{
	printf("Command-line use:\n");
	printf("  %s [options] <image-file> [filter(s)]\n\n",MY1APP_PROGNAME);
	printf("Options are:\n");
	printf("  --save <filename>  : save to image file\n");
	printf("  --cdata <filename> : save to C source file\n");
	printf("  --view  : show image (with user interface options)\n");
	printf("  --help  : show this message - overrides ALL above options\n");
	printf("Filters available (grayscale implied):\n");
	printf("  laplace1, laplace2, sobelx, sobely, sobel, gauss, canny\n\n");
}
/*----------------------------------------------------------------------------*/
void set_menu_position(GtkMenu *menu, gint *x, gint *y,
	gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = (GtkWidget*)user_data;
	gdk_window_get_origin(window->window, x, y);
	*x += window->allocation.x + window->allocation.width/2;
	*y += window->allocation.y + window->allocation.height/2;
}
/*----------------------------------------------------------------------------*/
gboolean on_key_press(GtkWidget *widget, GdkEventKey *kevent, gpointer data)
{
	my1image_test_t *q = (my1image_test_t*) data;
	if(kevent->type == GDK_KEY_PRESS)
	{
		/** g_message("%d, %c", kevent->keyval, kevent->keyval); */
		if(kevent->keyval == GDK_KEY_Escape||kevent->keyval == GDK_KEY_q)
		{
			gtk_main_quit();
			return TRUE;
		}
		else if(kevent->keyval == GDK_KEY_space) /** GDK_KEY_Return */
		{
			gtk_menu_popup(GTK_MENU(q->view.domenu),0x0,0x0,
				&set_menu_position,(gpointer)q->view.window,0x0,0x0);
			return TRUE;
		}
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	const gint RIGHT_CLICK = 3;
	if (event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == RIGHT_CLICK)
		{
			GtkWidget* menu = (GtkWidget*) data;
			gtk_menu_popup(GTK_MENU(menu),0x0,0x0,0x0,0x0,
				event->button,event->time);
		}
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void on_image_original(my1image_test_t *q)
{
	image_make(q->image,q->currimage.height,q->currimage.width);
	image_copy(q->image,&q->currimage);
	q->image->mask = q->currimage.mask;
	image_view_draw(&q->view,q->image);
	image_view_stat_time(&q->view,"Original Image restored!",1);
}
/*----------------------------------------------------------------------------*/
void on_image_grayscale(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_invert(my1image_test_t *q)
{
	if (q->image->mask==IMASK_COLOR)
	{
		cbyte r, g, b;
		int loop;
		for(loop=0;loop<q->image->length;loop++)
		{
			decode_rgb(q->image->data[loop],&r,&g,&b);
			r = WHITE - r; g = WHITE - g; b = WHITE - b;
			q->image->data[loop] = encode_rgb(r,g,b);
		}
	}
	else image_invert(q->image);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_laplace1(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_laplace1);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_laplace2(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_laplace2);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobelx(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobelx);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobely(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobely);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobel(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobel);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_gaussian(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_gauss);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_maxscale(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_maxscale);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_suppress(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobel);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_suppress);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_threshold(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_threshold);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_canny(my1image_test_t *q)
{
	q->pfilter = filter_insert(0x0,&q->ifilter_gray);
	q->pfilter = filter_insert(q->pfilter,&q->ifilter_canny);
	q->image = image_filter(q->image,q->pfilter);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_normalize(my1image_test_t *q)
{
	image_grayscale(q->image);
	image_normalize(q->image);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_rotate_ccw90(my1image_test_t *q)
{
	image_turn(q->work.curr,q->work.next,IMAGE_TURN_090);
	buffer_swap(&q->work);
	q->image = q->work.curr;
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_flip_v(my1image_test_t *q)
{
	image_flip(q->work.curr,q->work.next,IMAGE_FLIP_VERTICAL);
	buffer_swap(&q->work);
	q->image = q->work.curr;
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_flip_h(my1image_test_t *q)
{
	image_flip(q->work.curr,q->work.next,IMAGE_FLIP_HORIZONTAL);
	buffer_swap(&q->work);
	q->image = q->work.curr;
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_toggle_histogram(my1image_test_t *q, GtkCheckMenuItem *menu_item)
{
	q->view.gohist = !q->view.gohist;
	gtk_check_menu_item_set_active(menu_item,q->view.gohist?TRUE:FALSE);
	image_view_show_hist(&q->view);
}
/*----------------------------------------------------------------------------*/
void image_test_events(my1image_test_t* test)
{
	g_signal_connect(G_OBJECT(test->view.window),"key_press_event",
		G_CALLBACK(on_key_press),(gpointer)test);
}
/*----------------------------------------------------------------------------*/
void on_file_open_main(my1image_test_t* test)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Image File",
		GTK_WINDOW(test->view.window),GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(doopen))==GTK_RESPONSE_ACCEPT)
	{
		int error;
		my1image_t that;
		gchar *filename, *buff = 0x0;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		image_init(&that);
		if((error=image_load(&that,filename))<0)
		{
			switch (error)
			{
				case BMP_ERROR_FILEOPEN:
				case PNM_ERROR_FILEOPEN:
					buff = g_strdup_printf("Error opening file! (%d){%s}",
						error,filename);
					break;
				case BMP_ERROR_VALIDBMP:
				case PNM_ERROR_VALIDPNM:
				case FILE_ERROR_FORMAT:
					buff = g_strdup_printf("Unsupported file format! (%d){%s}",
						error,filename);
					break;
				case BMP_ERROR_RGBNGRAY:
					buff = g_strdup_printf("Unsupported BMP format! (%d){%s}",
						error,filename);
					break;
				case PNM_ERROR_NOSUPPORT:
					buff = g_strdup_printf("Unsupported PNM format! (%d){%s}",
						error,filename);
					break;
				case BMP_ERROR_FILESIZE:
				case PNM_ERROR_FILESIZE:
					buff = g_strdup_printf("Invalid file size! (%d){%s}",
						error,filename);
					break;
				case BMP_ERROR_MEMALLOC:
				case PNM_ERROR_MEMALLOC:
					buff = g_strdup_printf("Cannot allocate memory! (%d){%s}",
						error,filename);
					break;
				case BMP_ERROR_DIBINVAL:
				case BMP_ERROR_COMPRESS:
					buff = g_strdup_printf("Invalid BMP format! (%d){%s}",
						error,filename);
					break;
				default:
					buff = g_strdup_printf("Unknown error! (%d){%s}",
						error,filename);
			}
			/* show info on status bar */
			buff = g_strdup_printf("[ERROR] %s",buff);
			image_view_stat_time(&test->view,(char*)buff,5);
		}
		else
		{
			/* successful image file open */
			buffer_size(&test->work,that.height,that.width);
			image_copy(test->work.curr,&that);
			test->image = image_size_this(test->work.curr,test->work.next,
				MAX_HEIGHT,MAX_WIDTH);
			if (test->image!=test->work.curr)
				buffer_swap(&test->work);
			image_copy(&test->currimage,test->image); /* keep original */
			image_view_draw(&test->view,test->image);
			/* show info on status bar */
			buff = g_strdup_printf("[CHECK] %s",filename);
			image_view_stat_time(&test->view,(char*)buff,5);
		}
		image_free(&that);
		if (buff) g_free(buff);
		g_free (filename);
	}
	gtk_widget_destroy(doopen);
 }
/*----------------------------------------------------------------------------*/
void on_file_save_main(my1image_test_t* test)
{
	GtkWidget *dosave = gtk_file_chooser_dialog_new("Save Image File",
		GTK_WINDOW(test->view.window),GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dosave),
		TRUE);
	if (gtk_dialog_run(GTK_DIALOG(dosave))==GTK_RESPONSE_ACCEPT)
	{
		int error;
		gchar *filename, *buff = 0x0;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dosave));
		if ((error=image_save(test->image,filename))<0)
		{
			switch (error)
			{
				case BMP_ERROR_FILEOPEN:
				case PNM_ERROR_FILEOPEN:
					buff = g_strdup_printf("Write error! (%d){%s}",
						error,filename);
					break;
				default:
					buff = g_strdup_printf("Unknown error! (%d){%s}",
						error,filename);
			}
			/* show info on status bar */
			buff = g_strdup_printf("[ERROR] %s",buff);
			image_view_stat_time(&test->view,(char*)buff,5);
		}
		else
		{
			/* show info on status bar */
			buff = g_strdup_printf("[SAVED] %s",filename);
			image_view_stat_time(&test->view,(char*)buff,5);
		}
		if (buff) g_free(buff);
		g_free (filename);
	}
	gtk_widget_destroy(dosave);
 }
/*----------------------------------------------------------------------------*/
void image_test_menu(my1image_test_t* test)
{
	GtkWidget *menu_main, *menu_item, *menu_subs, *menu_temp;
	/* create popup menu for canvas */
	menu_main = gtk_menu_new();
	gtk_widget_add_events(test->view.canvas, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(GTK_OBJECT(test->view.canvas),"button-press-event",
		G_CALLBACK(on_mouse_click),(gpointer)menu_main);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* original menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Original");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_original),(gpointer)test);
	gtk_widget_show(menu_item);
	/* grayscale menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Grayscale");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_grayscale),(gpointer)test);
	gtk_widget_show(menu_item);
	/* invert menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Invert");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_invert),(gpointer)test);
	gtk_widget_show(menu_item);
	/* normalize menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Normalize");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_normalize),(gpointer)test);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Image");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* laplace1 menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Laplace _1");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_laplace1),(gpointer)test);
	gtk_widget_show(menu_item);
	/* laplace2 menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Laplace _2");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_laplace2),(gpointer)test);
	gtk_widget_show(menu_item);
	/* sobelx menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Sobel _X");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_sobelx),(gpointer)test);
	gtk_widget_show(menu_item);
	/* sobely menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Sobel _Y");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_sobely),(gpointer)test);
	gtk_widget_show(menu_item);
	/* sobel menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Sobel");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_sobel),(gpointer)test);
	gtk_widget_show(menu_item);
	/* gauss menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Gaussian");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_gaussian),(gpointer)test);
	gtk_widget_show(menu_item);
	/* maxscale menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Max-Rescale");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_maxscale),(gpointer)test);
	gtk_widget_show(menu_item);
	/* suppress menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Suppress");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_suppress),(gpointer)test);
	gtk_widget_show(menu_item);
	/* threshold menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Threshold");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_threshold),(gpointer)test);
	gtk_widget_show(menu_item);
	/* canny menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Canny");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_canny),(gpointer)test);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Filters");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* rotate menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Rotate CCW90");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_rotate_ccw90),(gpointer)test);
	gtk_widget_show(menu_item);
	/* flip v menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Vertical");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_flip_v),(gpointer)test);
	gtk_widget_show(menu_item);
	/* flip h menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Horizontal");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_flip_h),(gpointer)test);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Orientation");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* file load menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Open Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_file_open_main),(gpointer)test);
	gtk_widget_show(menu_item);
	/* file save as menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Save Image _As...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_file_save_main),(gpointer)test);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* test menu item */
	menu_item = gtk_check_menu_item_new_with_label("Histogram");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_toggle_histogram),(gpointer)test);
	gtk_widget_show(menu_item);
	/* quit menu item */
	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",
		G_CALLBACK(gtk_main_quit),0x0);
	gtk_widget_show(menu_item);
	/* save that menu */
	test->view.domenu = menu_main;
	/* show it! */
	gtk_widget_show(test->view.domenu);
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, error = 0, view = 0, help = 0;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1image_test_t q;
	/* print tool info */
	printf("\n%s - %s (%s)\n",
		MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* initialize image_test */
	image_test_init(&q);
	/* check program arguments */
	if(argc>1)
	{
		for(loop=1;loop<argc;loop++)
		{
			if(argv[loop][0]=='-') /* options! */
			{
				if(!strcmp(argv[loop],"--save"))
				{
					loop++;
					if(loop<argc)
						psave = argv[loop];
					else
						printf("Cannot get save file name - NOT saving!\n");
				}
				else if(!strcmp(argv[loop],"--cdata"))
				{
					loop++;
					if(loop<argc)
						pdata = argv[loop];
					else
						printf("Cannot get C file name - NOT writing!\n");
				}
				else if(!strcmp(argv[loop],"--view"))
				{
					view = 1;
				}
				else if(!strcmp(argv[loop],"--help"))
				{
					help = 1;
				}
				else
				{
					printf("Unknown option '%s'!\n",argv[loop]);
				}
			}
			else /* not an option? */
			{
				/* first non-option must be file name! */
				if(!pname)
				{
					pname = argv[loop];
					continue;
				}
				/* then check for command! */
				if(!strcmp(argv[loop],"laplace1"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_laplace1);
				}
				else if(!strcmp(argv[loop],"sobelx"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_sobelx);
				}
				else if(!strcmp(argv[loop],"sobely"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_sobely);
				}
				else if(!strcmp(argv[loop],"sobel"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_sobel);
				}
				else if(!strcmp(argv[loop],"laplace2"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_laplace2);
				}
				else if(!strcmp(argv[loop],"gauss"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_gauss);
				}
				else if(!strcmp(argv[loop],"canny"))
				{
					if (!q.pfilter)
						q.pfilter = filter_insert(q.pfilter,&q.ifilter_gray);
					q.pfilter = filter_insert(q.pfilter,&q.ifilter_canny);
				}
				else
				{
					printf("Unknown parameter %s!\n",argv[loop]);
					continue;
				}
			}
		}
	}
	/* check if user requested help */
	if(help)
	{
		about();
		return 0;
	}
	/** check input filename */
	if(pname)
	{
		/* try to open file */
		if((error=image_load(&q.currimage,pname))<0)
		{
			switch (error)
			{
				case BMP_ERROR_FILEOPEN:
				case PNM_ERROR_FILEOPEN:
					printf("Error opening file");
					break;
				case BMP_ERROR_VALIDBMP:
				case PNM_ERROR_VALIDPNM:
				case FILE_ERROR_FORMAT:
					printf("Unsupported file format (%d)",error);
					break;
				case BMP_ERROR_RGBNGRAY:
					printf("Unsupported BMP format");
					break;
				case PNM_ERROR_NOSUPPORT:
					printf("Unsupported PNM format");
					break;
				case BMP_ERROR_FILESIZE:
				case PNM_ERROR_FILESIZE:
					printf("Invalid file size");
					break;
				case BMP_ERROR_MEMALLOC:
				case PNM_ERROR_MEMALLOC:
					printf("Unable to allocate memory");
					break;
				case BMP_ERROR_DIBINVAL:
				case BMP_ERROR_COMPRESS:
					printf("Invalid BMP format");
					break;
				default:
					printf("Unknown error opening file (%d)",error);
			}
			printf(": '%s'!\n\n",pname);
			return error;
		}
	}
	else
	{
		if (!view)
		{
			printf("No filename given! Aborting!\n\n");
			return ERROR_NOFILE;
		}
		/* create blank image at default size */
		image_make(&q.currimage,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&q.currimage,BLACK);
	}
	/* prepare buffer */
	buffer_size(&q.work,q.currimage.height,q.currimage.width);
	image_copy(q.work.curr,&q.currimage);
	q.image = q.work.curr;
	/* only if a file has been loaded */
	if (pname)
	{
		/* display basic info */
		printf("Input image: %s\n",pname);
		print_image_info(q.image);
		/* run image filter, if requested */
		if (q.pfilter)
		{
			q.image = image_filter(q.image,q.pfilter);
			image_absolute(q.image);
			image_cliphi(q.image,WHITE);
		}
		/* display processed info */
		printf("Check image:\n");
		print_image_info(q.image);
		/** save results if requested */
		if(psave)
		{
			printf("Saving image data to %s... ",psave);
			if ((error=image_save(q.image,psave))<0)
			{
				switch (error)
				{
					case BMP_ERROR_FILEOPEN:
					case PNM_ERROR_FILEOPEN:
						printf("write error!\n");
						break;
					default:
						printf("unknown error!\n");
				}
			}
			else printf("done!\n");
		}
		/** save c-formatted data if requested */
		if(pdata)
		{
			printf("Saving C data to %s... ",pdata);
			if ((error=image_cdat(q.image,pdata))<0)
			{
				switch (error)
				{
					case FILE_ERROR_OPEN:
						printf("write error!\n");
						break;
					default:
						printf("unknown error!\n");
				}
			}
			else printf("done!\n");
		}
	}
	/* check request for ui */
	if (view)
	{
		gchar *buff;
		/* check size for gui */
		image_copy(q.work.curr,&q.currimage);
		image_size_this(q.work.curr,&q.currimage,MAX_HEIGHT,MAX_WIDTH);
		buffer_size_all(&q.work,q.currimage.height,q.currimage.width);
		image_copy(q.work.curr,&q.currimage);
		q.image = q.work.curr;
		/* initialize gui */
		gtk_init(&argc,&argv);
		/* make image_view */
		image_view_make(&q.view,q.image);
		image_view_draw(&q.view,q.image);
		/* allow histogram */
		image_view_make_hist(&q.view);
		/* show info on status bar */
		buff = g_strdup_printf("Size:%dx%d Mask:0x%08x",
			q.image->width,q.image->height,q.image->mask);
		image_view_stat_time(&q.view,(char*)buff,5);
		g_free(buff);
		/* event handlers */
		image_test_events(&q);
		/* menu stuff */
		image_test_menu(&q);
		/* main loop */
		gtk_main();
	}
	/* cleanup image_test */
	image_test_free(&q);
	/* done! */
	putchar('\n');
	return error;
}
/*----------------------------------------------------------------------------*/
