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
typedef struct _work_t
{
	my1image_t buff, *show;
	my1video_t ivid;
	my1vgrab_t grab;
	my1image_appw_t iwin;
	int mode;
	char* pick;
}
work_t;
/*----------------------------------------------------------------------------*/
void work_init(work_t* work)
{
	image_init(&work->buff);
	work->show = 0x0;
	video_init(&work->ivid);
	capture_init(&work->grab,&work->ivid);
	image_appw_init(&work->iwin);
	work->mode = IMAGE_NONE;
	work->pick = 0x0;
}
/*----------------------------------------------------------------------------*/
void work_free(work_t* work)
{
	image_appw_free(&work->iwin);
	capture_free(&work->grab);
	video_free(&work->ivid);
	image_free(&work->buff);
}
/*----------------------------------------------------------------------------*/
void work_args(work_t* work, int argc, char* argv[])
{
	int loop;
	/* check parameter */
	for (loop=1;loop<argc;loop++)
	{
		if (argv[loop][0]!='-')
		{
			work->mode = IMAGE_FILE;
			work->pick = argv[loop];
			break;
		}
		else if (!strncmp(argv[loop],"--live",6))
		{
			work->mode = VIDEO_LIVE;
			work->pick = argv[++loop];
			break;
		}
		else if (!strncmp(argv[loop],"--video",7))
		{
			work->mode = VIDEO_FILE;
			work->pick = argv[++loop];
			break;
		}
		else
		{
			printf("Invalid argument! (%s)\n",argv[loop]);
			work->mode |= ERROR_FLAG;
		}
	}
}
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
void work_prep(work_t* work)
{
	if (work->mode&ERROR_FLAG) return;
	/* prepare source */
	switch (work->mode)
	{
		case VIDEO_FILE: case VIDEO_LIVE:
			work->show = prepare_video_feed(&work->grab,work->pick,work->mode);
			break;
		case IMAGE_FILE: default:
			work->mode = IMAGE_FILE;
			work->show = prepare_image_file(&work->buff,work->pick);
			break;
	}
}
/*----------------------------------------------------------------------------*/
/** in microsec! */
#define CAPTURE_DELAY 10
/*----------------------------------------------------------------------------*/
void prepare_video_next(void* that_appw)
{
	my1image_appw_t* appw = (my1image_appw_t*) that_appw;
	my1video_t* video = (my1video_t*) appw->dodata;
	my1vgrab_t* vgrab = (my1vgrab_t*)video->capture;
	if (appw->doquit)
	{
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
void work_show(work_t* work)
{
	image_appw_show(&work->iwin,work->show,0x0,0);
	work->ivid.display = (void*) &work->iwin;
	work->iwin.dodata = (void*) &work->ivid;
	if (work->mode==IMAGE_FILE)
	{
		image_appw_name(&work->iwin,"MY1 Image");
		image_appw_domenu_full(&work->iwin);
		image_appw_task(&work->iwin,image_appw_is_done,ISDONE_TIMEOUT);
	}
	else
	{
		work->iwin.gofree = 0;
		video_play(&work->ivid);
		image_appw_name(&work->iwin,"MY1 Video");
		image_appw_task(&work->iwin,prepare_video_next,CAPTURE_DELAY);
	}
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	work_t data;
	/* init data */
	work_init(&data);
	work_args(&data,argc,argv);
	work_prep(&data);
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* show data */
	work_show(&data);
	/* main loop */
	gtk_main();
	/* clean up */
	work_free(&data);
	/* done! */
	return 0;
}
/*----------------------------------------------------------------------------*/
