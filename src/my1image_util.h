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
/** NOT SURE WHY I HAVE THIS?! */
#define FILTER_FLAG_PROG 0x08
/*----------------------------------------------------------------------------*/
typedef struct _my1image_filter_t
{
	char name[FILTER_NAMESIZE];
	unsigned int temp, flag; /* pad temp, filter properties flag (1-hot) */
	void *data; /* filter extra data */
	my1ibuff_t *buffer; /* external shared buffer */
	my1image_t *output; /* output image to write to */
	pfilter_t filter; /* pointer to filter function */
	pfsetup_t doinit; /* pointer to setup function */
	pfclean_t dofree; /* pointer to cleanup function */
	struct _my1image_filter_t *next, *last; /* linked list - last in list */
	struct _my1image_filter_t *prev; /* ONLY FOR PROCESSING! */
}
my1image_filter_t;
/*----------------------------------------------------------------------------*/
typedef my1image_filter_t my1ipass_t;
/*----------------------------------------------------------------------------*/
void filter_init(my1ipass_t* pass, pfilter_t filter, my1ibuff_t *buff);
void filter_free(my1ipass_t* pass);
void filter_free_clones(my1ipass_t* pass);
my1ipass_t* filter_insert(my1ipass_t* pass, my1ipass_t* next);
my1ipass_t* filter_remove(my1ipass_t* pass, int index, int cloned);
my1ipass_t* filter_search(my1ipass_t* pass, char *name);
my1ipass_t* filter_cloned(my1ipass_t* pass);
/* apply filter on image */
my1image_t* image_filter(my1image_t* data, my1ipass_t* pass);
my1image_t* image_filter_single(my1image_t* data, my1ipass_t* pass);
/*----------------------------------------------------------------------------*/
typedef struct _filter_info_t
{
	char name[FILTER_NAMESIZE];
	unsigned int temp, flag; /* temp pad! */
	pfilter_t filter;
	pfsetup_t fsetup;
	pfclean_t fclean;
}
filter_info_t;
/*----------------------------------------------------------------------------*/
/* malloc'ed filter structure */
my1ipass_t* info_create_filter(filter_info_t* info);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_UTILH__ */
/*----------------------------------------------------------------------------*/
