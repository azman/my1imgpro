/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image.h"
/*----------------------------------------------------------------------------*/
#define ERROR_FLAG 0x80
#define IMAGE_NONE 0
#define IMAGE_FILE 1
#define VIDEO_FILE 2
#define VIDEO_LIVE 3
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_t buff;
	my1image_appw_t iwin;
	int loop, mode = IMAGE_NONE;
	char* pick = 0x0;
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (argv[loop][0]!='-')
		{
			mode = IMAGE_FILE;
			pick = argv[loop];
		}
		else if (!strncmp(argv[loop],"--live",6))
		{
			mode = VIDEO_LIVE;
			pick = argv[++loop];
		}
		else if (!strncmp(argv[loop],"--video",7))
		{
			mode = VIDEO_FILE;
			pick = argv[++loop];
		}
		else
		{
			printf("Invalid argument! (%s)\n",argv[loop]);
			mode |= ERROR_FLAG;
		}
	}
	if (mode&ERROR_FLAG) return -1;
	/* initialize image */
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
	image_appw_show(&iwin,&buff,0x0);
	/* check for done flag */
	image_appw_task(&iwin,image_appw_is_done,ISDONE_TIMEOUT);
	/* main loop */
	gtk_main();
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
