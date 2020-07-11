/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_TASK_H__
#define __MY1IMAGE_TASK_H__
/*----------------------------------------------------------------------------*/
typedef void* pdata_t;
typedef int (*ptask_t)(pdata_t, pdata_t, pdata_t);
/*----------------------------------------------------------------------------*/
typedef struct _my1itask_t
{
	ptask_t task;
/**
 * task will be called with:
 * arg0 = (pdata_t) itask object
 * arg1 = that pointer
 * arg2 = xtra pointer
**/
	pdata_t data;
	pdata_t temp;
}
my1itask_t;
/*----------------------------------------------------------------------------*/
void itask_make(my1itask_t* task, ptask_t func, pdata_t data);
int itask_call(my1itask_t* task, pdata_t that, pdata_t xtra);
ptask_t itask_exec(my1itask_t* task, pdata_t that, pdata_t xtra);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_TASK_H__ */
/*----------------------------------------------------------------------------*/
