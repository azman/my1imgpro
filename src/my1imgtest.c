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
my1ImgFilter* insert_imgfilter(my1ImgFilter* pstack, my1ImgFilter* pcheck)
{
	my1ImgFilter *pthis = pstack;
	if (!pstack) return pcheck;
	while(pthis->next) pthis = pthis->next;
	pthis->next = pcheck;
	return pstack;
}
/*----------------------------------------------------------------------------*/
my1Image* apply_mask2image(my1Image* image, my1Image* result,
	int size, int* pdata)
{
	my1Mask mask;
	if(!createmask(&mask,size)) return image;
	setmask(&mask,pdata);
	if(!result->data) createimage(result,image->height,image->width);
	mask_image(&mask,image,result);
	freemask(&mask);
	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* laplace_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { 0,-1,0, -1,4,-1, 0,-1,0 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_x_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { -1,0,1, -2,0,2, -1,0,1 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_y_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { -1,-2,-1, 0,0,0, 1,2,1 };
	return apply_mask2image(image,result,3,coeff);
}
/*----------------------------------------------------------------------------*/
int iabs(int value)
{
	if(value<0) value = -value;
	return value;
}
/*----------------------------------------------------------------------------*/
my1Image* sobel_image(my1Image* image, my1Image* result, void* userdata)
{
	my1Image buff1, buff2, *pphase = (my1Image*) userdata;
	int irow, icol, x, y, temp;

	/* initialize buffer stuctures */
	initimage(&buff1);
	initimage(&buff2);
	/* create temporary buffers */
	if(!createimage(&buff1,image->height,image->width)||
		!createimage(&buff2,image->height,image->width))
	{
		freeimage(&buff1);
		freeimage(&buff2);
		return image;
	}

	/* calculate directional edge */
	sobel_x_image(image, &buff1, 0x0);
	sobel_y_image(image, &buff2, 0x0);

	/* prepare resulting image structure */
	if(!result->data) createimage(result,image->height,image->width);
	if(pphase&&!pphase->data) createimage(pphase,image->height,image->width);

	/* calculate magniture & phase for 3x3 neighbourhood */
	for(irow=0;irow<image->height;irow++)
	{
		for(icol=0;icol<image->width;icol++)
		{
			x = imagepixel(&buff1,irow,icol);
			y = imagepixel(&buff2,irow,icol);
			setimagepixel(result,irow,icol,iabs(y)+iabs(x));
			if(!pphase) continue;
			if(x>0)
			{
				if(y>0)
				{
					/* q1 */
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 90;
					else if(x>2*y) temp = 0;
					else temp = 45;
				}
				else if(y<0)
				{
					/* q4 */
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 270;
					else if(x>2*y) temp = 0;
					else temp = 315;
				}
				else
				{
					temp = 0; /* +ve x-axis */
				}
			}
			else if(x<0)
			{
				if(y>0)
				{
					/* q2 */
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 90;
					else if(x>2*y) temp = 180;
					else temp = 135;
				}
				else if(y<0)
				{
					/* q3 */
					y = iabs(y); x = iabs(x);
					if(y>2*x) temp = 270;
					else if(x>2*y) temp = 180;
					else temp = 225;
				}
				else
				{
					temp = 180; /* -ve x-axis */
				}
			}
			else
			{
				if(y>0)
				{
					temp = 90; /* +ve y-axis */
				}
				else if(y<0)
				{
					temp = 270; /* -ve y-axis */
				}
				else
				{
					temp = 0; /* origin! no edge? */
				}
			}
			setimagepixel(pphase,irow,icol,temp);
		}
	}

	/* clean-up */
	freeimage(&buff1);
	freeimage(&buff2);

	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* laplace_frame(my1Image* image, my1Image* result, void* userdata)
{
	my1IFrame buff1, buff2;
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1Kernel kernel;
	if(createkernel(&kernel,3))
	{
		setkernel(&kernel,coeff);
		initframe(&buff1);
		initframe(&buff2);
		if(createframe(&buff1,image->height,image->width)&&
			createframe(&buff2,image->height,image->width))
		{
			image2frame(image,&buff1,0);
			correlate_frame(&buff1,&buff2,&kernel);
			if(!result->data)
				createimage(result,image->height,image->width);
			frame2image(&buff2,result,1);
		}
		freeframe(&buff1);
		freeframe(&buff2);
		freekernel(&kernel);
	}
	return result;
}
/*----------------------------------------------------------------------------*/
my1Image* gaussian_image(my1Image* image, my1Image* result, void* userdata)
{
	int coeff[] = { 1,4,7,4,1, 4,16,26,16,4,
		7,26,41,26,7, 4,16,26,16,4, 1,4,7,4,1};
	apply_mask2image(image,result,5,coeff);
	/* divide 273??? */
	scale_pixel(image,(1.0/273.0));
	return result;
}
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

	/* copy image pixels to buffer */
	pImage = malloc(image->height*image->width*3);
	extract_rgb(image,pImage); /* should never be a problem! */

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
	printf("  %s [options] <image-file> [filter(s)]\n\n",MY1APP_PROGNAME);
	printf("Valid (stackable) image filter(s) are:\n");
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
	int loop, error = 0;
	int gray = 0, view = 1, help = 0;
	char *psave = 0x0, *pname = 0x0, *pdata = 0x0;
	my1Image currimage, *pimage;
	my1ImgFilter ifilter_laplace1,ifilter_laplace2,
		ifilter_sobelx, ifilter_sobely, ifilter_sobel, ifilter_gauss;
	my1ImgFilter *pfilter = 0x0;

	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.net\n\n");

	/* initialize filters */
	filter_init(&ifilter_laplace1, laplace_image);
	filter_init(&ifilter_laplace2, laplace_frame);
	filter_init(&ifilter_sobelx, sobel_x_image);
	filter_init(&ifilter_sobely, sobel_y_image);
	filter_init(&ifilter_sobel, sobel_image);
	filter_init(&ifilter_gauss, gaussian_image);

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
					pfilter = insert_imgfilter(pfilter,&ifilter_laplace1);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobelx"))
				{
					pfilter = insert_imgfilter(pfilter,&ifilter_sobelx);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobely"))
				{
					pfilter = insert_imgfilter(pfilter,&ifilter_sobely);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"sobelall"))
				{
					pfilter = insert_imgfilter(pfilter,&ifilter_sobel);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"laplace2"))
				{
					pfilter = insert_imgfilter(pfilter,&ifilter_laplace2);
					gray = 1;
				}
				else if(!strcmp(argv[loop],"gauss"))
				{
					pfilter = insert_imgfilter(pfilter,&ifilter_gauss);
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
		return ERROR_GENERAL;
	}

	/* initialize image & frame*/
	initimage(&currimage);

	/* try to open file */
	if((error=load_image(&currimage,pname))<0)
		return error;
	pimage = &currimage;

	/* display basic info */
	printf("Input image: %s\n",pname);
	print_image_info(pimage);

	/* convert grayscale if requested/required */
	if(gray) grayscale_image(pimage);

	/* run image filter */
	if (pfilter) pimage = filter_image(pfilter,pimage);

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
	freeimage(&currimage);
	filter_free(&ifilter_laplace1);
	filter_free(&ifilter_laplace2);
	filter_free(&ifilter_sobelx);
	filter_free(&ifilter_sobely);
	filter_free(&ifilter_sobel);
	filter_free(&ifilter_gauss);

	return error;
}
/*----------------------------------------------------------------------------*/
