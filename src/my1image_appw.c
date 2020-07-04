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
	appw->idtask = 0;
	appw->doquit = 0;
	appw->goquit = 1; /* by default, quit on close */
	appw->gofree = 0;
	appw->gofull = 0;
	appw->docopy = 1; /* by default, copy on make */
	appw->nostat = 0; /* by default, show status bar */
	appw->show = 0x0;
	appw->dotask = 0x0;
	appw->dodata = 0x0;
	appw->show = &appw->buff; /* show MUST always point to valid space */
	appw->orig = &appw->main; /* orig points to original image */
	image_init(&appw->main);
	image_init(&appw->buff);
	image_view_init(&appw->view);
	dotask_make(&appw->clickL,0x0,0x0);
	dotask_make(&appw->clickM,0x0,0x0);
	dotask_make(&appw->dodone,0x0,0x0);
	dotask_make(&appw->keychk,0x0,0x0);
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
		if (!appw->nostat)
			gtk_widget_show(appw->dostat);
		gtk_window_unfullscreen(GTK_WINDOW(appw->window));
	}
}
/*----------------------------------------------------------------------------*/
gboolean appw_on_done_all(gpointer data)
{
	my1image_appw_t* appw = (my1image_appw_t*) data;
	dotask_exec(&appw->dodone,0x0,0x0);
	if (appw->gofree) image_appw_free(appw);
	if (appw->goquit) gtk_main_quit();
	else appw->doquit = 1;
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gboolean appw_on_key_press(GtkWidget *widget, GdkEventKey *kevent,
	gpointer data)
{
	my1image_appw_t *appw = (my1image_appw_t*) data;
	if(kevent->type == GDK_KEY_PRESS)
	{
		/** g_message("%d, %c", kevent->keyval, kevent->keyval); */
		if(kevent->keyval == GDK_KEY_Escape||
			kevent->keyval == GDK_KEY_q)
		{
			appw_on_done_all(data);
			return TRUE;
		}
		else if(kevent->keyval == GDK_KEY_Return)
		{
			gtk_menu_popup_at_widget(GTK_MENU(appw->domenu),appw->window,
				GDK_GRAVITY_CENTER,GDK_GRAVITY_NORTH_WEST,0x0);
			return TRUE;
		}
		else if(kevent->keyval == GDK_KEY_f)
		{
			appw->gofull = !appw->gofull;
			image_appw_full(appw,appw->gofull);
			return TRUE;
		}
		else if (dotask_exec((void*)&appw->keychk,(void*)appw,(void*)kevent))
			return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
#define RIGHT_CLICK 3
#define MIDDLE_CLICK 2
#define LEFT_CLICK 1
/*----------------------------------------------------------------------------*/
gboolean appw_on_mouse_click(GtkWidget *widget,
	GdkEventButton *event, gpointer data)
{
	gboolean done = FALSE;
	if (event->type == GDK_BUTTON_PRESS)
	{
		my1image_appw_t *appw = (my1image_appw_t*) data;
		if (event->button == RIGHT_CLICK)
		{
			if (appw->domenu)
			{
				gtk_menu_popup_at_pointer(GTK_MENU(appw->domenu),0x0);
				done = TRUE;
			}
		}
		else if (event->button == MIDDLE_CLICK)
		{
			if (dotask_exec((void*)&appw->clickM,(void*)appw,(void*)event))
				done = TRUE;
		}
		else if (event->button == LEFT_CLICK)
		{
			if (dotask_exec((void*)&appw->clickL,(void*)appw,(void*)event))
				done = TRUE;
		}
	}
	return done;
}
/*----------------------------------------------------------------------------*/
void image_appw_make(my1image_appw_t* appw, my1image_t* that)
{
	int rows, cols;
	GtkAllocation alloc;
	GtkWidget* vbox;
	/* check if assigned new image */
	if (that&&that!=appw->show)
	{
		if (appw->docopy)
		{
			image_copy(&appw->main,that);
			image_copy(&appw->buff,that);
			appw->orig = &appw->main;
			appw->show = &appw->buff;
		}
		else appw->show = that; /* that MUST be a valid space */
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
		gtk_box_pack_end(GTK_BOX(vbox),appw->dostat,FALSE,FALSE,0);
		/* connect event handlers */
		g_signal_connect_swapped(G_OBJECT(appw->window),"delete-event",
			G_CALLBACK(appw_on_done_all),(gpointer)appw);
		g_signal_connect(G_OBJECT(appw->window),"key_press_event",
			G_CALLBACK(appw_on_key_press),(gpointer)appw);
		gtk_widget_add_events(appw->view.canvas, GDK_BUTTON_PRESS_MASK);
		g_signal_connect(appw->view.canvas,"button-press-event",
			G_CALLBACK(appw_on_mouse_click),(gpointer)appw);
		/* show window */
		gtk_widget_show_all(appw->window);
		if (appw->nostat) gtk_widget_hide(appw->dostat);
	}
	else image_view_make(&appw->view,that);
	rows = that->rows; cols = that->cols;
	if (gtk_widget_is_drawable(appw->dostat))
	{
		gtk_widget_get_allocation(appw->dostat,&alloc);
		rows += alloc.height;
	}
	/**printf("-- DoMake: %d x %d (%d)\n",cols,rows,that->rows);*/
	gtk_window_resize(GTK_WINDOW(appw->window),cols,rows);
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
	if (hide)
	{
		gtk_widget_hide(appw->dostat);
		appw->nostat = 1;
	}
	else
	{
		gtk_widget_show(appw->dostat);
		appw->nostat = 0;
	}
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
		my1image_t load;
		image_init(&load);
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		if((test=image_load(&load,filename))<0)
		{
			buff = g_strdup_printf("Error loading '%s'! (%d)",filename,test);
		}
		else
		{
			/* ALWAYS load to orig & show! */
			image_copy(appw->orig,&load);
			image_copy(appw->show,&load);
			image_appw_draw(appw,appw->show);
			buff = g_strdup_printf("[LOAD] '%s'",filename);
		}
		image_appw_stat_time(appw,(char*)buff,STATUS_TIMEOUT);
		image_free(&load);
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
	image_copy(appw->show,appw->orig);
	image_appw_draw(appw,appw->show);
	image_appw_stat_time(appw,"Original Image restored!",1);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_grayscale(my1image_appw_t *appw)
{
	image_copy_color2rgb(appw->show,&appw->view.buff);
	image_grayscale(appw->show);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_invert(my1image_appw_t *appw)
{
	image_copy_color2rgb(appw->show,&appw->view.buff);
	image_invert_this(appw->show);
	image_appw_draw(appw,REDRAW);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_rotate_cw90(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_turn(&temp,appw->show,IMAGE_TURN_270);
	image_appw_draw(appw,appw->show);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_rotate_ccw90(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_turn(&temp,appw->show,IMAGE_TURN_090);
	image_appw_draw(appw,appw->show);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_flip_v(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_flip(&temp,appw->show,IMAGE_FLIP_VERTICAL);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void appw_on_image_flip_h(my1image_appw_t *appw)
{
	my1image_t temp;
	image_init(&temp);
	image_copy_color2rgb(&temp,&appw->view.buff);
	image_flip(&temp,appw->show,IMAGE_FLIP_HORIZONTAL);
	image_appw_draw(appw,REDRAW);
	image_free(&temp);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_file(my1image_appw_t* appw, GtkWidget* menu_main)
{
	GtkWidget *menu_subs, *menu_item, *menu_temp;
	/* top menu */
	menu_subs = gtk_menu_new();
	/* file load menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Load Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_file_open_main),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* file save menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Save Image...");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_file_save_main),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* holder menu to insert as sub-menu (label) */
	menu_temp = gtk_menu_item_new_with_mnemonic("_File Load/Save");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_image(my1image_appw_t* appw, GtkWidget* menu_main)
{
	GtkWidget *menu_subs, *menu_item, *menu_temp;
	/* top menu */
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
	/* invert menu */
	menu_item = gtk_menu_item_new_with_mnemonic("_Invert");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_subs),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_image_invert),(gpointer)appw);
	gtk_widget_show(menu_item);
	/* temp menu to insert as sub-menu */
	menu_temp = gtk_menu_item_new_with_mnemonic("_Image");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_temp),menu_subs);
	/* add to main menu */
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_orientation(my1image_appw_t* appw, GtkWidget* menu_main)
{
	GtkWidget *menu_subs, *menu_item, *menu_temp;
	/* top menu */
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
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_extra(my1image_appw_t* appw)
{
	GtkWidget *menu_item;
	if (!appw->domenu) return;
	/* keep aspect ratio menu item */
	menu_item = gtk_check_menu_item_new_with_label("Keep Aspect");
	gtk_menu_shell_append(GTK_MENU_SHELL(appw->domenu),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
		appw->view.aspect?TRUE:FALSE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_toggle_aspectratio),(gpointer)appw);
	gtk_widget_show(menu_item);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_quit(my1image_appw_t* appw)
{
	GtkWidget *menu_item;
	if (!appw->domenu) return;
	/* quit menu item */
	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(appw->domenu),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(appw_on_done_all),(gpointer)appw);
	gtk_widget_show(menu_item);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu(my1image_appw_t* appw)
{
	GtkWidget *menu_main;
	if (appw->domenu) menu_main = appw->domenu;
	else
	{
		menu_main = gtk_menu_new();
		appw->domenu = menu_main;
		gtk_widget_show(appw->domenu);
	}
	image_appw_domenu_file(appw,menu_main);
	image_appw_domenu_image(appw,menu_main);
	image_appw_domenu_orientation(appw,menu_main);
}
/*----------------------------------------------------------------------------*/
void image_appw_domenu_full(my1image_appw_t* appw)
{
	image_appw_domenu(appw);
	image_appw_domenu_extra(appw);
	image_appw_domenu_quit(appw);
}
/*----------------------------------------------------------------------------*/
int image_appw_is_done(void* data, void* that, void* xtra)
{
	my1image_appw_t* appw = (my1image_appw_t*) that;
	if (appw->doquit) gtk_main_quit();
	else image_appw_task(appw,image_appw_is_done,ISDONE_TIMEOUT);
	return 0;
}
/*----------------------------------------------------------------------------*/
gboolean appw_on_timer_dotask(gpointer data)
{
	ptask_t task;
	my1image_appw_t *appw = (my1image_appw_t*) data;
	appw->idtask = 0;
	if (appw->dotask)
	{
		task = appw->dotask;
		appw->dotask = 0x0; /* in case task wants to reset itself */
		task(appw->dodata,(void*)appw,0x0);
	}
	return 0; /* a one-shot */
}
/*----------------------------------------------------------------------------*/
guint image_appw_task(my1image_appw_t* appw, ptask_t task, int usec)
{
	if (appw->dotask) return 0; /* cannot reassign, unless a one-shot */
	appw->dotask = task;
	appw->idtask = g_timeout_add(usec,appw_on_timer_dotask,(gpointer)appw);
	return appw->idtask;
}
/*----------------------------------------------------------------------------*/
void image_appw_show(my1image_appw_t* appw, my1image_t* that,
	char* name, int menu)
{
	image_appw_init(appw);
	appw->gofree = 1; /* auto free on close! */
	appw->goquit = 0; /* do not quit - assume there is another win! */
	appw->nostat = 1; /* hide status bar */
	image_appw_make(appw,that);
	if (name) image_appw_name(appw,name);
	if (menu) image_appw_domenu_full(appw);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_APPWC__ */
/*----------------------------------------------------------------------------*/
