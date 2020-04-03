/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#define TASK_ERROR 0x01
#define TASK_NONE 0x00
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_data_t data;
	int loop, task = TASK_NONE;
	char* pick = 0x0;
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (loop==1) pick = argv[loop];
		else if (!strncmp(argv[loop],"--none",7))
			task |= TASK_NONE;
		else
		{
			printf("Invalid argument! (%s)\n",argv[loop]);
			task |= TASK_ERROR;
		}
	}
	if (task&TASK_ERROR) return -1;
	/* initialize image_date */
	image_data_init(&data);
	/* setup auto-quit on close */
	data.appw.goquit = 1;
	/* load if requested */
	if (pick)
	{
		if((loop=image_load(&data.appw.buff,pick))<0)
		{
			printf("Error loading file '%s'!(%x)\n",pick,(unsigned)loop);
			return loop;
		}
	}
	else
	{
		image_make(&data.appw.buff,240,320);
		image_fill(&data.appw.buff,BLACK);
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* assign image */
	image_data_make(&data,&data.appw.buff);
	/* create menu */
	image_appw_domenu(&data.appw);
	/* main loop */
	gtk_main();
	/* cleanup */
	image_data_free(&data);
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
