/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_UTILH__
#define __MY1IMAGE_UTILH__
/*----------------------------------------------------------------------------*/
#include "my1image_buff.h"
/*----------------------------------------------------------------------------*/
#define FILTER_NAMESIZE 80
/*----------------------------------------------------------------------------*/
struct _my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef my1image_t* (*pfilter_t)(my1image_t* curr, my1image_t* next,
	struct _my1image_filter_t* filter);
typedef void (*pfsetup_t)(struct _my1image_filter_t* filter,
	struct _my1image_filter_t* pclone);
typedef void (*pfclean_t)(struct _my1image_filter_t* filter);
/*----------------------------------------------------------------------------*/
#define FILTER_FLAG_NONE 0x00
#define FILTER_FLAG_GRAY 0x01
#define FILTER_FLAG_PROG 0x08
/*----------------------------------------------------------------------------*/
typedef struct _my1image_filter_t
{
	char name[FILTER_NAMESIZE];
	unsigned int flag; /* filter properties flag (1-hot) */
	void *data; /* filter extra data */
	my1image_buffer_t *buffer; /* external shared buffer */
	my1image_t *output; /* output image to write to */
	pfilter_t filter; /* pointer to filter function */
	pfsetup_t doinit; /* pointer to setup function */
	pfclean_t dofree; /* pointer to cleanup function */
	struct _my1image_filter_t *next, *last; /* linked list - last in list */
	struct _my1image_filter_t *prev; /* ONLY FOR PROCESSING! */
}
my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef my1image_filter_t my1ifilter_t;
/*----------------------------------------------------------------------------*/
void filter_init(my1ifilter_t* pass, pfilter_t filter, my1ibuffer_t *buff);
void filter_free(my1ifilter_t* pass);
void filter_free_clones(my1ifilter_t* pass);
my1ifilter_t* filter_insert(my1ifilter_t* pass, my1ifilter_t* next);
my1ifilter_t* filter_remove(my1ifilter_t* pass, int index, int cloned);
my1ifilter_t* filter_search(my1ifilter_t* pass, char *name);
my1ifilter_t* filter_cloned(my1ifilter_t* pass);
/* apply filter on image */
my1image_t* image_filter(my1image_t* data, my1ifilter_t* pass);
my1image_t* image_filter_single(my1image_t* data, my1ifilter_t* pass);
/*----------------------------------------------------------------------------*/
typedef struct _filter_info_t
{
	char name[FILTER_NAMESIZE];
	unsigned int flag;
	pfilter_t filter;
	pfsetup_t fsetup;
	pfclean_t fclean;
}
filter_info_t;
/*----------------------------------------------------------------------------*/
/* malloc'ed filter structure */
my1ifilter_t* info_create_filter(filter_info_t* info);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_UTILH__ */
/*----------------------------------------------------------------------------*/
