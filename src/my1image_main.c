/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_MAIN_C__
#define __MY1IMAGE_MAIN_C__
/*----------------------------------------------------------------------------*/
#include "my1image_main.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
int iwork_make(my1iwork_t* work, my1idata_t data)
{
	work->init = 0x0;
	work->free = 0x0;
	work->args = 0x0;
	work->prep = 0x0;
	work->proc = 0x0;
	work->show = 0x0;
	work->data = data;
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_init(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->init)
		return work->init(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_free(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->free)
		return work->free(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_args(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->args)
		return work->args(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_prep(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->prep)
		return work->prep(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_proc(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->proc)
		return work->proc(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
int iwork_show(my1iwork_t* work, my1idata_t that, my1idata_t xtra)
{
	if (work->show)
		return work->show(work->data,that,xtra);
	return 0;
}
/*----------------------------------------------------------------------------*/
void imain_init(my1imain_t* imain, my1iwork_t* iwork)
{
	image_init(&imain->main);
	image_init(&imain->temp);
	imain->show = 0x0;
	imain->orig = 0x0; /* switch to show original image */
	image_appw_init(&imain->iwin);
	filter_init(&imain->pass,0x0,0x0); /* no function & no buffer */
	imain->pass.output = &imain->main; /* writes to main storage */
	buffer_init(&imain->buff);
	imain->flag = IFLAG_OK;
	imain->pick = 0x0;
	imain->list = 0x0;
	imain->curr = 0x0;
	imain->pchk = 0x0; /* pure pointer - filter switch */
	imain->work = iwork;
	if (imain->work)
		imain->flag |= iwork_init(imain->work,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
void imain_free(my1imain_t* imain)
{
	if (imain->work)
		imain->flag |= iwork_free(imain->work,(void*)imain,0x0);
	if (imain->curr) filter_free_clones(imain->curr);
	if (imain->list) filter_free_clones(imain->list);
	buffer_free(&imain->buff);
	filter_free(&imain->pass);
	image_appw_free(&imain->iwin);
	image_free(&imain->temp);
	image_free(&imain->main);
}
/*----------------------------------------------------------------------------*/
void imain_args(my1imain_t* imain, int argc, char* argv[])
{
	if (argc<2) imain->flag |= IFLAG_ERROR_ARGS;
	else  imain->pick = argv[1];
	if (imain->work)
		imain->flag |= iwork_args(imain->work,(void*)&argc,(void*)argv);
}
/*----------------------------------------------------------------------------*/
void imain_prep(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	if (!strncmp(imain->pick,"--blank",7))
	{
		image_make(&imain->temp,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&imain->temp,BLACK);
	}
	else if (image_load(&imain->temp,imain->pick)<0)
	{
		imain->flag |= IFLAG_ERROR_LOAD;
		return;
	}
	/* check pre-filter */
	if (imain->pass.filter)
		imain->pass.filter(&imain->temp,&imain->main,&imain->pass);
	else
		image_copy(&imain->main,&imain->temp);
	imain->show = &imain->main;
	if (imain->work)
		imain->flag |= iwork_prep(imain->work,(void*)imain,0x0);
}
/*----------------------------------------------------------------------------*/
void imain_proc(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	imain_filter_doexec(imain);
	if (imain->work)
		imain->flag |= iwork_proc(imain->work,(void*)imain,0x0);
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
		filter_free_clones(imain->curr);
		imain->curr = 0x0;
	}
}
/*----------------------------------------------------------------------------*/
void imain_on_filter_unload(my1imain_t *imain, GtkMenuItem *menu_item)
{
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	imain_filter_unload(imain,name);
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
	int flag = 0;
	my1ipass_t *temp;
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
		G_CALLBACK(imain_on_filter_execute),(gpointer)imain);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_main),menu_exec);
	gtk_widget_set_sensitive(menu_exec,FALSE);
	gtk_widget_show(menu_exec);
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
	temp = imain->curr;
	while (temp)
	{
		menu_temp = gtk_menu_item_new_with_label(temp->name);
		g_signal_connect_swapped(G_OBJECT(menu_temp),"activate",
			G_CALLBACK(imain_on_filter_unload),(gpointer)imain);
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
void imain_on_filter_select(my1imain_t *imain, GtkMenuItem *menu_item)
{
	char* name = (char*)gtk_menu_item_get_label(menu_item);
	imain_filter_doload(imain,name);
}
/*----------------------------------------------------------------------------*/
void imain_domenu_filters(my1imain_t* imain, GtkWidget *menu_main)
{
	my1ipass_t *temp;
	GtkWidget *menu_item, *menu_subs, *menu_temp;
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
}
/*----------------------------------------------------------------------------*/
void imain_show(my1imain_t* imain)
{
	if (imain->flag&IFLAG_ERROR) return;
	if (imain->show)
	{
		image_show(imain->show,&imain->iwin,"MY1 Image");
		/* menu! */
		image_appw_domenu(&imain->iwin);
		imain_domenu_filters(imain,imain->iwin.domenu);
		image_appw_domenu_quit(&imain->iwin);
	}
	if (imain->work)
		imain->flag |= iwork_show(imain->work,(void*)imain,0x0);
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
void imain_filter_doload(my1imain_t* imain, char* name)
{
	my1ipass_t *find, *temp = 0x0;
	find = filter_search(imain->list,name);
	if (find) temp = filter_cloned(find);
	if (temp)
	{
		temp->buffer = &imain->buff;
		imain->curr = filter_insert(imain->curr,temp);
	}
}
/*----------------------------------------------------------------------------*/
void imain_filter_unload(my1imain_t* imain, char* name)
{
	my1ipass_t *prev, *curr;
	int size = strlen(name);
	prev = 0x0; curr = imain->curr;
	while (curr)
	{
		if (!strncmp(curr->name,name,size+1))
		{
			if (prev) prev->next = curr->next;
			else imain->curr = curr->next;
			/* free this */
			filter_free(curr);
			free((void*)curr);
			break;
		}
		prev = curr;
		curr = curr->next;
	}
}
/*----------------------------------------------------------------------------*/
void imain_filter_enable(my1imain_t* imain, int enable)
{
	if (enable) imain->pchk = imain->curr;
	else imain->pchk = 0x0;
}
/*----------------------------------------------------------------------------*/
void imain_filter_doexec(my1imain_t* imain)
{
	my1image_t *temp;
	if (imain->pchk)
	{
		temp = imain->iwin.show;
		temp = image_filter(temp,imain->pchk);
		if (temp!=imain->iwin.show)
			image_copy(imain->iwin.show,temp);
		image_appw_draw(&imain->iwin,REDRAW);
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
