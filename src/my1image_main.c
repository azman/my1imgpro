/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MAIN_C__
#define __MY1IMAGE_MAIN_C__
/*----------------------------------------------------------------------------*/
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_main.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
void iwork_make(my1iwork_t* work, pdata_t data)
{
	dotask_make(&work->init,0x0,data);
	dotask_make(&work->free,0x0,data);
	dotask_make(&work->args,0x0,data);
	dotask_make(&work->prep,0x0,data);
	dotask_make(&work->proc,0x0,data);
	dotask_make(&work->show,0x0,data);
	work->data = data;
}
/*----------------------------------------------------------------------------*/
void imain_init(my1imain_t* imain, my1iwork_t* iwork)
{
	image_init(&imain->load);
	imain->show = 0x0;
	image_appw_init(&imain->iwin);
	igrab_init(&imain->grab);
	buffer_init(&imain->buff);
	imain->flag = IFLAG_OK;
	imain->tdel = MY1IMAIN_LOOP_DELAY;
	imain->dovf = 0x0;
	imain->list = 0x0;
	imain->curr = 0x0;
	imain->work = iwork;
	if (imain->work)
		dotask_exec(&imain->work->init,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
void imain_free(my1imain_t* imain)
{
	if (imain->work)
		dotask_exec(&imain->work->free,(void*)imain,0x0);
	if (imain->curr) filter_free_clones(imain->curr);
	if (imain->list) filter_free_clones(imain->list);
	buffer_free(&imain->buff);
	igrab_free(&imain->grab);
	image_appw_free(&imain->iwin);
	image_free(&imain->load);
	if ((imain->flag&IFLAG_ERROR_2END)==IFLAG_ERROR_2END)
		imain->flag = IFLAG_OK;
	if (imain->flag&IFLAG_ERROR)
		printf("-- [FLAG] %08x\n",imain->flag);
}
/*----------------------------------------------------------------------------*/
void imain_args(my1imain_t* imain, int argc, char* argv[])
{
	if (argc<2) imain->flag |= IFLAG_ERROR_2END;
	else
	{
		imain->grab.pick = argv[1];
		imain->grab.grab = &imain->load;
	}
	if (imain->work)
	{
		imain->work->args.xtra = (void*)imain;
		dotask_exec(&imain->work->args,(void*)&argc,(void*)argv);
	}
	igrab_grab_default(&imain->grab);
}
/*----------------------------------------------------------------------------*/
void imain_prep(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	if (!strncmp(imain->grab.pick,"--blank",7))
	{
		image_make(&imain->load,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&imain->load,BLACK);
		imain->show = &imain->load;
	}
	else
	{
		igrab_grab(&imain->grab);
		if (imain->grab.flag&IGRAB_FLAG_ERROR)
		{
			imain->flag |= IFLAG_ERROR_LOAD;
			return;
		}
		imain->show = imain->grab.grab;
	}
	if (imain->work)
		dotask_exec(&imain->work->prep,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
void imain_proc(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	imain_filter_doexec(imain);
	if (imain->work)
		dotask_exec(&imain->work->proc,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_execute(my1imain_t *imain, GtkMenuItem *menu_item)
{
	imain_filter_doexec(imain);
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_clear(my1imain_t *imain, GtkMenuItem *menu_item)
{
	if (imain->curr)
	{
		imain->flag |= IFLAG_FILTER_CHK;
		while (imain->flag&IFLAG_FILTER_RUN);
		filter_free_clones(imain->curr);
		imain->curr = 0x0;
		imain->flag &= ~IFLAG_FILTER_CHK;
	}
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_unload(my1imain_t *imain, GtkMenuItem *menu_item)
{
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	imain_filter_unload(imain,name);
}
/*----------------------------------------------------------------------------*/
void imain_menu_filter_enable(my1imain_t *imain, int enable)
{
	gboolean able = FALSE;
	GtkMenuItem *menu_item = (GtkMenuItem*)imain->dovf;
	if (!menu_item) return;
	if (enable) able = TRUE;
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),able);
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_toggle(my1imain_t *imain, GtkMenuItem *menu_item)
{
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menu_item)))
		imain->flag |= IFLAG_FILTER_EXE;
	else
		imain->flag &= ~IFLAG_FILTER_EXE;
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_load(my1imain_t* imain)
{
	GtkWidget *doopen = gtk_file_chooser_dialog_new("Open Filter List",
		GTK_WINDOW(imain->iwin.window),GTK_FILE_CHOOSER_ACTION_OPEN,
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
				imain_filter_doload(imain,fname);
			}
			fclose(pfile);
			buff = g_strdup_printf("[CHECK] '%s' loaded!",filename);
		}
		else
			buff = g_strdup_printf("[ERROR] Cannot load '%s'!",filename);
		printf("-- %s\n",(char*)buff);
		g_free(buff);
		g_free(filename);
	}
	gtk_widget_destroy(doopen);
}
/*----------------------------------------------------------------------------*/
void imain_domenu_current(my1imain_t *imain, GtkMenuItem *menu_item)
{
	int flag;
	my1ipass_t *temp;
	GList *curr_list, *next_list;
	GtkWidget *menu_main, *menu_temp, *menu_exec, *menu_clra;
	menu_main = gtk_menu_item_get_submenu(menu_item);
	/* remove all items? */
	curr_list = gtk_container_get_children(GTK_CONTAINER(menu_main));
	for (next_list=curr_list;next_list!=NULL;next_list=next_list->next)
		gtk_widget_destroy(GTK_WIDGET(next_list->data));
	g_list_free(curr_list);
	if (!(imain->flag&IFLAG_VIDEO_MODE))
	{
		/* create executor */
		menu_exec = gtk_menu_item_new_with_label("Execute");
		g_signal_connect_swapped(G_OBJECT(menu_exec),"activate",
			G_CALLBACK(imain_on_filter_execute),(gpointer)imain);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_exec);
		gtk_widget_set_sensitive(menu_exec,FALSE);
		gtk_widget_show(menu_exec);
	} else menu_exec = 0x0;
	/* reset filter */
	menu_clra = gtk_menu_item_new_with_label("Clear All");
	g_signal_connect_swapped(G_OBJECT(menu_clra),"activate",
		G_CALLBACK(imain_on_filter_clear),(gpointer)imain);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_clra);
	gtk_widget_set_sensitive(menu_clra,FALSE);
	gtk_widget_show(menu_clra);
	/* filter load menu */
	menu_temp = gtk_menu_item_new_with_label("Load...");
	g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
		G_CALLBACK(imain_on_filter_load),(gpointer)imain);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* separator */
	menu_temp = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
	gtk_widget_show(menu_temp);
	/* create new items */
	temp = imain->curr; flag = 0;
	while (temp)
	{
		gchar *buff = g_strdup_printf("%d-%s",flag,temp->name);
		menu_temp = gtk_menu_item_new_with_label(buff);
		g_free(buff);
		g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
			G_CALLBACK(imain_on_filter_unload),(gpointer)imain);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_temp);
		gtk_widget_show(menu_temp);
		temp = temp->next;
		flag++;
	}
	if (flag)
	{
		if (menu_exec)
			gtk_widget_set_sensitive(menu_exec,TRUE);
		gtk_widget_set_sensitive(menu_clra,TRUE);
	}
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_select(my1imain_t *imain, GtkMenuItem *menu_item)
{
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	imain_filter_doload(imain,name);
}
/*----------------------------------------------------------------------------*/
void imain_domenu_filters(my1imain_t* imain)
{
	my1ipass_t *temp;
	GtkWidget *menu_main, *menu_item, *menu_subs, *menu_temp;
	if (imain->iwin.domenu) menu_main = imain->iwin.domenu;
	else
	{
		menu_main = gtk_menu_new();
		imain->iwin.domenu = menu_main;
		gtk_widget_show(menu_main);
	}
	temp = imain->list; menu_subs = 0x0;
	while (temp)
	{
		if (!menu_subs) menu_subs = gtk_menu_new();
		menu_item = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
			G_CALLBACK(imain_on_filter_select),(gpointer)imain);
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
			G_CALLBACK(imain_domenu_current),(gpointer)imain);
		gtk_widget_show(menu_temp);
	}
	if (imain->flag&IFLAG_VIDEO_MODE)
	{
		menu_item = gtk_check_menu_item_new_with_label("Filtered");
		gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_item);
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu_item),
			imain->flag&IFLAG_FILTER_EXE?TRUE:FALSE);
		g_signal_connect_swapped(G_OBJECT(menu_item),"activate",
			G_CALLBACK(imain_on_filter_toggle),(gpointer)imain);
		gtk_widget_show(menu_item);
		imain->dovf = (void*) menu_item;
	}
}
/*----------------------------------------------------------------------------*/
void imain_show(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	if (imain->show)
		image_show(imain->show,&imain->iwin,"MY1 Image");
	if (imain->work)
		dotask_exec(&imain->work->show,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
int imain_on_task_timer(void* data, void* that, void* xtra)
{
	my1imain_t* imain = (my1imain_t*) data;
	my1ishow_t* ishow = (my1ishow_t*) that;
	igrab_grab(&imain->grab);
	imain->show = imain->grab.grab;
	imain_proc(imain);
	image_appw_make(ishow,imain->show);
	image_appw_task(ishow,imain_on_task_timer,imain->tdel);
	return 0;
}
/*----------------------------------------------------------------------------*/
void imain_loop(my1imain_t* imain, int delta_ms)
{
	if (delta_ms>0) imain->tdel = delta_ms;
	if (imain->iwin.dotask) return;
	imain->iwin.dodata = (void*) imain;
	image_appw_task(&imain->iwin,imain_on_task_timer,imain->tdel);
}
/*----------------------------------------------------------------------------*/
void imain_filter_dolist(my1imain_t* imain, filter_info_t* info)
{
	my1ipass_t* temp = filter_search(imain->list,info->name);
	if (temp)
	{
		imain->flag |= IFLAG_ERROR_LIST1;
		return;
	}
	temp = info_create_filter(info);
	if (temp) imain->list = filter_insert(imain->list,temp);
	else imain->flag |= IFLAG_ERROR_LIST2;
}
/*----------------------------------------------------------------------------*/
/**
void print_filters(my1ipass_t* pass)
{
	printf("** Filters: ");
	if (!pass) printf("None");
	printf("\n");
	while (pass)
	{
		printf("-- {%p:%s}=>(%p,%p)\n",pass,pass->name,pass->next,pass->last);
		pass = pass->next;
	}
	fflush(stdout);
}
**/
void imain_filter_doload(my1imain_t* imain, char* name)
{
	my1ipass_t *find, *temp = 0x0;
	find = filter_search(imain->list,name);
	if (find) temp = filter_cloned(find);
	if (temp)
	{
		imain->flag |= IFLAG_FILTER_CHK;
		while (imain->flag&IFLAG_FILTER_RUN);
		temp->buffer = &imain->buff;
		imain->curr = filter_insert(imain->curr,temp);
		/**print_filters(imain->curr);*/
		imain->flag &= ~IFLAG_FILTER_CHK;
	}
}
/*----------------------------------------------------------------------------*/
void imain_filter_unload(my1imain_t* imain, char* name)
{
	int size, loop = 0;
	my1ipass_t* pass = imain->curr;
	imain->flag |= IFLAG_FILTER_CHK;
	while (imain->flag&IFLAG_FILTER_RUN);
	while (name[0]!='-') { name++; } name++;
	while (pass)
	{
		if (pass->name)
		{
			size = strlen(pass->name) + 1;
			if (!strncmp(pass->name,name,size))
			{
				imain->curr = filter_remove(imain->curr,loop,1);
				/**print_filters(imain->curr);*/
				break;
			}
		}
		loop++;
		pass = pass->next;
	}
	imain->flag &= ~IFLAG_FILTER_CHK;
}
/*----------------------------------------------------------------------------*/
void imain_filter_doexec(my1imain_t* imain)
{
	my1image_t *temp;
	if (imain->flag&IFLAG_FILTER_EXE)
	{
		imain->flag |= IFLAG_FILTER_RUN;
		while (imain->flag&IFLAG_FILTER_CHK);
		if (imain->curr)
		{
			temp = imain->show;
			temp = image_filter(temp,imain->curr);
			if (temp!=imain->show)
				image_copy(imain->show,temp);
		}
		imain->flag &= ~IFLAG_FILTER_RUN;
	}
}
/*----------------------------------------------------------------------------*/
void image_show(my1image_t* image, my1ishow_t* ishow, char* name)
{
	image_appw_show(ishow,image,name,0);
	ishow->goquit = 1;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_MAIN_C__ */
/*----------------------------------------------------------------------------*/
