/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include "my1imgbmp.h"
#include "my1imgpnm.h"
#include "my1imgfpo.h"
#include "my1imgutil.h"
#include "my1imgmath.h"
#include <math.h> /* for gaussian calc! */
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1imgtest"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "Basic Image Tool Library"
#endif
/*----------------------------------------------------------------------------*/
#define ERROR_GENERAL -1
/*----------------------------------------------------------------------------*/
#define COMMAND_NONE 0
#define COMMAND_LAPLACE1 1
#define COMMAND_SOBELX 2
#define COMMAND_SOBELY 3
#define COMMAND_SOBELALL 4
#define COMMAND_LAPLACE2 5
#define COMMAND_GAUSS 6
/*----------------------------------------------------------------------------*/
/* sample filters */
/*----------------------------------------------------------------------------*/
int laplace_image(my1Image *src, my1Image *dst)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	my1Mask mask;

	if(createmask(&mask,3)==0x0)
		return -1;

	setmask(&mask,coeff);
	mask.orig_x = 1;
	mask.orig_y = 1;

	filter_image(src,dst,&mask);
	freemask(&mask);

	return 0;
}
/*----------------------------------------------------------------------------*/
int sobel_x_image(my1Image *src, my1Image *dst)
{
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	my1Mask mask;

	if(createmask(&mask,3)==0x0)
		return -1;

	setmask(&mask,coeff);
	mask.orig_x = 1;
	mask.orig_y = 1;

	filter_image(src,dst,&mask);
	freemask(&mask);

	return 0;
}
/*----------------------------------------------------------------------------*/
int sobel_y_image(my1Image *src, my1Image *dst)
{
	int coeff[] = { -1,-2,-1, 0,0,0, 1,2,1 };
	my1Mask mask;

	if(createmask(&mask,3)==0x0)
		return -1;

	setmask(&mask,coeff);
	mask.orig_x = 1;
	mask.orig_y = 1;

	filter_image(src,dst,&mask);
	freemask(&mask);

	return 0;
}
/*----------------------------------------------------------------------------*/
int iabs(int value)
{
	if(value<0) value = -value;
	return value;
}
/*----------------------------------------------------------------------------*/
int sobel_image(my1Image *src, my1Image *dst_mag, my1Image *dst_ang)
{
	my1Image buff1, buff2;
	int irow, icol, x, y, temp;

	// create temporary buffer
	if(!createimage(&buff1,src->height,src->width))
	{
		//printf("Cannot allocate buff1 memory\n");
		return -1;
	}
	if(!createimage(&buff2,src->height,src->width))
	{
		freeimage(&buff1);
		//printf("Cannot allocate buff2 memory\n");
		return -1;
	}

	// calculate directional edge
	sobel_x_image(src, &buff1);
	sobel_y_image(src, &buff2);

	// calculate angle for 3x3 neighbourhood
	for(irow=0;irow<src->height;irow++)
	{
		for(icol=0;icol<src->width;icol++)
		{
			x = imagepixel(&buff1,irow,icol);
			y = imagepixel(&buff2,irow,icol);
			setimagepixel(dst_mag,irow,icol,iabs(y)+iabs(x));
			if(!dst_ang) continue;
			if(x>0)
			{
				if(y>0)
				{
					// q1
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 90;
					else if(x>2*y) temp = 0;
					else temp = 45;
				}
				else if(y<0)
				{
					// q4
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 270;
					else if(x>2*y) temp = 0;
					else temp = 315;
				}
				else
				{
					temp = 0; // +ve x-axis
				}
			}
			else if(x<0)
			{
				if(y>0)
				{
					// q2
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 90;
					else if(x>2*y) temp = 180;
					else temp = 135;
				}
				else if(y<0)
				{
					// q3
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 270;
					else if(x>2*y) temp = 180;
					else temp = 225;
				}
				else
				{
					temp = 180; // -ve x-axis
				}
			}
			else
			{
				if(y>0)
				{
					temp = 90; // +ve y-axis
				}
				else if(y<0)
				{
					temp = 270; // -ve y-axis
				}
				else
				{
					temp = 0; // origin! no edge?
				}
			}
			setimagepixel(dst_ang,irow,icol,temp);
		}
	}

	/* clean-up */
	freeimage(&buff1);
	freeimage(&buff2);

	return 0;
}
/*----------------------------------------------------------------------------*/
int laplace_frame(my1IFrame *src, my1IFrame *dst)
{
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1Kernel kernel;

	if(createkernel(&kernel,3)==0x0)
		return -1;

	setkernel(&kernel,coeff);
	kernel.orig_x = 1;
	kernel.orig_y = 1;

	correlate_frame(src,dst,&kernel);
	freekernel(&kernel);

	return 0;
}
/*----------------------------------------------------------------------------*/
#define PI 3.14159265
#define THRES2 0.0039 /* 1/256 - max division possible for grayscale */
#define THRES1 0.0156 /* 1/64 - leave 2 significant bits */
#define THRESH 0.0625 /* 1/16 - leave 4 significant bits */
/*----------------------------------------------------------------------------*/
float fgaussian2d(float x, float y, float sigma)
{
	/* isotropic (circularly symmetric) mean at (0,0) */
	return pow(2.71828,-(((x*x)+(y*y))/(2*sigma*sigma)))/(sigma*sigma*2*PI);
}
/*----------------------------------------------------------------------------*/
int gauss_frame(my1IFrame *src, my1IFrame *dst, float sigma, float *over)
{
	my1Kernel kernel;
	/* computing MY 2D gaussian smoothing kernel */
	int loop = 0, loop2, index = 0;
	float temp = 1.0, corner;
	float mult, value;
	while(temp>THRESH)
	{
		temp = fgaussian2d(loop,loop,sigma);
		corner = temp;
		loop++;
	}
	value = 1.0/corner;

	int windowsize = (loop*2)-1;
	int count = windowsize/2;
	int length = windowsize*windowsize;
	float *pcoeff = (float*) malloc(length*sizeof(float));
	if(pcoeff==0x0)
		return -1;
	for(loop=0;loop<windowsize;loop++)
	{
		for(loop2=0;loop2<windowsize;loop2++)
		{
			if(over)
				mult = value * (int)fgaussian2d(loop-count, loop2-count, sigma);
			else
				mult = fgaussian2d(loop-count, loop2-count, sigma);
			pcoeff[index++] = mult;
		}
	}
	if(over) *over = value;

	if(createkernel(&kernel,windowsize)==0x0)
	{
		free(pcoeff);
		return -1;
	}
	setkernel(&kernel, pcoeff);
	kernel.orig_x = count;
	kernel.orig_y = count;
	free(pcoeff);

	correlate_frame(src, dst, &kernel);
	freekernel(&kernel);

	return 0;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void print_image_info(my1Image* image)
{
	printf("Size: %d x %d,",image->width,image->height);
	printf("Mask: %08X\n\n",image->mask);
}
/*----------------------------------------------------------------------------*/
int load_image(my1Image* image, char *pfilename)
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
		if(!(bmp=loadBMPimage(pfilename,image))) break;
		if(!(pnm=loadPNMimage(pfilename,image))) break;
		printf("Cannot load input file '%s'! [%d][%d]\n",
			pfilename,bmp,pnm);
	}
	while(0);
	return pnm;
}
/*----------------------------------------------------------------------------*/
int save_image(my1Image* image, char *pfilename)
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
			if((bmp=saveBMPimage(pfilename,image))<0)
				printf("Cannot write BMP file '%s'! [%d]\n", pfilename, bmp);
		}
		else /* default is pnm! */
		{
			if((pnm=savePNMimage(pfilename,image))<0)
				printf("Cannot write PNM file '%s'! [%d]\n", pfilename, pnm);
		}
		if(!bmp&&!pnm) printf("Image written to '%s'.\n", pfilename);
	}
	return pnm;
}
/*----------------------------------------------------------------------------*/
int cdata_image(my1Image* image, char *pfilename)
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
void view_image(my1Image* image)
{
	SDL_Surface *screen;
	SDL_Surface *temp;
	SDL_Event event;
	unsigned char *pImage;

	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return;
	}

	screen = SDL_SetVideoMode(image->width, image->height, 24, SDL_ANYFORMAT);
	if(!screen)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		return;
	}

	if(!image->mask) image-> mask = IMASK_GRAY8;
	pImage = malloc(image->height*image->width*3);
	if(!extract_rgb(image,pImage))
	{
		free(pImage);
		printf("Unable to extract data from image: %08X\n", image->mask);
		return;
	}

	/* Create the temp surface from the raw RGB data */
	temp = SDL_CreateRGBSurfaceFrom(pImage, image->width, image->height,
		24, image->width*3, 0, 0, 0, 0);
	if(!temp)
	{
		printf("Unable to load image to SDL: %s\n", SDL_GetError());
		return;
	}

	SDL_BlitSurface(temp, NULL, screen, NULL);
	SDL_Flip(screen);
	SDL_FreeSurface(temp);
	free(pImage);

	printf("Showing image... Press 'q' to quit.\n");

	while(1)
	{
		if(SDL_PollEvent(&event))
		{
			if(event.type==SDL_QUIT)
			{
				break;
			}
			else if(event.type==SDL_KEYDOWN)
			{
				if(event.key.keysym.sym == SDLK_q)
				{
					break;
				}
			}
		}
	}

	SDL_Quit();
}
/*----------------------------------------------------------------------------*/
void about(void)
{
	printf("Command-line use:\n");
	printf("  %s [options] <image-file> [command]\n\n",MY1APP_PROGNAME);
	printf("Valid Commands (image filter) are:\n");
	printf("  laplace1  : Laplace Gradient Filter\n");
	printf("  laplace2  : Laplace Gradient Filter (floating-point data)\n");
	printf("  sobelx    : Sobel Horizontal Gradient Filter\n");
	printf("  sobely    : Sobel Vertical Gradient Filter\n");
	printf("  sobelall  : Sobel Directional Gradient Filter\n");
	printf("  gauss     : Gaussian Gradient Filter\n\n");
	printf("Options are:\n");
	printf("  --save <filename>  : save to image file\n");
	printf("  --cdata <filename> : save to C source file\n");
	printf("  --gray    : force grayscale format\n");
	printf("  --hide    : skip image display\n");
	printf("  --help    : show this message - overrides ALL above options\n\n");
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int loop, test = 0, error = 0, command = COMMAND_NONE;
	int gray = 0, view = 1, help = 0;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1Image currimage, tempimage, *pimage;
	my1IFrame currframe, tempframe;

	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.net\n\n");

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
				else if(!strcmp(argv[loop],"--hide"))
				{
					view = 0;
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
					command = COMMAND_LAPLACE1;
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobelx"))
				{
					command = COMMAND_SOBELX;
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobely"))
				{
					command = COMMAND_SOBELY;
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobelall"))
				{
					command = COMMAND_SOBELALL;
					gray = 1;
				}
				else if(!strcmp(argv[loop],"laplace2"))
				{
					command = COMMAND_LAPLACE2;
					gray = 1;
				}
				else if(!strcmp(argv[loop],"gauss"))
				{
					command = COMMAND_GAUSS;
					gray = 1;
				}
				else
				{
					printf("Unknown parameter %s!\n",argv[loop]);
					continue;
				}
				/* warn if overriding previous command! */
				if(command)
				{
					printf("Warning! Command '%s' overrides '%s'!\n",
						argv[loop],argv[test]);
				}
				test = loop;
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
		return ERROR_GENERAL;
	}

	/* initialize image & frame*/
	initimage(&currimage);
	initimage(&tempimage);
	initframe(&currframe);
	initframe(&tempframe);

	/* try to open file */
	if((error=load_image(&currimage,pname))<0)
	{
		return error;
	}

	/* display basic info */
	printf("Input image: %s\n",pname);
	print_image_info(&currimage);

	/* convert grayscale if requested/required */
	if(gray) grayscale_image(&currimage);

	/* process command */
	switch(command)
	{
		case COMMAND_LAPLACE1:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			laplace_image(&currimage,&tempimage);
			break;
		}
		case COMMAND_SOBELX:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			sobel_x_image(&currimage,&tempimage);
			break;
		}
		case COMMAND_SOBELY:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			sobel_y_image(&currimage,&tempimage);
			break;
		}
		case COMMAND_SOBELALL:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			sobel_image(&currimage,&tempimage,0x0);
			break;
		}
		case COMMAND_LAPLACE2:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			createframe(&currframe,currimage.height,currimage.width);
			createframe(&tempframe,currimage.height,currimage.width);
			image2frame(&currimage,&currframe,0);
			laplace_frame(&currframe,&tempframe);
			frame2image(&tempframe,&tempimage,1);
			break;
		}
		case COMMAND_GAUSS:
		{
			createimage(&tempimage,currimage.height,currimage.width);
			createframe(&currframe,currimage.height,currimage.width);
			createframe(&tempframe,currimage.height,currimage.width);
			image2frame(&currimage,&currframe,0);
			gauss_frame(&currframe,&tempframe,1.0,0x0);
			frame2image(&tempframe,&tempimage,1);
			break;
		}
	}

	if(!tempimage.length)
		pimage = &currimage;
	else
		pimage = &tempimage;
	printf("Check image:\n");
	print_image_info(pimage);

	/** save results if requested */
	if(psave)
	{
		printf("Saving image data to %s...\n",psave);
		error=save_image(pimage,psave);
		view = 0;
	}

	if(pdata)
	{
		printf("Saving C data to %s...\n",pdata);
		error=cdata_image(pimage,pdata);
	}

	/* view image if no request to hide! .. and NOT saving! */
	if(view) view_image(pimage);

	/* cleanup */
	freeframe(&currframe);
	freeframe(&tempframe);
	freeimage(&currimage);
	freeimage(&tempimage);

	return error;
}
/*----------------------------------------------------------------------------*/
