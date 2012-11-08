#include <stdio.h>
#include <SDL/SDL.h>
#include "my1imgbmp.h"
#include "my1imgfpo.h"
#include "my1imgutil.h"
#include "my1imgmath.h"

#include <math.h> /* for gaussian calc! */

#define PI 3.14159265

void print_image_info(my1Image* image)
{
	printf("Current Image: ");
	if(image->length)
	{
		printf("Image loaded!\n");
		printf("Size: %d x %d\n",image->width,image->height);
		printf("Mask: %08X\n",image->mask);
	}
	else
	{
		printf("No image loaded!\n");
	}
}

void load_image(my1Image* image)
{
	char filename[80];
	int colorbmp;
	printf("Enter BMP filename: ");
	scanf("%s",filename);
	colorbmp = loadBMPimage(filename,image);
	if(colorbmp<0)
		printf("Cannot load input file '%s'! [%d]\n", filename, colorbmp);
	else
		printf("Color: %d.\n", colorbmp);
}

void save_image(my1Image* image)
{
	char filename[80];
	int errorlog;
	printf("Enter BMP filename: ");
	scanf("%s",filename);
	errorlog = saveBMPimage(filename,image);
	if(errorlog<0)
		printf("Cannot write to file '%s'! [%d]\n", filename, errorlog);
	else
		printf("Image written to '%s'.\n", filename);
}

void view_image(my1Image* image)
{
	SDL_Surface *screen;
	SDL_Surface *temp;
	SDL_Event event;

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

	char *pImage = malloc(image->height*image->width*3);
	if(!extract_rgb(image,pImage))
	{
		free(pImage);
		printf("Unable to extract data from image: %08X\n", image->mask);
		return;
	}

	// Create the temp surface from the raw RGB data
	temp = SDL_CreateRGBSurfaceFrom(pImage, image->width, image->height,
		24, image->width*3, 0, 0, 0, 0);
	if(!temp)
	{
		printf("Unable to load image to SDL: %s\n", SDL_GetError());
		return; 
	}
	free(pImage);

	SDL_BlitSurface(temp, NULL, screen, NULL);
	SDL_Flip(screen);
	SDL_FreeSurface(temp);

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

void menu_image(my1Image* image)
{
	int not_done = 1, select;
	my1Image tempimage;
	initimage(&tempimage);
	createimage(&tempimage,image->height,image->width);
	copyimage(image,&tempimage);
	do
	{
		printf("\n");
		printf("---------------------\n");
		printf("Image Processing Menu\n");
		printf("---------------------\n");
		printf("\n");
		print_image_info(image);
		printf("\n");
		printf("1 - View Image\n");
		printf("5 - Quit\n\n");
		printf("Your choice: ");
		scanf("%d",&select);
		switch(select)
		{
			case 1:
				view_image(&tempimage);
				break;
			case 2:
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				not_done = 0;
				break;
			default:
				printf("Invalid selection!\n\n");
		}
	}
	while(not_done);
	freeimage(&tempimage);
}

void menu_matrix(my1Image* image)
{
	printf("Coming soon...\n\n");
}

/* sample filters */
int laplace_image(my1Image *src, my1Image *dst);
int sobel_x_image(my1Image *src, my1Image *dst);
int sobel_y_image(my1Image *src, my1Image *dst);
int sobel_image(my1Image *src, my1Image *dst_mag, my1Image *dst_ang);
int laplace_frame(my1IFrame *src, my1IFrame *dst);
int gauss_frame(my1IFrame *src, my1IFrame *dst, float sigma, float *over);

int main(int argc, char* argv[])
{
	int not_done = 1, select, loop;
	char *pfilename = 0x0;
	my1Image currimage;
	initimage(&currimage);

	/* check program arguments */
	for(loop=1;loop<argc;loop++)
	{
		if(!pfilename)
		{
			pfilename = argv[loop];
		}
		else
		{
			printf("Unknown parameter %s!\n",argv[loop]);
		}
	}

	/* try to open file if requested! */
	select = loadBMPimage(pfilename,&currimage);
	if(select<0)
		printf("Cannot load input file '%s'! [%d]\n", pfilename, select);

	/* main loop */
	do
	{
		printf("\n");
		printf("--------------------------\n");
		printf("Test Program for my1imgpro\n");
		printf("--------------------------\n");
		printf("\n");
		print_image_info(&currimage);
		printf("\n");
		printf("1 - Load Image\n");
		printf("2 - Save Image\n");
		printf("3 - View Image\n");
		printf("4 - Image Menu\n");
		printf("5 - Matrix Menu\n");
		printf("6 - Quit\n\n");
		printf("Your choice: ");
		scanf("%d",&select);
		switch(select)
		{
			case 1:
				load_image(&currimage);
				break;
			case 2:
				save_image(&currimage);
				break;
			case 3:
				view_image(&currimage);
				break;
			case 4:
				menu_image(&currimage);
				break;
			case 5:
				menu_matrix(&currimage);
				break;
			case 6:
				not_done = 0;
				break;
			default:
				printf("Invalid selection!\n\n");
		}
	}
	while(not_done);
	freeimage(&currimage);
	return 0;
}

/* sample filters */

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

int iabs(int value)
{
	if(value<0) value = -value;
	return value;
}

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

/* sample filters */

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

#define THRES2 0.0039 /* 1/256 - max division possible for grayscale */
#define THRES1 0.0156 /* 1/64 - leave 2 significant bits */
#define THRESH 0.0625 /* 1/16 - leave 4 significant bits */

float fgaussian2d(float x, float y, float sigma)
{
	/* isotropic (circularly symmetric) mean at (0,0) */
	return pow(2.71828,-(((x*x)+(y*y))/(2*sigma*sigma)))/(sigma*sigma*2*PI);
}

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
