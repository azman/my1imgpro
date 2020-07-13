/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_UTILC__
#define __MY1IMAGE_UTILC__
/*----------------------------------------------------------------------------*/
#include "my1image_util.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
#include <stdlib.h> /* for malloc and free? */
#include <string.h>
/*----------------------------------------------------------------------------*/
void filter_init(my1ifilter_t* pass, pfilter_t filter, my1ibuffer_t* buff)
{
	pass->name[0] = 0x0; /* anonymous */
	pass->flag = FILTER_FLAG_NONE;
	pass->data = 0x0;
	pass->buffer = buff;
	pass->output = 0x0;
	pass->filter = filter;
	pass->doinit = 0x0; /* used in cloning */
	pass->dofree = 0x0;
	pass->next = 0x0; /* linked list */
	pass->last = 0x0; /* last filter in chain */
	pass->prev = 0x0; /* will be set by the previous filter in exec chain */
}
/*----------------------------------------------------------------------------*/
void filter_free(my1ifilter_t* pass)
{
	if (pass->dofree)
		pass->dofree(pass);
}
/*----------------------------------------------------------------------------*/
void filter_free_clones(my1ifilter_t* pass)
{
	my1ifilter_t* temp;
	while (pass)
	{
		temp = pass;
		pass = pass->next;
		filter_free(temp);
		free((void*)temp);
	}
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* filter_insert(my1ifilter_t* pass, my1ifilter_t* next)
{
	next->next = 0x0;
	next->last = 0x0;
	if (!pass)
	{
		next->last = next;
		return next;
	}
	pass->last->next = next;
	pass->last = next;
	return pass;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* filter_remove(my1ifilter_t* pass, int index, int cloned)
{
	int loop = 0;
	my1ifilter_t *prev = 0x0, *curr = pass;
	while (curr)
	{
		if (index==loop)
		{
			if (prev)
			{
				prev->next = curr->next;
				if (!prev->next)
					pass->last = prev;
			}
			else
			{
				pass = curr->next;
				if (pass)
					pass->last = curr->last;
			}
			if (cloned)
			{
				filter_free(curr);
				free((void*)curr);
			}
			break;
		}
		prev = curr;
		curr = curr->next;
		loop++;
	}
	return pass;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* filter_search(my1ifilter_t* pass, char *name)
{
	int size;
	my1ifilter_t* find = 0x0;
	while (pass)
	{
		if (pass->name)
		{
			size = strlen(pass->name);
			if (!strncmp(pass->name,name,size+1))
			{
				find = pass;
				break;
			}
		}
		pass = pass->next;
	}
	return find;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* filter_cloned(my1ifilter_t* pass)
{
	my1ifilter_t* that = (my1ifilter_t*)malloc(sizeof(my1ifilter_t));
	if (that)
	{
		filter_init(that,pass->filter,pass->buffer);
		strncpy(that->name,pass->name,FILTER_NAMESIZE);
		that->doinit = pass->doinit;
		that->dofree = pass->dofree;
		if (that->doinit)
			that->doinit(that,pass);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_filter(my1image_t* data, my1ifilter_t* pass)
{
	my1image_t* temp;
	my1ibuffer_t* buff;
	my1ifilter_t* prev = 0x0;
	while (pass)
	{
		if (prev) pass->prev = prev;
		temp = pass->output;
		buff = 0x0;
		if (!temp)
		{
			buff = pass->buffer;
			if (!buff) return 0x0;
			temp = buff->next;
		}
		data = pass->filter(data,temp,pass);
		if (buff) buffer_swap(buff);
		prev = pass;
		pass = pass->next;
	}
	return data;
}
/*----------------------------------------------------------------------------*/
my1image_t* image_filter_single(my1image_t* data, my1ifilter_t* pass)
{
	my1image_t buff;
	/* assumes pass is always valid - output MUST BE defined */
	my1image_t* done = pass->output;
	if (!done) return data;
	image_init(&buff);
	if (pass->flag&FILTER_FLAG_GRAY)
	{
		image_copy(&buff,data);
		image_grayscale(&buff);
		data = &buff;
	}
	done = pass->filter(data,done,pass);
	image_free(&buff);
	return done;
}
/*----------------------------------------------------------------------------*/
my1ifilter_t* info_create_filter(filter_info_t* info)
{
	my1ifilter_t* that = (my1ifilter_t*)malloc(sizeof(my1ifilter_t));
	if (that)
	{
		filter_init(that,info->filter,0x0);
		strncpy(that->name,info->name,FILTER_NAMESIZE);
		that->flag = info->flag;
		that->doinit = info->fsetup;
		that->dofree = info->fclean;
		if (that->doinit)
			that->doinit(that,0x0);
	}
	return that;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_UTILC__ */
/*----------------------------------------------------------------------------*/
