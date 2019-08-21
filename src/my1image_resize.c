/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_t buff, save;
	char *pname, *psave;
	/* check program arguments */
	if(argc<2)
	{
		printf("No filename given for image resize! Aborting!\n\n");
		return -1;
	}
	pname = argv[1];
	/* initialize */
	image_init(&buff);
	image_init(&save);
	/* load image */
	if(image_load(&buff,pname)<0)
	{
		printf("Cannot load image file '%s'! Aborting!\n\n",pname);
		return -1;
	}
	/* file info */
	printf("File: %s\n",pname);
	printf("Size: %d x %d {Mask:%08X}\n",buff.width,buff.height,buff.mask);
	psave = (char*) malloc(strlen(pname)*2);
	/* double size and save */
	image_size_size(&buff,&save,buff.height*2,buff.width*2);
	sprintf(psave,"%s_double.bmp",pname);
	if (image_save(&save,psave)<0)
		printf("\nCannot save double file '%s'!\n\n",psave);
	else printf("File '%s' saved.\n",psave);
	/* half size and save */
	image_size_size(&buff,&save,buff.height/2,buff.width/2);
	sprintf(psave,"%s_half.bmp",pname);
	if (image_save(&save,psave)<0)
		printf("\nCannot save half file '%s'!\n\n",psave);
	else printf("File '%s' saved.\n",psave);
	/* half width, double height */
	image_size_size(&buff,&save,buff.height*2,buff.width/2);
	sprintf(psave,"%s_sized1.bmp",pname);
	if (image_save(&save,psave)<0)
		printf("\nCannot save sized file '%s'!\n\n",psave);
	else printf("File '%s' saved.\n",psave);
	/* double width, half height */
	image_size_size(&buff,&save,buff.height/2,buff.width*2);
	sprintf(psave,"%s_sized2.bmp",pname);
	if (image_save(&save,psave)<0)
		printf("\nCannot save sized file '%s'!\n\n",psave);
	else printf("File '%s' saved.\n",psave);
	/* quad size */
	image_size_size(&buff,&save,buff.height<<2,buff.width<<2);
	sprintf(psave,"%s_quad.bmp",pname);
	if (image_save(&save,psave)<0)
		printf("\nCannot save sized file '%s'!\n\n",psave);
	else printf("File '%s' saved.\n",psave);
	/* cleanup */
	free((void*)psave);
	image_free(&buff);
	image_free(&save);
	return 0;
}
/*----------------------------------------------------------------------------*/
