/*----------------------------------------------------------------------------*/
#include "my1visdev.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
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
my1Image* fgrayfilter(my1Image* image, my1Image* result, void* userdata)
{
	if (image->mask!=0xffffff) return image;
	if (!result->data) createimage(result,image->height,image->width);
	int loop;
	for (loop=0;loop<image->length;loop++)
		result->data[loop] = color2gray(image->data[loop]);
	result->mask = 0;
	return result;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1Video cMain;
	my1Capture cCapture;
	my1Display cDisplay;
	SDL_Event event;
	my1VFilter grayfilter;
	int loop, filter = 1, errorcount = 0;
	char *pfilename = 0x0, *pdevice = 0x0;

	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (!strcmp(argv[loop],"--live"))
		{
			if (pdevice)
			{
				printf("Multiple source? (%s&%s)",pdevice,argv[loop]);
				errorcount++;
			}
			else if (loop<argc-1) /* still with param! */
			{
				pdevice = argv[++loop];
			}
			else
			{
				printf("No param for '--live'?");
				errorcount++;
			}
		}
		else
		{
			if (pfilename)
			{
				printf("Multiple source? (%s&%s)",pfilename,argv[loop]);
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
	initvideo(&cMain);
	initcapture(&cCapture);
	initdisplay(&cDisplay);
	cCapture.video = &cMain;
	cDisplay.video = &cMain;
	filter_init(&grayfilter,fgrayfilter);
	cMain.filter = &grayfilter;

	/* setup devices */
	if (pfilename)
		filecapture(&cCapture,pfilename);
	else if (pdevice)
		livecapture(&cCapture,pdevice);
	setupdisplay(&cDisplay);

	printf("Press 'h' for hotkeys.\n");
	printf("Starting main capture loop.\n");
	/* main capture loop */
	while (1)
	{
		grabcapture(&cCapture);
		if(cMain.newframe)
		{
			if (filter) filtervideo(&cMain);
			if (cMain.count>=0)
				printf("Video frame index: %d/%d\n", cMain.index, cMain.count);
			buffdisplay(&cDisplay);
			showdisplay(&cDisplay);
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
					playvideo(&cMain);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					printf("Stop.\n");
					stopvideo(&cMain);
				}
				else if(event.key.keysym.sym == SDLK_SPACE)
				{
					printf("Pause.\n");
					pausevideo(&cMain);
				}
				else if(event.key.keysym.sym == SDLK_f)
				{
					printf("Next.\n");
					nextvframe(&cMain);
				}
				else if(event.key.keysym.sym == SDLK_b)
				{
					printf("Previous.\n");
					printf("Not implemented... yet!\n");
					prevvframe(&cMain);
				}
				else if(event.key.keysym.sym == SDLK_l)
				{
					cMain.looping = !cMain.looping;
					printf("Looping=%d.\n",cMain.looping);
				}
				else if(event.key.keysym.sym == SDLK_h)
				{
					printf("%s",showkeys);
				}
				else if(event.key.keysym.sym == SDLK_z)
				{
					if(filter)
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
		postinput(&cMain);
	}

	filter_free(&grayfilter);
	cleancapture(&cCapture);
	cleandisplay(&cDisplay);
	cleanvideo(&cMain);

	return 0;
}
/*----------------------------------------------------------------------------*/
