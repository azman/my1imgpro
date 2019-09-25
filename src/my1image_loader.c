/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_t buff;
	char *pname;
	int test;
	/* check program arguments */
	if(argc<2)
	{
		printf("No filename given for image loading! Aborting!\n");
		return -1;
	}
	pname = argv[1];
	/* initialize */
	image_init(&buff);
	/* load image */
	test = image_load(&buff,pname);
	if(test<0)
	{
		printf("Cannot load image file '%s'! [%d] Aborting!\n",pname,test);
	}
	else
	{
		/* file info */
		printf("File: %s\n",pname);
		printf("Size: %d x %d (Size:%d) {Mask:%08X}\n",
			buff.width,buff.height,buff.length,buff.mask);
	}
	image_free(&buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
