/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include "my1image_bmp.h"
#include "my1image_pnm.h"
#include "my1image_util.h"
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1image_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "Basic Image Tool Library"
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
int load_image(my1image_t* image, char *pfilename)
{
	char filename[80];
	int bmp = 0, pnm = 0;
	/** request filename if not given */
	if(!pfilename)
	{
		pfilename = filename;
		printf("Enter image filename: ");
		scanf("%s",filename);
	}
	/** try to open multiple type - maybe check extension? */
	do
	{
		if(!(bmp=image_load_bmp(pfilename,image))) break;
		if(!(pnm=image_load_pnm(pfilename,image))) break;
		printf("Cannot load input file '%s'! [%d][%d]\n",
			pfilename,bmp,pnm);
	}
	while(0);
	return pnm;
}
/*----------------------------------------------------------------------------*/
int save_image(my1image_t* image, char *pfilename)
{
	char filename[80];
	int bmp = 0, pnm = 0, size;
	/** request filename if not given */
	if(!pfilename)
	{
		pfilename = filename;
		printf("Enter image filename: ");
		scanf("%s",filename);
	}
	/** check extension for format? */
	{
		char *ptest;
		size = strlen(pfilename);
		ptest = &pfilename[size-4];
		if(strcmp(ptest,".bmp")==0)
		{
			if((bmp=image_save_bmp(pfilename,image))<0)
				printf("Cannot write BMP file '%s'! [%d]\n", pfilename, bmp);
		}
		else /* default is pnm! */
		{
			if((pnm=image_save_pnm(pfilename,image))<0)
				printf("Cannot write PNM file '%s'! [%d]\n", pfilename, pnm);
		}
		if(!bmp&&!pnm) printf("Image written to '%s'.\n", pfilename);
	}
	return pnm;
}
/*----------------------------------------------------------------------------*/
int cdata_image(my1image_t* image, char *pfilename)
{
	char filename[80];
	int loop;
	/** request filename if not given */
	if(!pfilename)
	{
		pfilename = filename;
		printf("Enter C data filename: ");
		scanf("%s",filename);
	}
	/** write it? */
	{
		FILE *cfile = fopen(pfilename,"wt");
		if(!cfile) return -1; /* cannot open file */
		fprintf(cfile,"unsigned char image[%d] = {",image->length);
		/* write data! */
		for(loop=0;loop<image->length;loop++)
		{
			if(loop%16==0)
				fprintf(cfile,"\n");
			fprintf(cfile,"0x%02X",image->data[loop]);
			if(loop<image->length-1)
				fprintf(cfile,",");
		}
		fprintf(cfile,"};");
		fclose(cfile);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
void about(void)
{
	printf("Command-line use:\n");
	printf("  %s [options] <image-file> [filter(s)]\n\n",MY1APP_PROGNAME);
	printf("Options are:\n");
	printf("  --save <filename>  : save to image file\n");
	printf("  --cdata <filename> : save to C source file\n");
	printf("  --gray  : force grayscale format\n");
	printf("  --view  : show image (with user interface options)\n");
	printf("  --help  : show this message - overrides ALL above options\n\n");
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, error = 0;
	int gray = 0, view = 0, help = 0, next = 1, temp;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1image_t currimage, nextimage, *image;
	SDL_Surface *screen;
	SDL_Surface *buffer;
	SDL_Event event;
	my1image_filter_t ifilter_laplace1,ifilter_laplace2,
		ifilter_sobelx, ifilter_sobely, ifilter_sobel, ifilter_gauss;
	my1image_filter_t *pfilter = 0x0;

	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");

	/* initialize filters */
	filter_init(&ifilter_laplace1, image_laplace);
	filter_init(&ifilter_laplace2, frame_laplace);
	filter_init(&ifilter_sobelx, image_sobel_x);
	filter_init(&ifilter_sobely, image_sobel_y);
	filter_init(&ifilter_sobel, image_sobel);
	filter_init(&ifilter_gauss, image_gauss);

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
						printf("Cannot get C data file name - NOT writing!\n");
				}
				else if(!strcmp(argv[loop],"--gray"))
				{
					gray = 1;
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
				/* then check for command! */
				if(!strcmp(argv[loop],"laplace1"))
				{
					pfilter = filter_insert(pfilter,&ifilter_laplace1);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobelx"))
				{
					pfilter = filter_insert(pfilter,&ifilter_sobelx);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobely"))
				{
					pfilter = filter_insert(pfilter,&ifilter_sobely);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobel"))
				{
					pfilter = filter_insert(pfilter,&ifilter_sobel);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"laplace2"))
				{
					pfilter = filter_insert(pfilter,&ifilter_laplace2);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"gauss"))
				{
					pfilter = filter_insert(pfilter,&ifilter_gauss);
					gray = 1;
				}
				else
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
		about();
		return 0;
	}

	/** check input filename */
	if(!pname)
	{
		printf("No filename given! Aborting!\n");
		return ERROR_NOFILE;
	}

	/* initialize image */
	image_init(&currimage);
	image_init(&nextimage);

	/* try to open file */
	if((error|=load_image(&currimage,pname))<0)
		return error;
	image_make(&nextimage,currimage.height,currimage.width);
	image_copy(&nextimage,&currimage);
	image = &nextimage;

	/* display basic info */
	printf("Input image: %s\n",pname);
	print_image_info(image);

	/* convert grayscale if requested/required */
	if(gray) image_grayscale(image);

	/* run image filter */
	if (pfilter)
	{
		image = image_filter(image,pfilter,0x0);
		image_absolute(image);
		image_cliphi(image,WHITE);
	}

	/* display processed info */
	printf("Check image:\n");
	print_image_info(image);

	/** save results if requested */
	if(psave)
	{
		printf("Saving image data to %s...\n",psave);
		error |= save_image(image,psave);
		view = 0;
	}

	/** save c-formatted data if requested */
	if(pdata)
	{
		printf("Saving C data to %s...\n",pdata);
		error |= cdata_image(image,pdata);
	}

	/* we are done if no request for ui? */
	if (!view)
	{
		putchar('\n');
		return error;
	}

	/* user interface section? */
	if(SDL_Init(SDL_INIT_VIDEO)!=0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		exit(ERROR_GENERAL);
	}

	printf("\n-----------------------------\n");
	printf("Image Processing Test Utility\n");
	printf("-----------------------------\n\n");
	printf("# <O>riginal Image\n");
	printf("# <G>rayscale Image\n");
	printf("# <I>nvert Image\n");
	printf("# Laplace<1> Image\n");
	printf("# Laplace<2> Image\n");
	printf("# Sobel<X> Image\n");
	printf("# Sobel<Y> Image\n");
	printf("# <S>obel Image\n");
	printf("# G<a>uss Image\n");
	printf("# Fill Image All <B>lack\n");
	printf("# Fill Image All <W>hite\n");
	printf("# <Q>uit\n");

	while(view)
	{
		if (next)
		{
			/* setup main surface */
			screen = SDL_SetVideoMode(image->width,
				image->height,32,SDL_ANYFORMAT);
			if(!screen)
			{
				printf("Unable to set video mode: %s\n", SDL_GetError());
				return ERROR_GENERAL;
			}
			/* if in grayscale, convert to colormode grayscale? */
			if (image->mask!=IMASK_COLOR24) image_colormode(image);
			/* create the temp surface from the raw RGB data */
			buffer = SDL_CreateRGBSurfaceFrom(image->data,image->width,
				image->height,32,image->width*4,0,0,0,0);
			if(!buffer)
			{
				printf("Unable to load image to SDL: %s\n", SDL_GetError());
				return ERROR_GENERAL;
			}
			/* copy to main surface */
			SDL_BlitSurface(buffer,0x0,screen,0x0);
			SDL_Flip(screen);
			SDL_FreeSurface(buffer);
			next = 0;
		}
		if(SDL_PollEvent(&event))
		{
			if(event.type==SDL_QUIT)
			{
				view = 0;
			}
			else if(event.type==SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_q)
				{
					view = 0;
				}
				else if(event.key.keysym.sym == SDLK_o)
				{
					image_copy(&nextimage,&currimage);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_g)
				{
					image_grayscale(image);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_i)
				{
					if (image->mask)
					{
						cbyte r, g, b;
						for(loop=0;loop<image->length;loop++)
						{
							decode_rgb(image->data[loop],&r,&g,&b);
							r = WHITE - r; g = WHITE - g; b = WHITE - b;
							image->data[loop] = encode_rgb(r,g,b);
						}
					}
					else image_invert(image);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_1)
				{
					image_grayscale(image); /* need this! */
					ifilter_laplace1.next = 0x0;
					image_filter(image,&ifilter_laplace1,0x0);
					image_copy(image,&ifilter_laplace1.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_2)
				{
					image_grayscale(image); /* need this! */
					ifilter_laplace2.next = 0x0;
					image_filter(image,&ifilter_laplace2,0x0);
					image_copy(image,&ifilter_laplace2.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_x)
				{
					image_grayscale(image); /* need this! */
					ifilter_sobelx.next = 0x0;
					image_filter(image,&ifilter_sobelx,0x0);
					image_copy(image,&ifilter_sobelx.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_y)
				{
					image_grayscale(image); /* need this! */
					ifilter_sobely.next = 0x0;
					image_filter(image,&ifilter_sobely,0x0);
					image_copy(image,&ifilter_sobelx.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_s)
				{
					image_grayscale(image); /* need this! */
					ifilter_sobel.next = 0x0;
					image_filter(image,&ifilter_sobel,0x0);
					image_copy(image,&ifilter_sobel.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_a)
				{
					image_grayscale(image); /* need this! */
					ifilter_gauss.next = 0x0;
					image_filter(image,&ifilter_gauss,0x0);
					image_copy(image,&ifilter_gauss.buffer);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_b)
				{
					temp = BLACK;
					if (image->mask)
						temp = encode_rgb(temp,temp,temp);
					image_fill(image,temp);
					next = 1;
				}
				else if(event.key.keysym.sym == SDLK_w)
				{
					temp = WHITE;
					if (image->mask)
						temp = encode_rgb(temp,temp,temp);
					image_fill(image,temp);
					next = 1;
				}
			}
		}
	}
	putchar('\n');
	/* cleanup */
	SDL_Quit();
	image_free(&currimage);
	image_free(&nextimage);
	filter_free(&ifilter_laplace1);
	filter_free(&ifilter_laplace2);
	filter_free(&ifilter_sobelx);
	filter_free(&ifilter_sobely);
	filter_free(&ifilter_sobel);
	filter_free(&ifilter_gauss);
	return error;
}
/*----------------------------------------------------------------------------*/
