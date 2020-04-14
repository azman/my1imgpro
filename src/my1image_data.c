/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_DATAC__
#define __MY1IMAGE_DATAC__
/*----------------------------------------------------------------------------*/
#include "my1image_data.h"
#include "my1image_file.h"
#include "my1image_chsv.h"
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
int image_data_histogram(void* data, void* that, void* xtra)
{
	my1dotask_t *task = (my1dotask_t*)data;
	//my1image_view_t* view = (my1image_view_t*) that;
	my1image_hist_t* hist = (my1image_hist_t*) task->data;
	image_hist_show(hist);
	return 0;
}
/*----------------------------------------------------------------------------*/
void image_data_init(my1image_data_t* data)
{
	data->dosize = 0;
	data->maxh = DEFAULT_MAX_HEIGHT;
	data->maxw = DEFAULT_MAX_WIDTH;
	data->flag = DATA_FLAG_OK;
	image_appw_init(&data->appw);
	data->appw.goquit = 1;
	dotask_make(&data->appw.view.domore,
		image_data_histogram,(void*)&data->hist);
	image_hist_init(&data->hist,&data->appw);
	buffer_init(&data->work);
	data->pflist = 0x0;
	data->pfcurr = 0x0;
}
/*----------------------------------------------------------------------------*/
void image_data_free(my1image_data_t* data)
{
	if (data->pflist) filter_free_clones(data->pflist);
	if (data->pfcurr) filter_free_clones(data->pfcurr);
	buffer_free(&data->work);
	image_hist_free(&data->hist);
	image_appw_free(&data->appw);
}
/*----------------------------------------------------------------------------*/
void image_data_draw(my1image_data_t* data)
{
	image_appw_draw(&data->appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void image_data_make(my1image_data_t* data, my1image_t* that)
{
	my1image_t temp;
	int cols, rows;
	image_appw_make(&data->appw,that);
	that = data->appw.show;
	if (data->dosize)
	{
		if (that->cols>data->maxw||that->rows>data->maxh)
		{
			image_init(&temp);
			cols = that->cols>data->maxw ? data->maxw : that->cols;
			rows = that->rows>data->maxh ? data->maxh : that->rows;
			image_size_this(that,&temp,rows,cols);
			image_copy(that,&temp);
			image_free(&temp);
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_data_work(my1image_data_t* data)
{
	if (!data->pflist)
		data->pflist = image_work_create_all();
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
void data_on_toggle_histogram(my1image_data_t *data,
	GtkCheckMenuItem *menu_item)
{
	data->hist.dohide = !data->hist.dohide;
	gtk_check_menu_item_set_active(menu_item,data->hist.dohide?FALSE:TRUE);
	image_hist_show(&data->hist);
}
/*----------------------------------------------------------------------------*/
void data_on_load_filter(my1image_data_t* data)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Filter List",
		GTK_WINDOW(data->appw.window),GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(doopen))==GTK_RESPONSE_ACCEPT)
	{
		char fname[FILTER_NAMESIZE], *pname;
		FILE* pfile;
		gchar *filename, *buff;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		pfile = fopen((char*)filename,"rt");
		if (pfile)
		{
			while (fgets(fname,FILTER_NAMESIZE,pfile))
			{
				pname = fname;
				while (*pname)
				{
					if (pname[0]=='\n'||pname[0]==' '||
							pname[0]=='\t'||pname[0]=='\r')
					{
						pname[0] = 0x0;
					}
					pname++;
				}
				image_data_filter_load(data,fname);
			}
			fclose(pfile);
			buff = g_strdup_printf("[CHECK] '%s' loaded!",filename);
		}
		else
			buff = g_strdup_printf("[ERROR] Cannot load '%s'!",filename);
		/* show info on status bar */
		image_appw_stat_time(&data->appw,(char*)buff,5);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(doopen);
}
/*----------------------------------------------------------------------------*/
void data_on_filter_select(my1image_data_t *data, GtkMenuItem *menu_item)
{
	image_data_filter_load(data,(char*)gtk_menu_item_get_label(menu_item));
}
/*----------------------------------------------------------------------------*/
void data_on_filter_execute(my1image_data_t *data, GtkMenuItem *menu_item)
{
	image_data_filter_exec(data);
	image_appw_draw(&data->appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void data_on_filter_clear(my1image_data_t *data, GtkMenuItem *menu_item)
{
	if (data->pfcurr)
	{
		filter_free_clones(data->pfcurr);
		data->pfcurr = 0x0;
	}
}
/*----------------------------------------------------------------------------*/
void data_on_filter_soloexec(my1image_data_t *data, GtkMenuItem *menu_item)
{
	my1image_t *save, *temp;
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	my1image_filter_t *pass = filter_search(data->pflist,name);
	if (pass)
	{
		image_copy_color2rgb(&data->work.main,&data->appw.view.buff);
		save = pass->output; /* just in case */
		pass->output = &data->work.buff;
		temp = image_filter_single(&data->work.main,pass);
		pass->output = save;
		image_copy(data->appw.show,temp);
		image_appw_draw(&data->appw,REDRAW);
	}
}
/*----------------------------------------------------------------------------*/
void data_on_list_current(my1image_data_t *data, GtkMenuItem *menu_item)
{
	int flag = 0;
	my1image_filter_t *temp;
	GList *curr_list, *next_list;
	GtkWidget *menu_main, *menu_temp, *menu_exec, *menu_clra;
	menu_main = gtk_menu_item_get_submenu(menu_item);
	/* remove all items? */
	curr_list = gtk_container_get_children(GTK_CONTAINER(menu_main));
	for (next_list=curr_list;next_list!=NULL;next_list=next_list->next)
		gtk_widget_destroy(GTK_WIDGET(next_list->data));
	g_list_free(curr_list);
	/* create executor */
	menu_exec = gtk_menu_item_new_with_label("Execute");
	g_signal_connect_swapped(G_OBJECT(menu_exec),"activate",
		G_CALLBACK(data_on_filter_execute),(gpointer)data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_exec);
	gtk_widget_set_sensitive(menu_exec,FALSE);
	gtk_widget_show(menu_exec);
	/* reset filter */
	menu_clra = gtk_menu_item_new_with_label("Clear All");
	g_signal_connect_swapped(G_OBJECT(menu_clra),"activate",
		G_CALLBACK(data_on_filter_clear),(gpointer)data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_clra);
	gtk_widget_set_sensitive(menu_clra,FALSE);
	gtk_widget_show(menu_clra);
	/* filter load menu */
	menu_temp = gtk_menu_item_new_with_label("Load...");
	g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
		G_CALLBACK(data_on_load_filter),(gpointer)data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* separator */
	menu_temp = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* create new items */
	temp = data->pfcurr;
	while (temp)
	{
		menu_temp = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
			G_CALLBACK(data_on_filter_soloexec),(gpointer)data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
		gtk_widget_show(menu_temp);
		temp = temp->next;
		flag++;
	}
	if (flag)
	{
		gtk_widget_set_sensitive(menu_exec,TRUE);
		gtk_widget_set_sensitive(menu_clra,TRUE);
	}
}
/*----------------------------------------------------------------------------*/
void image_data_domenu_filters(my1image_data_t* data, GtkWidget *menu_main)
{
	my1image_filter_t *temp;
	GtkWidget *menu_item, *menu_subs, *menu_temp;
	/* LIST ALL AVAILABLE FILTERS? */
	temp = data->pflist; menu_subs = 0x0;
	while (temp)
	{
		if (!menu_subs) menu_subs = gtk_menu_new();
		menu_item = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
			G_CALLBACK(data_on_filter_select),(gpointer)data);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
		gtk_widget_show(menu_item);
		temp = temp->next;
	}
	if (menu_subs)
	{
		/* temp menu to insert as sub-menu */
		menu_temp = gtk_menu_item_new_with_label("Filter Listed");
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
		/* add to main menu */
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
		gtk_widget_show(menu_temp);
		/* create sub menu for current filter? */
		menu_subs = gtk_menu_new();
		menu_temp = gtk_menu_item_new_with_label("Filter Picked");
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
		g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
			G_CALLBACK(data_on_list_current),(gpointer)data);
		gtk_widget_show(menu_temp);
	}
}
/*----------------------------------------------------------------------------*/
void image_data_domenu_extra(my1image_data_t* data)
{
	GtkWidget *menu_main, *menu_item;
	if (!data->appw.domenu) return;
	menu_main = data->appw.domenu;
	/* data menu item */
	menu_item = gtk_check_menu_item_new_with_label("Histogram");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item)
		,data->hist.dohide?FALSE:TRUE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_toggle_histogram),(gpointer)data);
	gtk_widget_show(menu_item);
}
/*----------------------------------------------------------------------------*/
void image_data_domenu(my1image_data_t* data)
{
	GtkWidget *menu_main;
	/* get main menu or create one */
	if (!data->appw.domenu)
		image_appw_domenu(&data->appw);
	dotask_make(&data->appw.clickL,data_on_clickL,(void*)data);
	dotask_make(&data->appw.clickM,data_on_clickM,(void*)data);
	menu_main = data->appw.domenu;
	/* add filters */
	image_data_domenu_filters(data,menu_main);
	image_appw_domenu_extra(&data->appw);
	image_data_domenu_extra(data);
	image_appw_domenu_quit(&data->appw);
}
/*----------------------------------------------------------------------------*/
void image_data_filter_more(my1image_data_t* data, filter_info_t* info)
{
	my1image_filter_t* temp = filter_search(data->pflist,info->name);
	if (temp)
	{
		/* cannot have duplicate name here! */
		data->flag = DATA_FLAG_ERROR;
		return;
	}
	temp = info_create_filter(info);
	if (temp)
		data->pflist = filter_insert(data->pflist,temp);
	else
		data->flag = DATA_FLAG_ERROR;
}
/*----------------------------------------------------------------------------*/
void image_data_filter_load(my1image_data_t* data, char* name)
{
	my1image_filter_t *ipass, *tpass = 0x0;
	ipass = filter_search(data->pflist,name);
	if (ipass) tpass = filter_cloned(ipass);
	if (tpass)
	{
		tpass->buffer = &data->work;
		data->pfcurr = filter_insert(data->pfcurr,tpass);
	}
}
/*----------------------------------------------------------------------------*/
void image_data_filter_exec(my1image_data_t* data)
{
	my1image_t *temp;
	if (data->pfcurr)
	{
		temp = data->appw.show;
		temp = image_filter(temp,data->pfcurr);
		image_copy(data->appw.show,temp);
		image_appw_draw(&data->appw,REDRAW);
	}
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_DATAC__ */
/*----------------------------------------------------------------------------*/
