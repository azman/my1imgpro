/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_APPWC__
#define __MY1IMAGE_APPWC__
/*----------------------------------------------------------------------------*/
#include "my1image_appw.h"
#include "my1image_crgb.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#include <string.h>
/*----------------------------------------------------------------------------*/
void image_appw_init(my1image_appw_t* appw)
{
	appw->window = 0x0;
	appw->domenu = 0x0;
	appw->dostat = 0x0;
	appw->idstat = 0;
	appw->idtime = 0;
	appw->doquit = 0;
	appw->goquit = 1; /* by default, quit on close */
	appw->gofree = 0;
	appw->gofull = 0;
	appw->doshow = 0;
	appw->show = 0x0;
	image_init(&appw->main);
	image_init(&appw->buff);
	image_view_init(&appw->view);
}
/*----------------------------------------------------------------------------*/
void image_appw_free(my1image_appw_t* appw)
{
	image_view_free(&appw->view);
	image_free(&appw->buff);
	image_free(&appw->main);
}
/*----------------------------------------------------------------------------*/
void image_appw_full(my1image_appw_t* appw, int full)
{
	if (full)
	{
		gtk_widget_hide(appw->dostat);
		gtk_window_fullscreen(GTK_WINDOW(appw->window));
	}
	else
	{
		gtk_widget_show(appw->dostat);
		gtk_window_unfullscreen(GTK_WINDOW(appw->window));
	}
}
/*----------------------------------------------------------------------------*/
gboolean appw_on_done_all(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	my1image_appw_t* appw = (my1image_appw_t*) data;
	if (appw->gofree) image_appw_free(appw);
	if (appw->goquit) gtk_main_quit();
	else appw->doquit = 1;
	return TRUE;
}
/*----------------------------------------------------------------------------*/
void image_appw_make(my1image_appw_t* appw, my1image_t* that)
{
	GtkAllocation alloc;
	GtkWidget* vbox;
	/* check if assigned new image */
	if (that)
	{
		if (appw->doshow) appw->show = that;
		else
		{
			/** original in main, send buff to view */
			image_copy(&appw->main,that);
			image_copy(&appw->buff,that);
			appw->show = &appw->buff;
		}
	}
	that = appw->show;
	/* must have image */
	if (!that) return;
	/* create window */
	if (!appw->window)
	{
		/* create gtk window */
		appw->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(appw->window),MAIN_WINDOW_TITLE);
		gtk_window_set_default_size(GTK_WINDOW(appw->window),
			that->cols,that->rows);
		gtk_window_set_position(GTK_WINDOW(appw->window),GTK_WIN_POS_CENTER);
		/* connect event handlers */
		g_signal_connect(G_OBJECT(appw->window),"delete-event",
			G_CALLBACK(appw_on_done_all),(gpointer)appw);
		/* container box for image */
		vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
		gtk_container_add(GTK_CONTAINER(appw->window),vbox);
		/* canvas stuff */
		image_view_make(&appw->view,that);
		gtk_box_pack_start(GTK_BOX(vbox),appw->view.canvas,TRUE,TRUE,0);
		/* create status bar */
		appw->dostat = gtk_statusbar_new();
		appw->idstat = gtk_statusbar_get_context_id(
			(GtkStatusbar*)appw->dostat,"MY1ImageViewStat");
		gtk_box_pack_start(GTK_BOX(vbox),appw->dostat,FALSE,FALSE,0);
		/* show window */
		gtk_widget_show_all(appw->window);
	}
	else image_view_make(&appw->view,that);
	gtk_widget_get_allocation(appw->dostat,&alloc);
	gtk_window_resize(GTK_WINDOW(appw->window),
		that->cols,that->rows+alloc.height);
}
/*----------------------------------------------------------------------------*/
void image_appw_draw(my1image_appw_t* appw, my1image_t* that)
{
	if (that) image_appw_make(appw,that);
	if (appw->view.canvas)
		gtk_widget_queue_draw(appw->view.canvas);
}
/*----------------------------------------------------------------------------*/
void image_appw_name(my1image_appw_t* appw,const char* name)
{
	/* must have window! */
	if (!appw->window) return;
	/* set title */
	gtk_window_set_title(GTK_WINDOW(appw->window),name);
}
/*----------------------------------------------------------------------------*/
void image_appw_stat_show(my1image_appw_t* appw, const char* mesg)
{
	if (!appw->dostat) return;
	/* remove all previous */
	gtk_statusbar_remove_all((GtkStatusbar*)appw->dostat,appw->idstat);
	/* show new */
	gtk_statusbar_push((GtkStatusbar*)appw->dostat,appw->idstat,mesg);
}
/*----------------------------------------------------------------------------*/
gboolean appw_on_timer_status(gpointer data)
{
	my1image_appw_t *appw = (my1image_appw_t*) data;
	appw->idtime = 0;
	gtk_statusbar_pop((GtkStatusbar*)appw->dostat,appw->idstat);
	return 0; /* a one-shot */
}
/*----------------------------------------------------------------------------*/
void image_appw_stat_time(my1image_appw_t* appw, const char* mesg, int secs)
{
	if (!appw->dostat) return;
	if (appw->idtime)
	{
		g_source_remove(appw->idtime);
		gtk_statusbar_pop((GtkStatusbar*)appw->dostat,appw->idstat);
	}
	gtk_statusbar_push((GtkStatusbar*)appw->dostat,appw->idstat,mesg);
	appw->idtime = g_timeout_add_seconds(secs,
		appw_on_timer_status,(gpointer)appw);
}
/*----------------------------------------------------------------------------*/
guint image_appw_stat_push(my1image_appw_t* appw, const char* mesg)
{
	if (!appw->dostat) return 0;
	return gtk_statusbar_push((GtkStatusbar*)appw->dostat,appw->idstat,mesg);
}
/*----------------------------------------------------------------------------*/
void image_appw_stat_pop(my1image_appw_t* appw)
{
	if (!appw->dostat) return;
	gtk_statusbar_pop((GtkStatusbar*)appw->dostat,appw->idstat);
}
/*----------------------------------------------------------------------------*/
void image_appw_stat_remove(my1image_appw_t* appw, guint mesg_id)
{
	if (!appw->dostat) return;
	gtk_statusbar_remove((GtkStatusbar*)appw->dostat,appw->idstat,mesg_id);
}
/*----------------------------------------------------------------------------*/
void image_appw_stat_hide(my1image_appw_t* appw, int hide)
{
	if (!appw->dostat) return;
	if (hide) gtk_widget_hide(appw->dostat);
	else gtk_widget_show(appw->dostat);
}
/*----------------------------------------------------------------------------*/
#define RIGHT_CLICK 3
/*----------------------------------------------------------------------------*/
gboolean appw_on_mouse_click(GtkWidget *widget,
	GdkEventButton *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		my1image_appw_t *q = (my1image_appw_t*) data;
		if (event->button == RIGHT_CLICK)
		{
			gtk_menu_popup_at_pointer(GTK_MENU(q->domenu),0x0);
		}
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void appw_on_file_open_main(my1image_appw_t* appw)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Image File",
		GTK_WINDOW(appw->window),GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(doopen))==GTK_RESPONSE_ACCEPT)
	{
		int test;
		gchar *filename, *buff;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		if((test=image_load(&appw->buff,filename))<0)
		{
			buff = g_strdup_printf("Error loading '%s'! (%d)",filename,test);
		}
		else
		{
			image_copy(&appw->main,&appw->buff);
			image_appw_make(appw,&appw->buff);
			image_appw_draw(appw,REDRAW);
			buff = g_strdup_printf("[LOAD] '%s'",filename);
		}
		image_appw_stat_time(appw,(char*)buff,STATUS_TIMEOUT);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(doopen);
 }
