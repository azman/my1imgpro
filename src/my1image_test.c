/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
#include "my1image_work.h"
#include "my1image_view.h"
#include "my1image_hist.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#define ERROR_MAX 5
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
#define MY1APP_PROGINFO "MY1Image Test Program"
#endif
/*----------------------------------------------------------------------------*/
#define ERROR_GENERAL 0
#define ERROR_NOFILE -1
/*----------------------------------------------------------------------------*/
typedef struct _my1image_test_t
{
	my1image_t currimage, *image;
	my1image_view_t view;
	my1image_hist_t hist;
	my1image_buffer_t work;
	my1image_filter_t *pflist;
	my1image_filter_t *pfcurr;
}
my1image_test_t;
/*----------------------------------------------------------------------------*/
void image_test_histogram(void* data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	my1image_hist_t* hist = (my1image_hist_t*) view->draw_more_data;
	image_hist_show(hist);
}
/*----------------------------------------------------------------------------*/
void image_test_init(my1image_test_t* test)
{
	image_init(&test->currimage);
	test->image = 0x0;
	image_view_init(&test->view);
	image_hist_init(&test->hist,&test->view);
	test->view.draw_more = (void*) &image_test_histogram;
	test->view.draw_more_data = (void*) &test->hist;
	buffer_init(&test->work);
	test->pflist = image_work_create_all();
	test->pfcurr = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_test_free(my1image_test_t* test)
{
	image_free(&test->currimage);
	image_view_free(&test->view);
	image_hist_free(&test->hist);
	buffer_free(&test->work);
	if (test->pflist)
		filter_clean(test->pflist);
	if (test->pfcurr)
		filter_clean(test->pfcurr);
}
/*----------------------------------------------------------------------------*/
void image_test_load(my1image_test_t* test, char* name)
{
	my1image_filter_t *ipass, *tpass = 0x0;
	ipass = filter_search(test->pflist,name);
	if (ipass) tpass = filter_clone(ipass);
	if (tpass)
	{
		tpass->buffer = &test->work;
		test->pfcurr = filter_insert(test->pfcurr,tpass);
	}
}
/*----------------------------------------------------------------------------*/
void image_test_filter(my1image_test_t* test)
{
	if (test->pfcurr)
	{
#if 0
		/* debug! */
		int size = 0;
		my1image_filter_t* ppass = test->pfcurr;
		printf("Filtering:\n");
		while (ppass)
		{
			printf("- {%s}\n",ppass->name);
			ppass = ppass->next;
			size++;
		}
		printf("Done:%d.\n",size);
#endif
		test->image = image_filter(test->image,test->pfcurr);
		filter_clean(test->pfcurr);
		test->pfcurr = 0x0;
	}
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
	printf("  laplace, sobelx, sobely, sobel, gauss, canny\n\n");
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
		if(kevent->keyval == GDK_KEY_Escape||
			kevent->keyval == GDK_KEY_q)
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
		else if(kevent->keyval == GDK_KEY_F||kevent->keyval == GDK_KEY_f)
		{
			q->view.gofull = !q->view.gofull;
			image_view_full(&q->view,q->view.gofull);
			return TRUE;
		}
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
gboolean on_mouse_click(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	const gint RIGHT_CLICK = 3, LEFT_CLICK = 1;
	if (event->type == GDK_BUTTON_PRESS)
	{
		my1image_test_t *q = (my1image_test_t*) data;
		if (event->button == RIGHT_CLICK)
		{
			GtkWidget* menu = (GtkWidget*) q->view.domenu;
			gtk_menu_popup(GTK_MENU(menu),0x0,0x0,0x0,0x0,
				event->button,event->time);
		}
		else if (event->button == LEFT_CLICK)
		{
			int temp = event->x + event->y * q->view.buff.width;
			int mask = q->view.buff.mask;
			int data = q->view.buff.data[temp] & mask;
			gchar *buff;
			my1rgb_t *chk;
			my1hsv_t that;
			data = color_swap(data);
			chk = (my1rgb_t*)&data;
			that = rgb2hsv(*chk);
			buff = g_strdup_printf("[PIXEL] %08X{%08X}<%d>@(%d,%d)",
				data,mask,that.h,(int)event->x,(int)event->y);
			image_view_stat_time(&q->view,(char*)buff,3);
			g_free(buff);
		}
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void on_image_original(my1image_test_t *q)
{
	image_copy(q->image,&q->currimage);
	image_view_make(&q->view,q->image);
	image_view_draw(&q->view,q->image); /* force redraw - in case no resize! */
	image_view_stat_time(&q->view,"Original Image restored!",1);
}
/*----------------------------------------------------------------------------*/
void on_image_grayscale(my1image_test_t *q)
{
	image_grayscale(q->image);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_blue(my1image_test_t *test)
{
	image_test_load(test,IFNAME_COLORBLUE);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_green(my1image_test_t *test)
{
	image_test_load(test,IFNAME_COLORGREEN);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_red(my1image_test_t *test)
{
	image_test_load(test,IFNAME_COLORRED);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
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
void on_image_normalize(my1image_test_t *q)
{
	image_grayscale(q->image);
	image_normalize(q->image);
	image_view_draw(&q->view,q->image);
}
/*----------------------------------------------------------------------------*/
void on_image_laplace(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_LAPLACE);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobelx(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_SOBELX);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobely(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_SOBELY);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_sobel(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_SOBEL);
	image_test_load(test,"dummy");
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_gaussian(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_GAUSS);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_maxscale(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_MAXSCALE);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_suppress(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_SOBEL);
	image_test_load(test,IFNAME_SUPPRESS);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
}
/*----------------------------------------------------------------------------*/
void on_image_threshold(my1image_test_t *test)
{
	image_test_load(test,IFNAME_GRAYSCALE);
	image_test_load(test,IFNAME_THRESHOLD);
	image_test_filter(test);
	image_view_draw(&test->view,test->image);
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
	q->hist.dohide = !q->hist.dohide;
	gtk_check_menu_item_set_active(menu_item,q->hist.dohide?FALSE:TRUE);
	image_hist_show(&q->hist);
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
			image_copy(test->work.curr,&that);
			test->image = test->work.curr;
			/* limit size if it gets too big? */
			if (test->image->width>MAX_WIDTH||test->image->height>MAX_HEIGHT)
			{
				test->image = image_size_this(test->work.curr,test->work.next,
					MAX_HEIGHT,MAX_WIDTH);
				if (test->image!=test->work.curr)
					buffer_swap(&test->work);
			}
			image_copy(&test->currimage,test->image); /* keep original */
			image_view_make(&test->view,test->image);
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
		if ((error=image_save(&test->view.buff,filename))<0)
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
		G_CALLBACK(on_mouse_click),(gpointer)test);
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
	/* color filter menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Color:_Blue");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_blue),(gpointer)test);
	gtk_widget_show(menu_item);
	/* color filter menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Color:Gr_een");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_green),(gpointer)test);
	gtk_widget_show(menu_item);
	/* color filter menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Color:Re_d");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_red),(gpointer)test);
	gtk_widget_show(menu_item);
	/* laplace1 menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Laplace");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(on_image_laplace),(gpointer)test);
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
	menu_item = gtk_menu_item_new_with_mnemonic("_MaxScale");
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
typedef struct _hsv_stat_t
{
	int col, max, min, del, hue, reg, off, sub, chk;
}
hsv_stat_t;
/*----------------------------------------------------------------------------*/
void hsv_get_stat(my1hsv_t hsv, hsv_stat_t* hsv_stat)
{
	int max, min, del, hue, reg, off, sub, chk;
	/* check zero saturation => pure grayscale! */
	if (!hsv.s)
	{
		hsv_stat->col = 0;
		hsv_stat->max = hsv.v;
		return;
	}
	/* max & min for rgb are 100% reproducible! */
	max = hsv.v;
	del = hsv.s * max / WHITE;
	min = max - del;
	/* third value */
	hue = hsv.h;
	reg = (hue / HUE_DIFF); /* should be <HUE_PART */
	off = (hue - ((reg>>1) * HUE_COMP)); /* offset from HUE_COMP */
	if (off>HUE_DIFF) off = HUE_COMP-off;
	sub = (off * del / HUE_DIFF); /* get diff for third value */
	chk = (sub + min); /* third value */
	/* assign to struct */
	hsv_stat->col = 1;
	hsv_stat->max = max;
	hsv_stat->min = min;
	hsv_stat->del = del;
	hsv_stat->hue = hue;
	hsv_stat->reg = reg;
	hsv_stat->off = off;
	hsv_stat->sub = sub;
	hsv_stat->chk = chk;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, domax = ERROR_MAX, error = 0, dohsv = 0;
	int view = 0, help = 0, full = 0;
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
				else if(!strcmp(argv[loop],"--domax"))
				{
					loop++;
					if(loop<argc)
						domax = atoi(argv[loop]);
					else
						printf("Cannot get max error! Using {%d}\n",domax);
				}
				else if(!strcmp(argv[loop],"--dohsv"))
				{
					dohsv = 1;
				}
				else if(!strcmp(argv[loop],"--view"))
				{
					view = 1;
				}
				else if(!strcmp(argv[loop],"--full"))
				{
					full = 1;
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
				if(!strcmp(argv[loop],"laplace"))
				{
					if (!q.pfcurr)
						image_test_load(&q,IFNAME_GRAYSCALE);
					image_test_load(&q,IFNAME_LAPLACE);
				}
				else if(!strcmp(argv[loop],"sobelx"))
				{
					if (!q.pfcurr)
						image_test_load(&q,IFNAME_GRAYSCALE);
					image_test_load(&q,IFNAME_SOBELX);
				}
				else if(!strcmp(argv[loop],"sobely"))
				{
					if (!q.pfcurr)
						image_test_load(&q,IFNAME_GRAYSCALE);
					image_test_load(&q,IFNAME_SOBELY);
				}
				else if(!strcmp(argv[loop],"sobel"))
				{
					if (!q.pfcurr)
						image_test_load(&q,IFNAME_GRAYSCALE);
					image_test_load(&q,IFNAME_SOBEL);
				}
				else if(!strcmp(argv[loop],"gauss"))
				{
					if (!q.pfcurr)
						image_test_load(&q,IFNAME_GRAYSCALE);
					image_test_load(&q,IFNAME_GAUSS);
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
		if (dohsv)
		{
			/* run some test here? */
			my1rgb_t test, buff; my1hsv_t temp;
			int check = 0, ckmax = -1, ckmin = -1;
			test.g = BLACK; test.a = 0; error = 0;
			printf("Testing RGB <-> HSV Color Conversion [BEGIN]");
			printf(" => {MaxError:%d}\n",domax);
			for (;;)
			{
				test.r = BLACK;
				for (;;)
				{
					test.b = BLACK;
					for (;;)
					{
						temp = rgb2hsv(test);
						buff = hsv2rgb(temp);
						if (abs(test.r-buff.r)>domax||
							abs(test.g-buff.g)>domax||
							abs(test.b-buff.b)>domax)
						{
							hsv_stat_t stat;
							printf("[CHECK](%d,%d,%d)->{%d,%d,%d}->"
								"(%d,%d,%d)=>",test.r,test.g,test.b,
								temp.h,temp.s,temp.v,buff.r,buff.g,buff.b);
							hsv_get_stat(temp,&stat);
							printf("{max:%d,del:%d,min:%d,hue:%d,"
								"reg:%d,off:%d,sub:%d,chk:%d}\n",
								stat.max,stat.del,stat.min,stat.hue,
								stat.reg,stat.off,stat.sub,stat.chk);
							check += stat.del;
							if (ckmax<0||stat.del>ckmax) ckmax = stat.del;
							if (ckmin<0||stat.del<ckmin) ckmin = stat.del;
							error++;
						}
						if (++test.b==0) break;
					}
					if (++test.r==0) break;
				}
				if (++test.g==0) break;
			}
			printf("Testing RGB <-> HSV Color Conversion [DONE]");
			printf(" => {DoMax:%d,Error:%d,DifAv:%d,DifMx:%d,DifMn:%d}\n\n",
				domax,error,error>0?(check/error):0,ckmax,ckmin);
			return error;
		}
		if (!view)
		{
			printf("No filename given for image testing! Aborting!\n\n");
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
		if (q.pfcurr)
		{
			image_test_filter(&q);
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
		q.image = q.work.curr;
		/* limit size if it gets too big? */
		if (q.image->width>MAX_WIDTH||q.image->height>MAX_HEIGHT)
		{
			q.image = image_size_this(q.image,q.work.next,MAX_HEIGHT,MAX_WIDTH);
			if (q.image!=q.work.curr)
				buffer_swap(&q.work);
			/* keep original copy */
			image_copy(&q.currimage,q.image);
		}
		/* initialize gui */
		gtk_init(&argc,&argv);
		/* setup auto-quit on close */
		q.view.goquit = 1;
		if (full) q.view.gofull = 1;
		/* make image_view */
		image_view_make(&q.view,q.image);
		image_view_name(&q.view,MY1APP_PROGINFO);
		/* allow histogram */
		image_hist_make(&q.hist);
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
