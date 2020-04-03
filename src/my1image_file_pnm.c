/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_FILE_PNMC__
#define __MY1IMAGE_FILE_PNMC__
/*----------------------------------------------------------------------------*/
#include "my1image_file_pnm.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
#define PNM_MAGIC_CHAR 2
#define PNM_MAGIC_SIZE (PNM_MAGIC_CHAR+1)
/*----------------------------------------------------------------------------*/
int image_load_pnm(my1image_t *image, FILE *pnmfile)
{
	char buffer[PNM_MAGIC_SIZE];
	int r, g, b, buff, loop, size;
	int width, height, version, maxval = 1;
	/* get and check magic number - will read PNM_MAGIC_SIZE-1 char */
	fgets(buffer, PNM_MAGIC_SIZE, pnmfile);
	if (buffer[0]=='P'&&(buffer[1]>0x30&&buffer[1]<0x37))
	{
		version = (int)buffer[1] - 0x30; /** P1 - P6 */
		if (version>3)
			return PNM_ERROR_NOSUPPORT; /* not supported */
	}
	else return FILE_NOT_FORMAT; /* not a pnm format */
	/** get to end of magic number line */
	while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
	if (feof(pnmfile)) return PNM_ERROR_CORRUPT;
	/** skip any comment lines */
	do {
		buff = fgetc(pnmfile);
		if (buff=='#') while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
		else ungetc(buff,pnmfile);
	} while (buff=='#');
	/** get width and height */
	fscanf(pnmfile,"%d %d",&width,&height);
	/** maxval for version 2/3 only! */
	if (version>1) fscanf(pnmfile,"%d",&maxval);
#ifdef MY1DEBUG
	printf("Image format: P%d, maxval: %d\n",version,maxval);
	printf("Image width: %d, height: %d\n",width,height);
	fflush(stdout);
#endif
	/** try to create storage */
	if (image_make(image,height,width)==0x0)
		return PNM_ERROR_MEMALLOC; /* cannot allocate memory */
	/** skip any comment lines */
	do {
		buff = fgetc(pnmfile);
		if (buff=='#') while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
		else ungetc(buff,pnmfile);
	} while (buff=='#');
	/** read in the pixels - depending on version (validated as 1-3)! */
	size = image->size;
	switch (version)
	{
		case 3:
		{
			for (loop=0;loop<size;loop++)
			{
				if (fscanf(pnmfile,"%d %d %d",&r,&g,&b)==EOF)
					return PNM_ERROR_FILESIZE;
				if ((r<0||r>maxval)||(g<0||g>maxval)||(b<0||b>maxval))
					return PNM_ERROR_LEVELPNM;
				buff = encode_rgb(r,g,b);
				image->data[loop] = buff;
			}
			break;
		}
		case 2:
		{
			for (loop=0;loop<size;loop++)
			{
				if (fscanf(pnmfile,"%d",&buff)==EOF)
					return PNM_ERROR_FILESIZE;
				if (buff<0||buff>maxval)
					return PNM_ERROR_LEVELPNM;
				image->data[loop] = buff;
			}
			break;
		}
		case 1:
		{
			for (loop=0;loop<size;loop++)
			{
				if (feof(pnmfile))
					return PNM_ERROR_FILESIZE;
				buff = fgetc(pnmfile);
				switch (buff)
				{
					/* PBM is 'ink on' coded => 1 is BLACK! */
					case '1': buff = BLACK; break;
					case '0': buff = WHITE; break;
					default: buff = -1; break;
					/* ignore whitespaces */
					case ' ': case '\t':
					case '\r': case '\n':
						continue;
				}
				if (buff<0)
					return PNM_ERROR_LEVELPNM;
				image->data[loop] = buff;
			}
			break;
		}
	}
	image->mask = (version==3) ? IMASK_COLOR : IMASK_GRAY;
	return FILE_OK;
}
/*----------------------------------------------------------------------------*/
int image_save_pnm(my1image_t *image, FILE* pnmfile)
{
	char buffer[PNM_MAGIC_SIZE];
	unsigned char r, g, b;
	int loop, next, buff, version = 2;
	/* check if color image */
	if (image->mask==IMASK_COLOR) version = 3;
	/* write magic number */
	buffer[0] = 'P'; buffer[1] = (char)(0x30+version); buffer[2] = 0x0;
	fprintf(pnmfile,"%s\n",buffer);
	/* write comment? */
	fprintf(pnmfile,"# Written by my1imgpro library\n");
	/* write size */
	fprintf(pnmfile,"%d %d\n",image->cols,image->rows);
	/* write max value - ALWAYS 8-bit {gray,color} max value */
	fprintf(pnmfile,"255\n");
#ifdef MY1DEBUG
	printf("Version: %d, ",version);
	printf("Image width: %d, height: %d\n",image->cols,image->rows);
#endif
	/* write data! */
	for (loop=0,next=0;loop<image->size;loop++)
	{
		buff = image->data[loop];
		if (image->mask==IMASK_COLOR)
		{
			decode_rgb(buff,&r,&g,&b);
			fprintf(pnmfile,"%d %d %d ",r,g,b);
		}
		else
		{
			fprintf(pnmfile,"%d ",buff);
		}
		/* newline for every row */
		if (++next==image->cols)
		{
			next = 0;
			fprintf(pnmfile,"\n");
		}
	}
	return FILE_OK;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_FILE_PNMC__ */
/*----------------------------------------------------------------------------*/
