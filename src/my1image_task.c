/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_TASK_C__
#define __MY1IMAGE_TASK_C__
/*----------------------------------------------------------------------------*/
#include "my1image_task.h"
/*----------------------------------------------------------------------------*/
void dotask_make(my1dotask_t* task, ptask_t func, pdata_t data)
{
	task->task = func;
	task->data = data;
}
/*----------------------------------------------------------------------------*/
int dotask_call(my1dotask_t* task, pdata_t that, pdata_t xtra)
{
	if (!task->task) return -1;
	return task->task((void*)task,that,xtra);
}
/*----------------------------------------------------------------------------*/
ptask_t dotask_exec(my1dotask_t* task, pdata_t that, pdata_t xtra)
{
	if (task->task)
		task->task((void*)task,that,xtra); /* ignore return value */
	return task->task;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_TASK_C__ */
/*----------------------------------------------------------------------------*/
