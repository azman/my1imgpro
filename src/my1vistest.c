#include "my1visdev.h"
//#include "my1imgbmp.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

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

int main(int argc, char* argv[])
{
	my1Video cMain;
	my1Capture cCapture;
	my1Display cDisplay;
	SDL_Event event;
	int loop, live = -1, errorcount = 0;
	char *pFileName = 0x0;

	/* check parameter */
	for(loop=1;loop<argc;loop++)
	{
		if(!strcmp(argv[loop],"--live"))
		{
			if(loop<argc-1) // still with param!
				live = atoi(argv[++loop]);
		}
		else
		{
			if(pFileName)
			{
				// already given filename!
				printf("Invalid param? (%s)",argv[loop]);
				errorcount++;
			}
			else
			{
				pFileName = argv[loop];
			}
		}
	}
	if(errorcount) return -errorcount;

	/* initialize main structures */
	initvideo(&cMain);
	initcapture(&cCapture);
	initdisplay(&cDisplay);
	cCapture.video = &cMain;
	cDisplay.video = &cMain;

	/* setup devices */
	if(pFileName)
		filecapture(&cCapture,pFileName);
	else
		livecapture(&cCapture,live);
	setupdisplay(&cDisplay);

	printf("Press 'h' for hotkeys.\n");
	printf("Starting main capture loop.\n");
	// main capture loop
	while(1)
	{
		grabcapture(&cCapture);
		if(cMain.newframe)
		{
			printf("Video frame index: %d/%d\n", cMain.index, cMain.count);
			buffdisplay(&cDisplay);
			showdisplay(&cDisplay);
		}
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
					printf("User abort!\n");
					break;
				}
				else if(event.key.keysym.sym == SDLK_c)
				{
					printf("Play.\n");
					playvideo(&cMain);
				}
				else if(event.key.keysym.sym == SDLK_s)
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
			}
		}
		postinput(&cMain);
	}

	cleancapture(&cCapture);
	cleandisplay(&cDisplay);
	cleanvideo(&cMain);

	return 0;
}
