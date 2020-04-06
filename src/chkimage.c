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
	my1image_t buff;
	my1image_appw_t data;
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
	image_init(&buff);
	/* load if requested */
	if (pick)
	{
		if((loop=image_load(&buff,pick))<0)
		{
			printf("Error loading file '%s'!(%x)\n",pick,(unsigned)loop);
			return loop;
		}
	}
	else
	{
		image_make(&buff,240,320);
		image_fill(&buff,BLACK);
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* show image */
	image_appw_show(&data,&buff,0x0);
	data.goquit = 1; /* single window, quit once done! */
	/* main loop */
	gtk_main();
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
