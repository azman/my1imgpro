/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEO_MAINC__
#define __MY1VIDEO_MAINC__
/*----------------------------------------------------------------------------*/
#include "my1video_main.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
/** need this to save a frame from video to file! */
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
void video_main_draw_index(void* data)
{
	my1image_view_t* view = (my1image_view_t*) data;
	my1vmain_t* vmain = (my1vmain_t*) view->draw_more_data;
	my1video_t* video = &vmain->video;
	my1vgrab_t* vgrab = &vmain->vgrab;
	gchar *buff = 0x0;
	if (video->count<0) return;
	cairo_set_source_rgb(view->dodraw,0.0,0.0,1.0);
	cairo_move_to(view->dodraw,20,20);
	cairo_set_font_size(view->dodraw,12);
	buff = g_strdup_printf("%d/%d(%d)",video->index,video->count,vgrab->index);
	cairo_show_text(view->dodraw,buff);
	cairo_stroke(view->dodraw);
	g_free(buff);
}
/*----------------------------------------------------------------------------*/
void video_main_check_input(void* data)
{
	my1vmain_t* vmain = (my1vmain_t*) data;
	my1vappw_t* vappw = (my1vappw_t*) &vmain->vappw;
	GtkWidget* menu = (GtkWidget*) vappw->domenu;
	if (!menu) return;
	switch ((guint)vmain->ikey)
	{
		case GDK_KEY_Return:
			video_hold(&vmain->video,1);
			gtk_menu_popup_at_widget(GTK_MENU(vappw->domenu),vappw->window,
				GDK_GRAVITY_CENTER,GDK_GRAVITY_NORTH_WEST,0x0);
			vmain->ikey = 0;
			break;
	}
}
/*----------------------------------------------------------------------------*/
void video_main_init(my1vmain_t* vmain)
{
	video_init(&vmain->video);
	vmain->video.parent_ = (void*)vmain;
	capture_init(&vmain->vgrab,&vmain->video);
	image_appw_init(&vmain->vappw);
	vmain->video.display = (void*)&vmain->vappw;
	vmain->plist = image_work_create_all();
	vmain->ikey = 0;
	vmain->type = VIDEO_SOURCE_NONE;
	vmain->xdel = VGRAB_DELAY;
	/* pointers to functions and data */
	vmain->data = 0x0;
	vmain->grabber = 0x0;
	vmain->grabber_data = 0x0;
	vmain->vappw.view.draw_more = video_main_draw_index;
	vmain->vappw.view.draw_more_data = (void*) vmain;
	vmain->doinput = video_main_check_input;
	vmain->doinput_data = (void*) vmain;
}
/*----------------------------------------------------------------------------*/
void video_main_free(my1vmain_t* vmain)
{
	/* using dynamically created filters */
	video_main_pass_done(vmain);
	if (vmain->plist)
		filter_free_clones(vmain->plist);
	image_appw_free(&vmain->vappw);
	capture_free(&vmain->vgrab);
	video_free(&vmain->video);
}
/*----------------------------------------------------------------------------*/
void video_main_capture(my1vmain_t* vmain, char* vsrc, int type)
{
	/* setup capture */
	switch (type)
	{
		case VIDEO_SOURCE_LIVE:
			vmain->type = type;
			capture_live(&vmain->vgrab,vsrc);
			break;
		case VIDEO_SOURCE_FILE:
			vmain->type = type;
			capture_file(&vmain->vgrab,vsrc);
			break;
		default:
			vmain->type = VIDEO_SOURCE_NONE;
			return;
	}
}
/*----------------------------------------------------------------------------*/
void video_main_display(my1vmain_t* vmain, char* name)
{
	image_appw_draw(&vmain->vappw,vmain->video.frame);
	image_appw_name(&vmain->vappw,name);
	image_appw_stat_hide(&vmain->vappw,1);
}
/*----------------------------------------------------------------------------*/
void vmain_on_filter_select(my1video_main_t *vmain, GtkMenuItem *menu_item)
{
	video_main_pass_load(vmain,(char*)gtk_menu_item_get_label(menu_item));
}
/*----------------------------------------------------------------------------*/
void vmain_on_filter_remove(my1video_main_t *vmain, GtkMenuItem *menu_item)
{
	int size;
	my1video_t *that;
	my1vpass_t *prev, *curr;
	char *name = (char*)gtk_menu_item_get_label(menu_item);
	size = strlen(name);
	that = &vmain->video;
	prev = 0x0; curr = that->ppass;
	while (curr)
	{
		if (!strncmp(curr->name,name,size))
		{
			if (prev) prev->next = curr->next;
			else that->ppass = curr->next;
			/* free this */
			filter_free(curr);
			free((void*)curr);
		}
		prev = curr;
		curr = curr->next;
	}
}
/*----------------------------------------------------------------------------*/
void vmain_on_filter_clear(my1video_main_t *vmain, GtkMenuItem *menu_item)
{
	video_main_pass_done(vmain);
}
/*----------------------------------------------------------------------------*/
void vmain_on_load_filter(my1vmain_t* vmain)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Filter List",
		GTK_WINDOW(vmain->vappw.window),GTK_FILE_CHOOSER_ACTION_OPEN,
		"_Cancel", GTK_RESPONSE_CANCEL,
		"_Open", GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run(GTK_DIALOG(doopen))==GTK_RESPONSE_ACCEPT)
	{
		FILE* pfile;
		char fname[FILTER_NAMESIZE], *pname;
		gchar *filename, *buff;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(doopen));
		pfile = fopen((char*)filename,"rt");
		if (pfile)
		{
			while (fgets(fname,FILTER_NAMESIZE,pfile))
			{
				pname = fname;
				if (pname[0]=='#') continue; /* skip those starting with # */
				while (*pname)
				{
					if (pname[0]=='\n'||pname[0]==' '||
							pname[0]=='\t'||pname[0]=='\r')
					{
						pname[0] = 0x0;
						break;
					}
					pname++;
				}
				video_main_pass_load(vmain,fname);
			}
			fclose(pfile);
			buff = g_strdup_printf("[CHECK] '%s' loaded!",filename);
		}
		else
			buff = g_strdup_printf("[ERROR] Cannot load '%s'!",filename);
		image_appw_stat_time(&vmain->vappw,(char*)buff,5);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(doopen);
}
/*----------------------------------------------------------------------------*/
void vmain_on_list_current(my1video_main_t *vmain, GtkMenuItem *menu_item)
{
	int flag = 0;
	my1image_filter_t *temp;
	GList *curr_list, *next_list;
	GtkWidget *menu_main, *menu_temp, *menu_clra;
	menu_main = gtk_menu_item_get_submenu(menu_item);
	/* remove all items? */
	curr_list = gtk_container_get_children(GTK_CONTAINER(menu_main));
	for (next_list=curr_list;next_list!=NULL;next_list=next_list->next)
		gtk_widget_destroy(GTK_WIDGET(next_list->data));
	g_list_free(curr_list);
	/* reset filter */
	menu_clra = gtk_menu_item_new_with_label("Clear All");
	g_signal_connect_swapped(G_OBJECT(menu_clra),"activate",
		G_CALLBACK(vmain_on_filter_clear),(gpointer)vmain);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_clra);
	gtk_widget_set_sensitive(menu_clra,FALSE);
	gtk_widget_show(menu_clra);
	/* filter load menu */
	menu_temp = gtk_menu_item_new_with_label("Load...");
	g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
		G_CALLBACK(vmain_on_load_filter),(gpointer)vmain);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* separator */
	menu_temp = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* create new items */
	temp = vmain->video.ppass;
	while (temp)
	{
		menu_temp = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
			G_CALLBACK(vmain_on_filter_remove),(gpointer)vmain);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
		gtk_widget_show(menu_temp);
		temp = temp->next;
		flag++;
	}
	if (flag)
		gtk_widget_set_sensitive(menu_clra,TRUE);
}
/*----------------------------------------------------------------------------*/
void vmain_on_filter_toggle(my1video_main_t *vmain, GtkMenuItem *menu_item)
{
	my1video_t* video = &vmain->video;
	my1vappw_t* vappw = &vmain->vappw;
	if (video->flags&VIDEO_FLAG_NO_FILTER)
	{
		video_skip_filter(video,0);
		image_appw_stat_time(vappw,"Filter ON",MESG_SHOWTIME);
	}
	else
	{
		video_skip_filter(video,1);
		image_appw_stat_time(vappw,"Filter OFF",MESG_SHOWTIME);
	}
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
		video->flags&VIDEO_FLAG_NO_FILTER?FALSE:TRUE);
}
/*----------------------------------------------------------------------------*/
gboolean vmain_on_done_all(gpointer data)
{
	my1vmain_t* vmain = (my1vmain_t*) data;
	vmain->vappw.doquit = 1;
	return TRUE;
}
/*----------------------------------------------------------------------------*/
gint vmain_on_display_key_press(GtkWidget *widget, GdkEventKey *kevent,
	gpointer data)
{
	if(kevent->type == GDK_KEY_PRESS)
	{
		my1vmain_t *vmain = (my1vmain_t*) data;
		/** g_message("%d, %c", kevent->keyval, kevent->keyval); */
		vmain->ikey = kevent->keyval;
		return TRUE;
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
#define RIGHT_CLICK 3
/*----------------------------------------------------------------------------*/
gboolean vmain_on_mouse_click(GtkWidget *widget,
	GdkEventButton *event, gpointer data)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		my1vmain_t *vmain = (my1vmain_t*) data;
		if (event->button==RIGHT_CLICK)
		{
			GtkWidget* menu = (GtkWidget*) vmain->vappw.domenu;
			if (!menu) return FALSE;
			gtk_menu_popup_at_pointer(GTK_MENU(menu),0x0);
			return TRUE;
		}
	}
	return FALSE;
}
/*----------------------------------------------------------------------------*/
void video_main_prepare(my1vmain_t* vmain)
{
	GtkWidget *menu_main, *menu_item, *menu_subs, *menu_temp;
	my1image_filter_t *temp;
	/* in case already assigned */
	if (vmain->vappw.domenu) return;
	/* create popup menu for canvas */
	menu_main = gtk_menu_new();
	/* sub menu? - LIST ALL AVAILABLE FILTERS? */
	temp = vmain->plist; menu_subs = 0x0;
	while (temp)
	{
		if (!menu_subs) menu_subs = gtk_menu_new();
		menu_item = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
			G_CALLBACK(vmain_on_filter_select),(gpointer)vmain);
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
			G_CALLBACK(vmain_on_list_current),(gpointer)vmain);
		gtk_widget_show(menu_temp);
	}
	/* Filter */
	menu_item = gtk_check_menu_item_new_with_label("Filtered");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
		vmain->video.flags&VIDEO_FLAG_NO_FILTER?FALSE:TRUE);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(vmain_on_filter_toggle),(gpointer)vmain);
	gtk_widget_show(menu_item);
	/* quit menu item */
	menu_item = gtk_menu_item_new_with_mnemonic("_Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
	g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
		G_CALLBACK(vmain_on_done_all),(gpointer)vmain);
	gtk_widget_show(menu_item);
	/* save that menu */
	vmain->vappw.domenu = menu_main;
	/* show it! */
	gtk_widget_show(vmain->vappw.domenu);
	/* setup gtk signal handler(s) */
	g_signal_connect(G_OBJECT(vmain->vappw.window),"key_press_event",
		G_CALLBACK(vmain_on_display_key_press),(gpointer)vmain);
	g_signal_connect(G_OBJECT(vmain->vappw.view.canvas),
		"button-press-event",G_CALLBACK(vmain_on_mouse_click),(gpointer)vmain);
	gtk_widget_add_events(vmain->vappw.view.canvas,GDK_BUTTON_PRESS_MASK);
}
/*----------------------------------------------------------------------------*/
void vmain_on_display_timer(void* data)
{
	my1vappw_t* vappw = (my1vappw_t*) data;
	my1vmain_t* vmain = (my1vmain_t*) vappw->dodata;
	my1video_t* video = (my1video_t*) &vmain->video;
	my1vgrab_t* vgrab = (my1vgrab_t*) &vmain->vgrab;
	guint keyval = vmain->ikey;
	if (vgrab->fcontext) capture_grab(vgrab);
	else if (vmain->grabber)
		vmain->grabber(vmain->grabber_data);
	if (video->flags&VIDEO_FLAG_NEW_FRAME)
	{
		video_filter_frame(video);
		video_post_frame(video);
		image_appw_draw(vappw,video->frame);
	}
	if (keyval == GDK_KEY_Escape || keyval == GDK_KEY_q || vappw->doquit)
	{
		gtk_main_quit();
	}
	else if (keyval == GDK_KEY_c)
	{
		video_play(video);
		image_appw_stat_time(vappw,"Play",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_s)
	{
		video_stop(video);
		image_appw_stat_time(vappw,"Stop",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_space)
	{
		if (video->index!=video->count||video->flags&VIDEO_FLAG_LOOP)
		{
			if (video->flags&VIDEO_FLAG_IS_PAUSED)
			{
				video_hold(video,0);
				image_appw_stat_time(vappw,"Play",MESG_SHOWTIME);
			}
			else
			{
				video_hold(video,1);
				image_appw_stat_time(vappw,"Paused",MESG_SHOWTIME);
			}
		}
	}
	else if (keyval == GDK_KEY_f)
	{
		video_next_frame(video);
		image_appw_stat_time(vappw,"Next",MESG_SHOWTIME);
	}
	else if (keyval == GDK_KEY_b)
	{
		if (video->count<0)
		{
			image_appw_stat_time(vappw,"Not during live feed!",MESG_SHOWTIME);
		}
		else
		{
			video_prev_frame(video);
			image_appw_stat_time(vappw,"Previous",MESG_SHOWTIME);
		}
	}
	else if (keyval == GDK_KEY_l)
	{
		if (video->flags&VIDEO_FLAG_LOOP)
		{
			video_loop(video,0);
			image_appw_stat_time(vappw,"Looping OFF",MESG_SHOWTIME);
		}
		else
		{
			video_loop(video,1);
			image_appw_stat_time(vappw,"Looping ON",MESG_SHOWTIME);
		}
	}
	else if (keyval == GDK_KEY_g)
	{
		GtkWidget *dosave;
		/* stop video on grabbing */
		video_stop(video);
		/* create save dialog */
		dosave = gtk_file_chooser_dialog_new("Save Image File",
			GTK_WINDOW(vappw->window),GTK_FILE_CHOOSER_ACTION_SAVE,
			"_Cancel", GTK_RESPONSE_CANCEL,
			"_Open", GTK_RESPONSE_ACCEPT, NULL);
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dosave),
			TRUE);
		/* show it! */
		if (gtk_dialog_run(GTK_DIALOG(dosave))==GTK_RESPONSE_ACCEPT)
		{
			gchar *filename;
			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dosave));
			if (image_save(video->frame,filename))
				image_appw_stat_time(vappw,"Grab failed!",MESG_SHOWTIME);
			else
				image_appw_stat_time(vappw,"Frame saved!",MESG_SHOWTIME);
			g_free(filename);
		}
		gtk_widget_destroy(dosave);
	}
	else if (keyval == GDK_KEY_z)
	{
		if (video->flags&VIDEO_FLAG_NO_FILTER)
		{
			video_skip_filter(video,0);
			image_appw_stat_time(vappw,"Filter ON",MESG_SHOWTIME);
		}
		else
		{
			video_skip_filter(video,1);
			image_appw_stat_time(vappw,"Filter OFF",MESG_SHOWTIME);
		}
	}
	else if (keyval)
	{
		vmain->ikey = keyval;
		if (vmain->doinput)
			vmain->doinput((void*)vmain);
	}
	video_post_input(video);
	image_appw_task(&vmain->vappw,vmain_on_display_timer,vmain->xdel);
}
/*----------------------------------------------------------------------------*/
void video_main_loop(my1vmain_t* vmain, int loopms)
{
	if (loopms>0) vmain->xdel = loopms;
	if (vmain->vappw.dotask) return;
	vmain->vappw.dodata = (void*) vmain;
	image_appw_task(&vmain->vappw,vmain_on_display_timer,vmain->xdel);
}
/*----------------------------------------------------------------------------*/
void video_main_pass_more(my1vmain_t* vmain, filter_info_t* info)
{
	my1vpass_t* temp = filter_search(vmain->plist,info->name);
	if (temp) return;
	temp = info_create_filter(info);
	if (temp) vmain->plist = filter_insert(vmain->plist,temp);
}
/*----------------------------------------------------------------------------*/
void video_main_pass_load(my1vmain_t* vmain, char* name)
{
	my1vpass_t *ipass, *tpass = 0x0;
	if (!vmain->plist) return;
	ipass = filter_search(vmain->plist,name);
	if (ipass) tpass = filter_cloned(ipass);
	if (tpass) video_filter_insert(&vmain->video,tpass);
}
/*----------------------------------------------------------------------------*/
void video_main_pass_done(my1vmain_t* vmain)
{
	if (!vmain->video.ppass) return;
	filter_free_clones(vmain->video.ppass);
	vmain->video.ppass = 0x0;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1VIDEO_MAINC__ */
/*----------------------------------------------------------------------------*/
