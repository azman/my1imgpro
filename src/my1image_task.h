/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_TASK_H__
#define __MY1IMAGE_TASK_H__
/*----------------------------------------------------------------------------*/
typedef void* pdata_t;
typedef int (*ptask_t)(pdata_t data, pdata_t that, pdata_t xtra);
/**
 * data = (pdata_t) dotask! so, we have xtra pdata_t (data&xtra)
 * that = usually main data struct
 * xtra = anything...
**/
/*----------------------------------------------------------------------------*/
typedef struct _my1dotask_t
{
	ptask_t task;
	pdata_t data; /** NOT data in ptask_t definition */
	pdata_t xtra; /** NOT xtra in ptask_t definition */
}
my1dotask_t;
/*----------------------------------------------------------------------------*/
void dotask_make(my1dotask_t* task, ptask_t func, pdata_t data);
int dotask_call(my1dotask_t* task, pdata_t that, pdata_t xtra);
ptask_t dotask_exec(my1dotask_t* task, pdata_t that, pdata_t xtra);
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_TASK_H__ */
/*----------------------------------------------------------------------------*/
