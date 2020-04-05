/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_DATAC__
#define __MY1IMAGE_DATAC__
/*----------------------------------------------------------------------------*/
#include "my1image_data.h"
#include "my1image_file.h"
#include "my1image_chsv.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include <gdk/gdkkeysyms.h>
/*----------------------------------------------------------------------------*/
void image_data_histogram(void* data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	my1image_hist_t* hist = (my1image_hist_t*) view->draw_more_data;
	image_hist_show(hist);
}
/*----------------------------------------------------------------------------*/
void image_data_init(my1image_data_t* data)
{
	data->dosize = 0;
	data->maxh = DEFAULT_MAX_HEIGHT;
	data->maxw = DEFAULT_MAX_WIDTH;
	data->flag = DATA_FLAG_OK;
	data->image = 0x0;
	image_appw_init(&data->appw);
	data->appw.doshow = 1;
	data->appw.view.draw_more = (void*) &image_data_histogram;
	data->appw.view.draw_more_data = (void*) &data->hist;
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
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void image_data_make(my1image_data_t* data, my1image_t* that)
{
	if (that)
	{
		image_copy(&data->appw.main,that); /* keep original */
		image_copy(&data->appw.buff,that);
		data->image = &data->appw.buff;
	}
	if (data->dosize)
	{
		if (data->image->cols>data->maxw||data->image->rows>data->maxh)
		{
			data->image = image_size_this(data->image,data->work.next,
				data->maxh,data->maxw);
			buffer_swap(&data->work);
		}
	}
	image_appw_make(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void image_data_work(my1image_data_t* data)
{
	if (!data->pflist)
		data->pflist = image_work_create_all();
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
	if (data->pfcurr)
		data->image = image_filter(data->image,data->pfcurr);
}
/*----------------------------------------------------------------------------*/
void data_set_menu_position(GtkMenu *menu, gint *x, gint *y,
	gboolean *push_in, gpointer user_data)
{
	GtkWidget *window = (GtkWidget*)user_data;
	gdk_window_get_origin(window->window, x, y);
	*x += window->allocation.x + window->allocation.width/2;
	*y += window->allocation.y + window->allocation.height/2;
}
/*----------------------------------------------------------------------------*/
gboolean data_on_key_press(GtkWidget *widget, GdkEventKey *kevent,
	gpointer data)
{
	my1image_data_t *q = (my1image_data_t*) data;
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
			gtk_menu_popup(GTK_MENU(q->appw.domenu),0x0,0x0,
				&data_set_menu_position,(gpointer)q->appw.window,0x0,0x0);
			return TRUE;
		}
		else if(kevent->keyval == GDK_KEY_F||kevent->keyval == GDK_KEY_f)
		{
			q->appw.gofull = !q->appw.gofull;
			image_appw_full(&q->appw,q->appw.gofull);
			return TRUE;
		}
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
#define RIGHT_CLICK 3
#define MIDDLE_CLICK 2
#define LEFT_CLICK 1
/*----------------------------------------------------------------------------*/
gboolean data_on_mouse_click(GtkWidget *widget, GdkEventButton *event,
	gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		my1image_data_t *q = (my1image_data_t*) data;
		if (event->button == RIGHT_CLICK)
		{
			gtk_menu_popup(GTK_MENU(q->appw.domenu),0x0,0x0,0x0,0x0,
				event->button,event->time);
		}
		else if (event->button == MIDDLE_CLICK)
		{
			/* show info on status bar */
			gchar *buff;
			buff = g_strdup_printf("Size:%dx%d Mask:0x%08x",
				q->appw.view.buff.cols,q->appw.view.buff.rows,
				q->appw.view.buff.mask);
			image_appw_stat_time(&q->appw,(char*)buff,5);
			g_free(buff);
		}
		else if (event->button == LEFT_CLICK)
		{
			gchar *buff; my1rgb_t *temp; my1hsv_t that;
			int mask = q->appw.view.buff.mask;
			int dpix = image_get_pixel(&q->appw.view.buff,event->y,event->x);
			dpix &= mask; /* remove alpha */
			dpix = color_swap(dpix); /* get rgb from bgr */
			temp = (my1rgb_t*)&dpix;
			that = rgb2hsv(*temp);
			buff = g_strdup_printf("[PIXEL] %08X{%08X}<%d>@(%d,%d)",
				dpix,mask,that.h,(int)event->x,(int)event->y);
			image_appw_stat_time(&q->appw,(char*)buff,3);
			g_free(buff);
		}
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void data_on_image_original(my1image_data_t *data)
{
	image_copy(&data->appw.buff,&data->appw.main);
	data->image = &data->appw.buff;
	image_appw_draw(&data->appw,data->image);
	image_appw_stat_time(&data->appw,"Original Image restored!",1);
}
/*----------------------------------------------------------------------------*/
void data_on_image_grayscale(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	data->image = &data->appw.buff;
	image_grayscale(data->image);
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_invert(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	data->image = &data->appw.buff;
	image_invert_this(data->image);
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_binary(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	data->image = &data->appw.buff;
	image_grayscale(data->image);
	image_binary(data->image,0,WHITE);
	image_invert_this(data->image);
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_rotate_cw90(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	image_turn(&data->appw.buff,data->work.curr,IMAGE_TURN_270);
	data->image = data->work.curr;
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_rotate_ccw90(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	image_turn(&data->appw.buff,data->work.curr,IMAGE_TURN_090);
	data->image = data->work.curr;
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_flip_v(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	image_flip(&data->appw.buff,data->work.curr,IMAGE_FLIP_VERTICAL);
	data->image = data->work.curr;
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_image_flip_h(my1image_data_t *data)
{
	image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
	image_flip(&data->appw.buff,data->work.curr,IMAGE_FLIP_HORIZONTAL);
	data->image = data->work.curr;
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_toggle_histogram(my1image_data_t *data, GtkCheckMenuItem *menu_item)
{
	data->hist.dohide = !data->hist.dohide;
	gtk_check_menu_item_set_active(menu_item,data->hist.dohide?FALSE:TRUE);
	image_hist_show(&data->hist);
}
/*----------------------------------------------------------------------------*/
void data_on_toggle_aspectratio(my1image_data_t *data, GtkCheckMenuItem *menu_item)
{
	data->appw.view.aspect = !data->appw.view.aspect;
	gtk_check_menu_item_set_active(menu_item,data->appw.view.aspect?TRUE:FALSE);
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void data_on_limit_size(my1image_data_t *data, GtkCheckMenuItem *menu_item)
{
	data->dosize = !data->dosize;
	gtk_check_menu_item_set_active(menu_item,data->dosize?TRUE:FALSE);
	image_data_make(data,0x0); /* reevaluate size */
	image_appw_draw(&data->appw,data->image);
}
/*----------------------------------------------------------------------------*/
void image_data_events(my1image_data_t* data)
{
	g_signal_connect(G_OBJECT(data->appw.window),"key_press_event",
		G_CALLBACK(data_on_key_press),(gpointer)data);
}
/*----------------------------------------------------------------------------*/
void data_on_file_open_main(my1image_data_t* data)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Image File",
		GTK_WINDOW(data->appw.window),GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(doopen))==GTK_RESPONSE_ACCEPT)
	{
		int test;
		my1image_t that;
		gchar *filename, *buff;
		image_init(&that);
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		if((test=image_load(&that,filename))<0)
		{
			buff = g_strdup_printf("Error loading '%s'! (%d)",filename,test);
		}
		else
		{
			image_data_make(data,&that);
			buff = g_strdup_printf("[LOAD] '%s'",filename);
		}
		image_appw_stat_time(&data->appw,(char*)buff,STATUS_TIMEOUT);
		g_free(buff);
		g_free(filename);
		image_free(&that);
	}
	gtk_widget_destroy(doopen);
 }
/*----------------------------------------------------------------------------*/
void data_on_file_save_main(my1image_data_t* data)
{
	GtkWidget *dosave = gtk_file_chooser_dialog_new("Save Image File",
		GTK_WINDOW(data->appw.window),GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dosave),
		TRUE);
	if (gtk_dialog_run(GTK_DIALOG(dosave))==GTK_RESPONSE_ACCEPT)
	{
		int test;
		gchar *filename, *buff;
		my1image_t that;
		image_init(&that);
		image_copy_color2rgb(&that,&data->appw.view.buff);
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dosave));
		if ((test=image_save(&that,filename))<0)
			buff = g_strdup_printf("Error saving '%s'! (%d)",filename,test);
		else
			buff = g_strdup_printf("[SAVE] '%s'",filename);
		image_appw_stat_time(&data->appw,(char*)buff,STATUS_TIMEOUT);
		image_free(&that);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(dosave);
}
/*----------------------------------------------------------------------------*/
void data_on_load_filter(my1image_data_t* data)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Filter List",
		GTK_WINDOW(data->appw.window),GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
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
	image_appw_draw(&data->appw,data->image);
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
	my1image_t *save;
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	my1image_filter_t *pass = filter_search(data->pflist,name);
	if (pass)
	{
		image_copy_color2rgb(&data->appw.buff,&data->appw.view.buff);
		save = pass->output; /* just in case */
		pass->output = data->work.curr;
		data->image = image_filter_single(&data->appw.buff,pass);
		pass->output = save;
		image_appw_draw(&data->appw,data->image);
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
void image_data_domenu(my1image_data_t* data)
{
	my1image_filter_t *temp;
	GtkWidget *menu_main, *menu_item, *menu_subs, *menu_temp;
	/* in case already created! */
	if (data->appw.domenu) return;
	/* create popup menu for canvas */
	menu_main = gtk_menu_new();
	gtk_widget_add_events(data->appw.view.canvas, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(GTK_OBJECT(data->appw.view.canvas),"button-press-event",
		G_CALLBACK(data_on_mouse_click),(gpointer)data);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* file load menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Load Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_file_open_main),(gpointer)data);
	gtk_widget_show(menu_item);
	/* file save as menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Save Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_file_save_main),(gpointer)data);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_File Load/Save");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* original menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Original");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_original),(gpointer)data);
	gtk_widget_show(menu_item);
	/* grayscale menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Grayscale");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_grayscale),(gpointer)data);
	gtk_widget_show(menu_item);
	/* binary menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Binary");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_binary),(gpointer)data);
	gtk_widget_show(menu_item);
	/* invert menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Invert");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_invert),(gpointer)data);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Image");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* rotate menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Rotate CW90");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_rotate_cw90),(gpointer)data);
	gtk_widget_show(menu_item);
	/* rotate menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Rotate CCW90");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_rotate_ccw90),(gpointer)data);
	gtk_widget_show(menu_item);
	/* flip v menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Vertical");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_flip_v),(gpointer)data);
	gtk_widget_show(menu_item);
	/* flip h menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Horizontal");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_image_flip_h),(gpointer)data);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Orientation");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* sub menu? - LIST ALL AVAILABLE FILTERS? */
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
	/* data menu item */
	menu_item = gtk_check_menu_item_new_with_label("Histogram");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item)
		,data->hist.dohide?FALSE:TRUE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_toggle_histogram),(gpointer)data);
	gtk_widget_show(menu_item);
	/* keep aspect ratio menu item */
	menu_item = gtk_check_menu_item_new_with_label("Keep Aspect");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item)
		,data->appw.view.aspect?TRUE:FALSE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_toggle_aspectratio),(gpointer)data);
	gtk_widget_show(menu_item);
	/* limit size menu item */
	menu_item = gtk_check_menu_item_new_with_label("Limit Size");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item)
		,data->dosize?TRUE:FALSE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(data_on_limit_size),(gpointer)data);
	gtk_widget_show(menu_item);
	/* quit menu item */
	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",
		G_CALLBACK(gtk_main_quit),0x0);
	gtk_widget_show(menu_item);
	/* save that menu */
	data->appw.domenu = menu_main;
	/* show it! */
	gtk_widget_show(data->appw.domenu);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_DATAC__ */
/*----------------------------------------------------------------------------*/
