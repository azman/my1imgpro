/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h> /** for file access */
#include <string.h> /** image_write needs string functions */
/*----------------------------------------------------------------------------*/
#include "my1image_file_bmp.h"
#include "my1image_file_pnm.h"
/*----------------------------------------------------------------------------*/
my1imgfmt_t ipnm = {PNM_ERROR_FLAG,"pnm",image_load_pnm,image_save_pnm,0x0};
my1imgfmt_t ibmp = {BMP_ERROR_FLAG,"bmp",image_load_bmp,image_save_bmp,&ipnm};
/*----------------------------------------------------------------------------*/
static my1image_format_t *init = &ibmp, *last = &ipnm;
/*----------------------------------------------------------------------------*/
void image_format_insert(my1image_format_t* imgfmt)
{
	last->next = imgfmt;
	last = imgfmt;
}
/*----------------------------------------------------------------------------*/
int image_load(my1image_t* image, char *pfilename)
{
	FILE* pfile;
	int flag;
	my1image_format_t *that = init;
	/* open file for read */
	pfile = fopen(pfilename, "rb");
	if (!pfile) return FILE_ERROR_OPEN; /* cannot open file */
	while (that)
	{
		/* do_load should always be available! */
		flag = that->do_load(image,pfile);
		if (flag==FILE_OK) break;
		/* if error other than non-format, we are done! */
		if (!(flag&FILE_NOT_FORMAT)) break;
		/* reset file pointer and try next format */
		fseek(pfile,0,SEEK_SET);
		that = that->next;
	}
	fclose(pfile);
	return flag;
}
/*----------------------------------------------------------------------------*/
int image_save(my1image_t* image, char *pfilename)
{
	FILE* pfile;
	int flag, size;
	char *ptest;
	my1image_format_t *that = init, *find = init;
	/* open file for read */
	pfile = fopen(pfilename, "wb");
	if (!pfile) return FILE_ERROR_OPEN; /* cannot open file */
	/* get file extension */
	size = strlen(pfilename);
	ptest = &pfilename[size-4];
	/* compare requested file extension */
	while (that)
	{
		/* make sure do_save exists && compare type */
		if (that->do_save&&ptest[0]=='.'&&!strcmp(&ptest[1],that->extd))
		{
			find = that;
			break;
		}
		that = that->next;
	}
	flag = find->do_save(image,pfile);
	fclose(pfile);
	return flag;
}
/*----------------------------------------------------------------------------*/
int image_cdat(my1image_t* image, char *pfilename)
{
	int loop;
	FILE *cfile = fopen(pfilename,"wt");
	if(!cfile) return FILE_ERROR_OPEN; /* cannot open file */
	fprintf(cfile,"unsigned char image[%d] = {",image->length);
	/* write data! */
	for (loop=0;loop<image->length;loop++)
	{
		if (loop%16==0)
			fprintf(cfile,"\n");
		fprintf(cfile,"0x%02X",image->data[loop]);
		if (loop<image->length-1)
			fprintf(cfile,",");
	}
	fprintf(cfile,"};");
	fclose(cfile);
	return 0;
}
/*----------------------------------------------------------------------------*/
