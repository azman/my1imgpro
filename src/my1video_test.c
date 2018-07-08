/*----------------------------------------------------------------------------*/
#include "my1video_dev.h"
#include "my1image_fpo.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#ifndef MY1APP_PROGNAME
#define MY1APP_PROGNAME "my1video_test"
#endif
#ifndef MY1APP_PROGVERS
#define MY1APP_PROGVERS "build"
#endif
#ifndef MY1APP_PROGINFO
#define MY1APP_PROGINFO "Basic Video Tool Library"
#endif
/*----------------------------------------------------------------------------*/
static char showkeys[] =
{
	"\t---------------------------\n"
	"\tDefined Keys:\n"
	"\t---------------------------\n"
	"\tc       - play video\n"
	"\ts       - stop video\n"
	"\t<SPACE> - pause video\n"
	"\tf       - next frame\n"
	"\tb       - previous frame\n"
	"\tl       - toggle looping\n"
	"\th       - show this message\n"
	"\t---------------------------\n"
};
/*----------------------------------------------------------------------------*/
my1image_t* fgrayfilter(my1image_t* image, my1image_t* check, void* userdata)
{
	int loop;
	if (image->mask!=IMASK_COLOR24) return image;
	if (!check->data) image_make(check,image->height,image->width);
	for (loop=0;loop<image->length;loop++)
		check->data[loop] = color2gray(image->data[loop]);
	check->mask = IMASK_GRAY;
	return check;
}
/*----------------------------------------------------------------------------*/
my1image_t* flaplacefilter(my1image_t* img, my1image_t* res, void* userdata)
{
	my1frame_t buff1, buff2;
	float coeff[] = { 0.0,-1.0,0.0, -1.0,4.0,-1.0, 0.0,-1.0,0.0 };
	my1frame_kernel_t kernel;
	if (!frame_kernel_init(&kernel,3))
		return img;
	frame_kernel_make(&kernel,coeff);
	frame_init(&buff1);
	frame_init(&buff2);
	if(frame_make(&buff1,img->height,img->width)&&
		frame_make(&buff2,img->height,img->width))
	{
		frame_get_image(&buff1,img,1);
		frame_correlate(&buff1,&buff2,&kernel);
		if(!res->data) image_make(res,img->height,img->width);
		frame_set_image(&buff2,res,1);
		res->mask = IMASK_GRAY;
	}
	frame_free(&buff1);
	frame_free(&buff2);
	frame_kernel_free(&kernel);
	return res;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1video_t cMain;
	my1video_capture_t cCapture;
	my1video_display_t cDisplay;
	SDL_Event event;
	my1image_filter_t grayfilter;
	my1image_filter_t edgefilter;
	my1image_filter_t morefilter;
	my1image_filter_t *pfilter = 0x0;
	int loop, filter = 0, errorcount = 0;
	char *pfilename = 0x0, *pdevice = 0x0;

	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_PROGNAME,MY1APP_PROGINFO,MY1APP_PROGVERS);
	printf("  => by azman@my1matrix.org\n\n");

	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (!strcmp(argv[loop],"--live"))
		{
			if (pdevice)
			{
				printf("Multiple source? (%s&%s)\n",pdevice,argv[loop]);
				errorcount++;
			}
			else if (loop<argc-1) /* still with param! */
			{
				/* on linux this should be /dev/video0 or something... */
				pdevice = argv[++loop];
			}
			else
			{
				printf("No param for '--live'?\n");
				errorcount++;
			}
		}
		else
		{
			if (pfilename)
			{
				printf("Multiple source? (%s&%s)\n",pfilename,argv[loop]);
				errorcount++;
			}
			else
			{
				pfilename = argv[loop];
			}
		}
	}
	if (errorcount) return -errorcount;
	if (!pfilename&&!pdevice)
	{
		printf("No video source requested!\n");
		exit(-1);
	}

	/* initialize main structures */
	video_init(&cMain);
	capture_init(&cCapture);
	display_init(&cDisplay);
	cCapture.video = &cMain;
	cDisplay.video = &cMain;
	filter_init(&grayfilter,fgrayfilter);
	filter_init(&edgefilter,flaplacefilter);
	filter_init(&morefilter,flaplacefilter);
	pfilter = filter_insert(pfilter,&grayfilter);
	pfilter = filter_insert(pfilter,&edgefilter);
	pfilter = filter_insert(pfilter,&morefilter);
	video_filter_init(&cMain,pfilter);

	/* setup devices */
	if (pfilename)
		capture_file(&cCapture,pfilename);
	else if (pdevice)
		capture_live(&cCapture,pdevice);
	display_make(&cDisplay);
	display_name(&cDisplay, "MY1 Video Test", "my1video_test");

	printf("Press 'h' for hotkeys.\n");
	printf("Starting main capture loop.\n");
	/* main capture loop */
	while (1)
	{
		capture_grab(&cCapture);
		if(cMain.newframe)
		{
			if (filter) video_filter(&cMain);
			if (cMain.count>=0)
				printf("Video frame index: %d/%d\n", cMain.index, cMain.count);
			display_make(&cDisplay);
			display_view(&cDisplay);
		}
		if (SDL_PollEvent(&event))
		{
			if (event.type==SDL_QUIT)
			{
				break;
			}
			else if (event.type==SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_q)
				{
					printf("User abort!\n");
					break;
				}
				else if (event.key.keysym.sym == SDLK_c)
				{
					printf("Play.\n");
					video_play(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					printf("Stop.\n");
					video_stop(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_SPACE)
				{
					printf("Pause.\n");
					video_hold(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_f)
				{
					printf("Next.\n");
					video_next_frame(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_b)
				{
					printf("Previous.\n");
					printf("Not implemented... yet!\n");
					video_prev_frame(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_l)
				{
					cMain.looping = !cMain.looping;
					printf("Looping=%d.\n",cMain.looping);
				}
				else if (event.key.keysym.sym == SDLK_h)
				{
					printf("%s",showkeys);
				}
				else if (event.key.keysym.sym == SDLK_z)
				{
					if (filter)
					{
						filter = 0;
						printf("Disable filter.\n");
					}
					else
					{
						filter = 1;
						printf("Enable filter.\n");
					}
				}
			}
		}
		video_post_input(&cMain);
	}

	filter_free(&grayfilter);
	display_free(&cDisplay);
	capture_free(&cCapture);
	video_free(&cMain);

	return 0;
}
/*----------------------------------------------------------------------------*/
