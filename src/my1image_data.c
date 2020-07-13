/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_DATAC__
#define __MY1IMAGE_DATAC__
/*----------------------------------------------------------------------------*/
#include "my1image_data.h"
#include "my1image_main.h"
#include "my1image_ifdb.h"
#include "my1image_chsv.h"
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
static const char BLANK_IMAGE[] = BLANK_IMAGE_OPT;
/*----------------------------------------------------------------------------*/
int image_data_histogram(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1image_hist_t* hist = (my1image_hist_t*) task->data;
	image_hist_show(hist);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_init(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	what->dosize = 0;
	what->maxh = DEFAULT_MAX_HEIGHT;
	what->maxw = DEFAULT_MAX_WIDTH;
	image_hist_init(&what->hist,&mdat->iwin);
	image_init(&what->buff);
	what->list = 0x0;
	itask_make(&mdat->iwin.view.domore,
		image_data_histogram,(void*)&what->hist);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_free(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	image_free(&what->buff);
	image_hist_free(&what->hist);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_args(void* data, void* that, void* xtra)
{
	int loop, argc, *temp = (int*) that;
	char** argv = (char**) xtra;
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)task->temp;
	argc = *temp;
	if (argc<2)
	{
		/* override 2end error and load blank! */
		mdat->flag = IFLAG_OK;
		mdat->grab.pick = (char*) BLANK_IMAGE;
		return 0;
	}
	/* check args */
	for (loop=2;loop<argc;loop++)
	{
		if (!strncmp(argv[loop],"--xmax",6))
		{
			what->maxw = atoi(argv[++loop]);
			what->dosize = 1;
		}
		else if (!strncmp(argv[loop],"--ymax",6))
		{
			what->maxh = atoi(argv[++loop]);
			what->dosize = 1;
		}
		else if (!strncmp(argv[loop],"--filter",9))
		{
			what->list = argv[++loop];
		}
		else
			printf("-- Unknown argument '%s'!\n",argv[loop]);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_prep(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	/** load all predefined filters */
	if (!mdat->list)
		mdat->list = image_work_create_all();
	/** check requested filters - do not auto exec? */
	if (what->list)
		imain_filter_doload_list(mdat,what->list);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_exec(void* data, void* that, void* xtra)
{
	int rows, cols;
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	if (what->dosize)
	{
		rows = mdat->show->rows;
		cols = mdat->show->cols;
		if (rows>what->maxh) rows = what->maxh;
		if (cols>what->maxw) cols = what->maxw;
		image_size_this(mdat->show,&what->buff,rows,cols);
	}
	else image_copy(&what->buff,mdat->show);
	mdat->show = &what->buff;
	return 0;
}
/*----------------------------------------------------------------------------*/
void data_on_toggle_histogram(my1image_data_t *data,
	GtkCheckMenuItem *menu_item)
{
	data->hist.dohide = !data->hist.dohide;
	gtk_check_menu_item_set_active(menu_item,data->hist.dohide?FALSE:TRUE);
	image_hist_show(&data->hist);
}
/*----------------------------------------------------------------------------*/
void image_data_domenu_extra(my1image_data_t* data, GtkWidget* menu_main)
{
	GtkWidget *menu_item;
	menu_item = gtk_check_menu_item_new_with_label("Histogram");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item)
		,data->hist.dohide?FALSE:TRUE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_toggle_histogram),(gpointer)data);
	gtk_widget_show(menu_item);
}
/*----------------------------------------------------------------------------*/
int data_on_clickM(void* args, void* that, void* xtra)
{
	gchar *buff;
	my1image_appw_t *appw = (my1image_appw_t*) that;
	my1image_t *last = appw->show; /* last image sent to viewer */
	my1image_t *view = &appw->view.buff;
	/* show info on status bar */
	buff = g_strdup_printf("Size:%dx%d Mask:0x%08x",
		view->cols,view->rows,last->mask);
	image_appw_stat_time(appw,(char*)buff,5);
	g_free(buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
int data_on_clickL(void* args, void* that, void* xtra)
{
	int mask, dpix;
	gchar *buff;
	my1rgb_t *temp;
	my1hsv_t chsv;
	my1image_appw_t *appw = (my1image_appw_t*) that;
	my1image_t *last = appw->show; /* last image sent to viewer */
	my1image_t *view = &appw->view.buff;
	GdkEventButton *event = (GdkEventButton*) xtra;
	mask = last->mask;
	dpix = image_get_pixel(view,event->y,event->x);
	dpix &= IMASK_COLOR; /* remove alpha */
	dpix = color_swap(dpix); /* get rgb from bgr */
	temp = (my1rgb_t*)&dpix;
	chsv = rgb2hsv(*temp);
	buff = g_strdup_printf("[PIXEL] %08X{%08X}<%d>@(%d,%d)",
		dpix,mask,chsv.h,(int)event->x,(int)event->y);
	image_appw_stat_time(appw,(char*)buff,3);
	g_free(buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_data_show(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1image_data_t *what = (my1image_data_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	itask_make(&mdat->iwin.clickL,data_on_clickL,(void*)what);
	itask_make(&mdat->iwin.clickM,data_on_clickM,(void*)what);
	image_appw_domenu(&mdat->iwin);
	imain_domenu_filters(mdat);
	image_data_domenu_extra(what,mdat->iwin.domenu);
	image_appw_domenu_quit(&mdat->iwin);
	image_appw_stat_hide(&mdat->iwin,0);
	image_hist_make(&what->hist);
	return 0;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_DATAC__ */
/*----------------------------------------------------------------------------*/
