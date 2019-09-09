/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_data.h"
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1image_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "MY1Image Test Program"
#endif
/*----------------------------------------------------------------------------*/
#define ERROR_GENERAL 0
#define ERROR_NOFILE -1
/*----------------------------------------------------------------------------*/
void print_image_info(my1image_t* image)
{
	printf("Size: %d x %d, ",image->width,image->height);
	printf("Mask: %08X\n",image->mask);
}
/*----------------------------------------------------------------------------*/
void about(my1image_data_t* data)
{
	int size = 0;
	my1image_filter_t* ppass = data->pflist;
	printf("Command-line use:\n");
	printf("  %s [options] <image-file> [filter(s)]\n\n",MY1APP_PROGNAME);
	printf("Options are:\n");
	printf("  --save <filename>  : save to image file\n");
	printf("  --cdata <filename> : save to C source file\n");
	printf("  --view  : show image (with user interface options)\n");
	printf("  --help  : show this message - overrides ALL above options\n");
	printf("Filters available:");
	while (ppass)
	{
		if (size%8==0) printf("\n  ");
		printf("%s",ppass->name);
		ppass = ppass->next;
		if (ppass) printf(",");
		size++;
	}
	printf(" [%d]\n\n",size);
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, error = 0;
	int view = 0, help = 0;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1image_data_t q;
	my1image_filter_t* ppass;
	/* print tool info */
	printf("\n%s - %s (%s)\n",
		MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* initialize image_test */
	image_data_init(&q);
	/* check program arguments */
	if(argc>1)
	{
		for(loop=1;loop<argc;loop++)
		{
			if(argv[loop][0]=='-') /* options! */
			{
				if(!strcmp(argv[loop],"--save"))
				{
					loop++;
					if(loop<argc)
						psave = argv[loop];
					else
						printf("Cannot get save file name - NOT saving!\n");
				}
				else if(!strcmp(argv[loop],"--cdata"))
				{
					loop++;
					if(loop<argc)
						pdata = argv[loop];
					else
						printf("Cannot get C file name - NOT writing!\n");
				}
				else if(!strcmp(argv[loop],"--view"))
				{
					view = 1;
				}
				else if(!strcmp(argv[loop],"--help"))
				{
					help = 1;
				}
				else
				{
					printf("Unknown option '%s'!\n",argv[loop]);
				}
			}
			else /* not an option? */
			{
				/* first non-option must be file name! */
				if(!pname)
				{
					pname = argv[loop];
					continue;
				}
				/* then check for command (filter name)! */
				ppass = q.pflist;
				while (ppass)
				{
					if(!strcmp(argv[loop],ppass->name))
					{
						image_data_filter_load(&q,ppass->name);
						break;
					}
					ppass = ppass->next;
				}
				if (!ppass)
				{
					printf("Unknown parameter %s!\n",argv[loop]);
					continue;
				}
			}
		}
	}
	/* check if user requested help */
	if(help)
	{
		about(&q);
		return 0;
	}
	/** check input filename */
	if(pname)
	{
		/* try to open file */
		if((error=image_load(&q.currimage,pname))<0)
		{
			switch (error)
			{
				case BMP_ERROR_FILEOPEN:
				case PNM_ERROR_FILEOPEN:
					printf("Error opening file");
					break;
				case BMP_ERROR_VALIDBMP:
				case PNM_ERROR_VALIDPNM:
				case FILE_ERROR_FORMAT:
					printf("Unsupported file format (%d)",error);
					break;
				case BMP_ERROR_RGBNGRAY:
					printf("Unsupported BMP format");
					break;
				case PNM_ERROR_NOSUPPORT:
					printf("Unsupported PNM format");
					break;
				case BMP_ERROR_FILESIZE:
				case PNM_ERROR_FILESIZE:
					printf("Invalid file size");
					break;
				case BMP_ERROR_MEMALLOC:
				case PNM_ERROR_MEMALLOC:
					printf("Unable to allocate memory");
					break;
				case BMP_ERROR_DIBINVAL:
				case BMP_ERROR_COMPRESS:
					printf("Invalid BMP format");
					break;
				default:
					printf("Unknown error opening file (%d)",error);
			}
			printf(": '%s'!\n\n",pname);
			return error;
		}
	}
	else
	{
		if (!view)
		{
			printf("No filename given for image testing! Aborting!\n\n");
			return ERROR_NOFILE;
		}
		/* create blank image at default size */
		image_make(&q.currimage,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&q.currimage,BLACK);
	}
	/* prepare buffer */
	buffer_size(&q.work,q.currimage.height,q.currimage.width);
	image_copy(q.work.curr,&q.currimage);
	q.image = q.work.curr;
	/* only if a file has been loaded */
	if (pname)
	{
		/* display basic info */
		printf("Input image: %s\n",pname);
		print_image_info(q.image);
		/* run image filter, if requested */
		if (q.pfcurr)
			image_data_filter_exec(&q);
		/* display processed info */
		printf("Check image:\n");
		print_image_info(q.image);
		/** save results if requested */
		if(psave)
		{
			printf("Saving image data to %s... ",psave);
			if ((error=image_save(q.image,psave))<0)
			{
				switch (error)
				{
					case BMP_ERROR_FILEOPEN:
					case PNM_ERROR_FILEOPEN:
						printf("write error!\n");
						break;
					default:
						printf("unknown error!\n");
				}
			}
			else printf("done!\n");
		}
		/** save c-formatted data if requested */
		if(pdata)
		{
			printf("Saving C data to %s... ",pdata);
			if ((error=image_cdat(q.image,pdata))<0)
			{
				switch (error)
				{
					case FILE_ERROR_OPEN:
						printf("write error!\n");
						break;
					default:
						printf("unknown error!\n");
				}
			}
			else printf("done!\n");
		}
	}
	/* check request for ui */
	if (view)
	{
		/* initialize gui */
		gtk_init(&argc,&argv);
		/* setup auto-quit on close */
		q.view.goquit = 1;
		/* assign image */
		image_data_make(&q,&q.currimage);
		image_data_filter_exec(&q);
		image_data_draw(&q);
		/* allow histogram */
		image_hist_make(&q.hist);
		/* event handlers */
		image_data_events(&q);
		/* menu stuff */
		image_data_domenu(&q);
		/* main loop */
		gtk_main();
	}
	/* cleanup image_test */
	image_data_free(&q);
	/* done! */
	putchar('\n');
	return error;
}
/*----------------------------------------------------------------------------*/
