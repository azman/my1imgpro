/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
#include "my1image_util.h"
#include "my1image_math.h"
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
#define _PI_ 3.14159265
#define _FULL_PI_ (_PI_*2)
#define _HALF_PI_ (_PI_/2)
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
	int sizex, sizey;
	my1image_t currimage, *image;
	my1image_buffer_t work;
	my1image_filter_t ifilter_gray, ifilter_laplace1, ifilter_laplace2,
		ifilter_sobelx, ifilter_sobely, ifilter_sobel, ifilter_gauss;
	my1image_filter_t *pfilter;
	GtkWidget *dodraw;
}
my1image_test_t;
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
	printf("  --gray  : force grayscale format\n");
	printf("  --view  : show image (with user interface options)\n");
	printf("  --help  : show this message - overrides ALL above options\n");
	printf("Filters available:\n");
	printf("  laplace1, laplace2, sobelx, sobely, sobel, gauss\n");
}
/*----------------------------------------------------------------------------*/
gboolean on_draw_expose(GtkWidget *widget, GdkEventExpose *event,
	gpointer user_data)
{
	my1image_test_t* p = (my1image_test_t*) user_data;
	my1image_t* image = (my1image_t*) p->image;
	gdk_draw_rgb_32_image(widget->window,
		widget->style->fg_gc[GTK_STATE_NORMAL],
		0, 0, image->width, image->height,
		GDK_RGB_DITHER_NONE, (const guchar*)image->data, image->width*4);
	return TRUE;
}
/*----------------------------------------------------------------------------*/
void check_size(my1image_test_t* q)
{
	if (q->sizex!=q->image->width||q->sizey!=q->image->height)
	{
		gtk_widget_set_size_request(q->dodraw,
			q->image->width,q->image->height);
		q->sizex = q->image->width;
		q->sizey = q->image->height;
	}
	/* if in grayscale, convert to colormode grayscale? */
	if (q->image->mask!=IMASK_COLOR24) image_colormode(q->image);
	/* queue drawing */
	gtk_widget_queue_draw(q->dodraw);
}
/*----------------------------------------------------------------------------*/
gint on_key_press(GtkWidget *widget, GdkEventKey *kevent, gpointer data)
{
	int next = 0, loop, temp;
	my1image_test_t *q = (my1image_test_t*) data;

	if(kevent->type == GDK_KEY_PRESS)
	{
		/*
		 * g_message("%d, %c", kevent->keyval, kevent->keyval);
		 */
		if(kevent->keyval == GDK_KEY_Escape||
			kevent->keyval == GDK_KEY_q)
		{
			gtk_main_quit();
			return TRUE;
		}
		else if(kevent->keyval == GDK_KEY_o)
		{
			image_make(q->image,q->currimage.height,q->currimage.width);
			image_copy(q->image,&q->currimage);
			q->image->mask = q->currimage.mask;
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_g)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_i)
		{
			if (q->image->mask==IMASK_COLOR)
			{
				cbyte r, g, b;
				for(loop=0;loop<q->image->length;loop++)
				{
					decode_rgb(q->image->data[loop],&r,&g,&b);
					r = WHITE - r; g = WHITE - g; b = WHITE - b;
					q->image->data[loop] = encode_rgb(r,g,b);
				}
			}
			else image_invert(q->image);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_1)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_laplace1);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_2)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_laplace2);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_x)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobelx);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_y)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobely);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_s)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_sobel);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_a)
		{
			q->pfilter = filter_insert(0x0,&q->ifilter_gray);
			q->pfilter = filter_insert(q->pfilter,&q->ifilter_gauss);
			q->image = image_filter(q->image,q->pfilter);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_n)
		{
			image_grayscale(q->image);
			image_normalize(q->image);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_t)
		{
			image_grayscale(q->image);
			image_binary(q->image,WHITE/3);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_u)
		{
			image_turn(q->work.curr,q->work.next,IMAGE_TURN_090);
			buffer_swap(&q->work);
			q->image = q->work.curr;
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_v)
		{
			image_flip(q->work.curr,q->work.next,IMAGE_FLIP_VERTICAL);
			buffer_swap(&q->work);
			q->image = q->work.curr;
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_h)
		{
			image_flip(q->work.curr,q->work.next,IMAGE_FLIP_HORIZONTAL);
			buffer_swap(&q->work);
			q->image = q->work.curr;
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_b)
		{
			temp = BLACK;
			if (q->image->mask)
				temp = encode_rgb(temp,temp,temp);
			image_fill(q->image,temp);
			next = 1;
		}
		else if(kevent->keyval == GDK_KEY_w)
		{
			temp = WHITE;
			if (q->image->mask)
				temp = encode_rgb(temp,temp,temp);
			image_fill(q->image,temp);
			next = 1;
		}
		if (next)
		{
			check_size(q);
			return TRUE;
		}
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, error = 0;
	int gray = 0, view = 0, help = 0;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1image_test_t q;
	GtkWidget *window = 0x0, *dodraw = 0x0;
	GdkPixbuf *pixbuf;

	/* print tool info */
	printf("\n%s - %s (%s)\n",
		MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");

	/* initialize buffer & filters */
	buffer_init(&q.work);
	filter_init(&q.ifilter_gray, filter_gray, &q.work);
	filter_init(&q.ifilter_laplace1, filter_laplace_1, &q.work);
	filter_init(&q.ifilter_laplace2, filter_laplace_2, &q.work);
	filter_init(&q.ifilter_sobelx, filter_sobel_x, &q.work);
	filter_init(&q.ifilter_sobely, filter_sobel_y, &q.work);
	filter_init(&q.ifilter_sobel, filter_sobel, &q.work);
	filter_init(&q.ifilter_gauss, filter_gauss, &q.work);
	q.sizex = 0; q.sizey = 0; q.pfilter = 0x0;

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
				else if(!strcmp(argv[loop],"--gray"))
				{
					gray = 1;
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
	if(!pname)
	{
		printf("No filename given! Aborting!\n\n");
		return ERROR_NOFILE;
	}

	/* initialize image */
	image_init(&q.currimage);

	/* try to open file */
	if((error|=image_load(&q.currimage,pname))<0)
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
	buffer_size(&q.work,q.currimage.height,q.currimage.width);
	image_copy(q.work.curr,&q.currimage);
	q.image = q.work.curr;

	/* display basic info */
	printf("Input image: %s\n",pname);
	print_image_info(q.image);

	/* convert grayscale if requested/required */
	if(gray) image_grayscale(q.image);

	/* run image filter */
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
		printf("Saving image data to %s...\n",psave);
		if ((error|=image_save(q.image,psave))<0)
		{
			switch (error)
			{
				case BMP_ERROR_FILEOPEN:
				case PNM_ERROR_FILEOPEN:
					printf("Error writing file");
					break;
				default:
					printf("Unknown error opening file");
			}
			printf(":'%s'!\n",psave);
		}
		view = 0;
	}

	/** save c-formatted data if requested */
	if(pdata)
	{
		printf("Saving C data to %s...\n",pdata);
		switch (image_cdat(q.image,pdata))
		{
			case FILE_ERROR_OPEN:
				printf("Error writing c file:'%s'!\n",pdata);
				return error|FILE_ERROR_OPEN;
		}
	}

	/* we are done if no request for ui? */
	if (!view)
	{
		putchar('\n');
		return error;
	}

	/* show menu */
	printf("\n-----------------------------\n");
	printf("Image Processing Test Utility\n");
	printf("-----------------------------\n\n");
	printf("# <O>riginal Image\n");
	printf("# <G>rayscale Image\n");
	printf("# <I>nvert Image\n");
	printf("# Laplace<1> Image\n");
	printf("# Laplace<2> Image\n");
	printf("# Sobel<X> Image\n");
	printf("# Sobel<Y> Image\n");
	printf("# <S>obel Image\n");
	printf("# G<a>uss Image\n");
	printf("# <N>ormalize Image\n");
	printf("# <T>hreshold Image\n");
	printf("# T<u>rn Image 90CCW\n");
	printf("# Flip Image <V>ertical\n");
	printf("# Flip Image <H>orizontal\n");
	printf("# Fill Image All <B>lack\n");
	printf("# Fill Image All <W>hite\n");
	printf("# <Q>uit\n");

	/* initialize gui */
	gtk_init(&argc,&argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "MY1 Image Tool");
	gtk_window_set_default_size(GTK_WINDOW(window),
		q.image->width,q.image->height);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	pixbuf = gdk_pixbuf_new_from_file("icon.png",0x0);
	if (pixbuf) gtk_window_set_icon(GTK_WINDOW(window),pixbuf);
	/* deprecated in gtk3 in favor of cairo? */
	dodraw = gtk_drawing_area_new(); q.dodraw = dodraw;
	gtk_widget_set_size_request(dodraw,q.image->width,q.image->height);
	gtk_container_add(GTK_CONTAINER(window),dodraw);
	gtk_signal_connect(GTK_OBJECT(dodraw),"expose-event",
		GTK_SIGNAL_FUNC(on_draw_expose),(gpointer)&q);
	gtk_widget_show_all(window);
	g_signal_connect(G_OBJECT(window),"key_press_event",
		G_CALLBACK(on_key_press),(gpointer)&q);
	g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),0x0);
	/* initial draw? */
	check_size(&q);
	/* main loop */
	gtk_main();
	/* cleanup */
	if (pixbuf) g_object_unref(pixbuf);
	image_free(&q.currimage);
	buffer_free(&q.work);
	filter_free(&q.ifilter_laplace1);
	filter_free(&q.ifilter_laplace2);
	filter_free(&q.ifilter_sobelx);
	filter_free(&q.ifilter_sobely);
	filter_free(&q.ifilter_sobel);
	filter_free(&q.ifilter_gauss);
	putchar('\n');
	return error;
}
/*----------------------------------------------------------------------------*/