/*----------------------------------------------------------------------------*/
void appw_on_file_save_main(my1image_appw_t* appw)
{
	GtkWidget *dosave = gtk_file_chooser_dialog_new("Save Image File",
		GTK_WINDOW(appw->window),GTK_FILE_CHOOSER_ACTION_SAVE,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Save", GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dosave),
		TRUE);
	if (gtk_dialog_run(GTK_DIALOG(dosave))==GTK_RESPONSE_ACCEPT)
	{
		int test;
		gchar *filename, *buff;
		my1image_t that;
		image_init(&that);
		image_copy_color2rgb(&that,&appw->view.buff);
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dosave));
		if ((test=image_save(&that,filename))<0)
			buff = g_strdup_printf("Error saving '%s'! (%d)",filename,test);
		else
			buff = g_strdup_printf("[SAVE] '%s'",filename);
		image_appw_stat_time(appw,(char*)buff,STATUS_TIMEOUT);
		image_free(&that);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(dosave);
}
/*----------------------------------------------------------------------------*/
void appw_on_toggle_aspectratio(my1image_appw_t *appw,
	GtkCheckMenuItem *menu_item)
{
	appw->view.aspect = !appw->view.aspect;
	gtk_check_menu_item_set_active(menu_item,appw->view.aspect?TRUE:FALSE);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_original(my1image_appw_t *appw)
{
	image_copy(&appw->buff,&appw->main);
	image_appw_make(appw,&appw->buff);
	image_appw_draw(appw,REDRAW);
	image_appw_stat_time(appw,"Original Image restored!",1);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_grayscale(my1image_appw_t *appw)
{
	image_copy_color2rgb(&appw->buff,&appw->view.buff);
	image_grayscale(&appw->buff);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_binary(my1image_appw_t *appw)
{
	image_copy_color2rgb(&appw->buff,&appw->view.buff);
	image_grayscale(&appw->buff);
	image_binary(&appw->buff,0,WHITE);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_invert(my1image_appw_t *appw)
{
	image_copy_color2rgb(&appw->buff,&appw->view.buff);
	image_invert_this(&appw->buff);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_rotate_cw90(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_turn(&temp,&appw->buff,IMAGE_TURN_270);
	image_appw_make(appw,REDRAW);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_rotate_ccw90(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_turn(&temp,&appw->buff,IMAGE_TURN_090);
	image_appw_make(appw,REDRAW);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_flip_v(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_flip(&temp,&appw->buff,IMAGE_FLIP_VERTICAL);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_flip_h(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_flip(&temp,&appw->buff,IMAGE_FLIP_HORIZONTAL);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu(my1image_appw_t* appw)
{
	GtkWidget *menu_main, *menu_item, *menu_subs, *menu_temp, *that;
	/* in case already created! */
	if (appw->domenu) return;
	that = appw->view.canvas;
	/* create popup menu for canvas */
	menu_main = gtk_menu_new();
	gtk_widget_add_events(that, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(that,"button-press-event",
		G_CALLBACK(appw_on_mouse_click),(gpointer)appw);
	/* file load menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Load Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_file_open_main),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* file save as menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Save Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_file_save_main),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* sub menu? */
	menu_subs = gtk_menu_new();
	/* original menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Original");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_original),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* grayscale menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Grayscale");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_grayscale),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* binary menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Binary");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_binary),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* invert menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Invert");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_invert),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Process");
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
		G_CALLBACK(appw_on_image_rotate_cw90),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* rotate menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Rotate CCW90");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_rotate_ccw90),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* flip v menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Vertical");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_flip_v),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* flip h menu */
	menu_item = gtk_menu_item_new_with_mnemonic("Flip _Horizontal");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_flip_h),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Orientation");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* keep aspect ratio menu item */
	menu_item = gtk_check_menu_item_new_with_label("Keep Aspect");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
		appw->view.aspect?TRUE:FALSE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_toggle_aspectratio),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* quit menu item */
	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect(G_OBJECT(menu_item),"activate",
		G_CALLBACK(gtk_main_quit),0x0);
	gtk_widget_show(menu_item);
	/* save that menu */
	appw->domenu = menu_main;
	/* show it! */
	gtk_widget_show(appw->domenu);
}
/*----------------------------------------------------------------------------*/
void image_appw_show(my1image_appw_t* appw, my1image_t* that, char* name)
{
	image_appw_init(appw);
	appw->gofree = 1; /* auto free on close! */
	appw->goquit = 0; /* do not quit - assume there is another win! */
	image_appw_make(appw,that);
	if (name) image_appw_name(appw,name);
	image_appw_stat_hide(appw,1);
	image_appw_domenu(appw);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_APPWC__ */
/*----------------------------------------------------------------------------*/
