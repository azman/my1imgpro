/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image.h"
#include "my1video.h"
/*----------------------------------------------------------------------------*/
#define ERROR_FLAG 0x80
#define IMAGE_NONE 0
#define IMAGE_FILE 1
#define VIDEO_FILE 2
#define VIDEO_LIVE 3
/*----------------------------------------------------------------------------*/
/** in microsec! */
#define CAPTURE_DELAY 10
/*----------------------------------------------------------------------------*/
my1image_t* prepare_image_file(my1image_t* data, char* pick)
{
	int test;
	if (pick)
	{
		if((test=image_load(data,pick))<0)
			printf("-- Error loading file '%s'!(%x)\n",pick,(unsigned)test);
	}
	if (!data->size)
	{
		image_make(data,240,320);
		image_fill(data,BLACK);
	}
	return data;
}
/*----------------------------------------------------------------------------*/
my1image_t* prepare_video_feed(my1video_grab_t* grab, char* pick, int mode)
{
	if (mode==VIDEO_FILE) capture_file(grab,pick);
	else capture_live(grab,pick);
	return grab->video->frame;
}
/*----------------------------------------------------------------------------*/
void prepare_video_next(void* that_appw)
{
	my1image_appw_t* appw = (my1image_appw_t*) that_appw;
	my1video_t* video = (my1video_t*) appw->dodata;
	my1vgrab_t* vgrab = (my1vgrab_t*)video->capture;
	if (appw->doquit)
	{
		capture_free(vgrab);
		video_free(video);
		image_appw_free(appw);
		gtk_main_quit();
		return;
	}
	if (vgrab->fcontext) capture_grab(vgrab);
	if (video->flags&VIDEO_FLAG_NEW_FRAME)
	{
		video_filter_frame(video);
		video_post_frame(video);
		image_appw_draw(appw,video->frame);
		video_post_input(video);
	}
	image_appw_task(appw,prepare_video_next,CAPTURE_DELAY);
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1image_t buff, *show;
	my1video_t ivid;
	my1video_grab_t grab;
	my1image_appw_t iwin;
	int loop, mode = IMAGE_NONE;
	char* pick = 0x0;
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (argv[loop][0]!='-')
		{
			mode = IMAGE_FILE;
			pick = argv[loop];
		}
		else if (!strncmp(argv[loop],"--live",6))
		{
			mode = VIDEO_LIVE;
			pick = argv[++loop];
		}
		else if (!strncmp(argv[loop],"--video",7))
		{
			mode = VIDEO_FILE;
			pick = argv[++loop];
		}
		else
		{
			printf("Invalid argument! (%s)\n",argv[loop]);
			mode |= ERROR_FLAG;
		}
	}
	if (mode&ERROR_FLAG) return -1;
	/* initialize data */
	image_init(&buff);
	video_init(&ivid);
	capture_init(&grab,&ivid);
	/* prepare source */
	switch (mode)
	{
		case VIDEO_FILE: case VIDEO_LIVE:
			show = prepare_video_feed(&grab,pick,mode); break;
		case IMAGE_FILE: default:
			mode = IMAGE_FILE;
			show = prepare_image_file(&buff,pick); break;
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* show image */
	image_appw_show(&iwin,show,0x0,0);
	ivid.display = (void*) &iwin;
	iwin.dodata = (void*) &ivid;
	if (mode==IMAGE_FILE)
	{
		image_appw_name(&iwin,"MY1 Image");
		image_appw_domenu_full(&iwin);
		image_appw_task(&iwin,image_appw_is_done,ISDONE_TIMEOUT);
	}
	else
	{
		iwin.gofree = 0;
		video_play(&ivid);
		image_appw_name(&iwin,"MY1 Video");
		image_appw_task(&iwin,prepare_video_next,CAPTURE_DELAY);
	}
	/* main loop */
	gtk_main();
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
