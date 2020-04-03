/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
#include "my1image_file_png.h"
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int test, loop;
	my1image_t buff;
	char *pname = 0x0, *psave = 0x0, *pdata = 0x0;
	for(loop=1;loop<argc;loop++)
	{
		if(!strcmp(argv[loop],"--save"))
		{
			loop++;
			if(loop<argc) psave = argv[loop];
			else printf("Cannot get save file name - NOT saving!\n");
		}
		else if(!strcmp(argv[loop],"--cdata"))
		{
			loop++;
			if(loop<argc) pdata = argv[loop];
			else printf("Cannot get C file name - NOT writing!\n");
		}
		else
		{
			if(!pname) pname = argv[loop];
			else printf("Unknown option '%s'!\n",argv[loop]);
		}
	}
	if (!pname) return -1;
	/* add png support */
	image_format_insert(&ipng);
	image_init(&buff);
	printf("Loading image file '%s'... ",pname);
	if ((test=image_load(&buff,pname))<0) printf("error![%x]\n",(unsigned)test);
	else
	{
		printf("done![%x]\n",(unsigned)test);
		printf("-- File: %s\n",pname);
		printf("-- Size: %d x %d (Size:%d) {Mask:%08X}\n",
			buff.cols,buff.rows,buff.size,buff.mask);
		if(psave)
		{
			printf("Saving image data to %s... ",psave);
			if ((test=image_save(&buff,psave))<0) printf("error![%d]\n",test);
			else printf("done!\n");
		}
		if(pdata)
		{
			printf("Saving C data to %s... ",pdata);
			if ((test=image_cdat(&buff,pdata))<0) printf("error![%d]\n",test);
			else printf("done!\n");
		}
	}
	image_free(&buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
