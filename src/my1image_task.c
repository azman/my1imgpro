/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_TASK_C__
#define __MY1IMAGE_TASK_C__
/*----------------------------------------------------------------------------*/
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
void itask_make(my1itask_t* task, ptask_t func, pdata_t data)
{
	task->task = func;
	task->data = data;
}
/*----------------------------------------------------------------------------*/
int itask_call(my1itask_t* task, pdata_t that, pdata_t xtra)
{
	if (!task->task) return -1;
	return task->task((void*)task,that,xtra);
}
/*----------------------------------------------------------------------------*/
ptask_t itask_exec(my1itask_t* task, pdata_t that, pdata_t xtra)
{
	if (task->task)
		task->task((void*)task,that,xtra); /* ignore return value */
	return task->task;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_TASK_C__ */
/*----------------------------------------------------------------------------*/
