/*----------------------------------------------------------------------------*/
#include "my1imgpnm.h"
#include "my1imgutil.h"
#include <stdio.h>
/*----------------------------------------------------------------------------*/
#define PNM_MAGIC_CHAR 2
#define PNM_MAGIC_SIZE (PNM_MAGIC_CHAR+1)
/*----------------------------------------------------------------------------*/
int loadPNMimage(char *filename, my1Image *image)
{
	FILE *pnmfile;
	char buffer[PNM_MAGIC_SIZE];
	int r, g, b, buff, loop;
	int width, height, levels = 1, error = 0, version = 0;
	/* open file for read */
	pnmfile = fopen(filename, "rt");
	if(!pnmfile) return PNM_ERROR_FILEOPEN; /* cannot open file */
	/* get and check magic number */
	fgets(buffer, PNM_MAGIC_SIZE, pnmfile);
	if(buffer[0]=='P'&&(buffer[1]>0x30&&buffer[1]<0x37))
	{
		version = (int)buffer[1] - 0x30; /** P1 - P6 */
		if(version>3)
		{
			fclose(pnmfile);
			return PNM_ERROR_NOSUPPORT; /* not supported */
		}
	}
	if(!version)
	{
		fclose(pnmfile);
		return PNM_ERROR_VALIDPNM; /* not a pnm format */
	}
	/** skip the next 2 newlines (magic number line & comment line) */
	while(!feof(pnmfile)) if(fgetc(pnmfile)=='\n') break;
	while(!feof(pnmfile)) if(fgetc(pnmfile)=='\n') break;
	/** get width and height */
	fscanf(pnmfile,"%d %d",&width,&height);
	/** levels for version 2/3 only! */
	if(version>1) fscanf(pnmfile,"%d",&levels);
#ifdef MY1DEBUG
	printf("Version: %d, levels: %d!\n",version,levels);
	printf("Image width: %d, height: %d\n",width,height);
#endif
	/** try to create storage */
	if(createimage(image,height,width)==0x0)
	{
		fclose(pnmfile);
		return PNM_ERROR_MEMALLOC; /* cannot allocate memory */
	}
	/** read in the pixels */
	for(loop=0; loop<image->length; loop++)
	{
		if(version==3)
		{
			if(fscanf(pnmfile,"%d %d %d",&r,&g,&b)==EOF)
			{
				error = PNM_ERROR_FILESIZE;
				break;
			}
			if(r>levels) r = levels;
			if(g>levels) g = levels;
			if(b>levels) b = levels;
			buff = encode_rgb(r,g,b);
		}
		else
		{
			if(fscanf(pnmfile,"%d",&buff)==EOF)
			{
				error = PNM_ERROR_FILESIZE;
				break;
			}
			if(buff>levels) buff = levels;
			/* adjust for binary pixel */
			if(version==1)
			{
				if(buff) buff = WHITE;
			}
		}
		image->data[loop] = buff;
	}
	fclose(pnmfile);
	image->mask = (version==3) ? IMASK_COLOR24 : IMASK_GRAY8;
	return error;
}
/*----------------------------------------------------------------------------*/
int savePNMimage(char *filename, my1Image *image)
{
	FILE *pnmfile;
	char buffer[PNM_MAGIC_SIZE];
	unsigned char r, g, b;
	int loop, buff, version = 2;
	/* check if color image */
	if(image->mask==IMASK_COLOR24) version = 3;
	/* try to open file for write! */
	pnmfile = fopen(filename,"wt");
	if(!pnmfile) return PNM_ERROR_FILEOPEN; /* cannot open file */
	/* write magic number */
	buffer[0] = 'P'; buffer[1] = (char)(0x30+version); buffer[2] = 0x0;
	fprintf(pnmfile,"%s\n",buffer);
	/* write comment? */
	fprintf(pnmfile,"# Written by my1imgpro library\n");
	/* write size */
	fprintf(pnmfile,"%d %d\n",image->width,image->height);
	/* write level - ALWAYS 8-bit {gray,color}levels */
	fprintf(pnmfile,"255\n");
#ifdef MY1DEBUG
	printf("Version: %d, ",version);
	printf("Image width: %d, height: %d\n",image->width,image->height);
#endif
	/* write data! */
	for(loop=0;loop<image->length;loop++)
	{
		buff = image->data[loop];
		if(image->mask==IMASK_COLOR24)
		{
			decode_rgb(buff,&r,&g,&b);
			fprintf(pnmfile,"%d %d %d\n",r,g,b);
		}
		else
		{
			fprintf(pnmfile,"%d\n",buff);
		}
	}
	fclose(pnmfile);
	return 0;
}
/*----------------------------------------------------------------------------*/
