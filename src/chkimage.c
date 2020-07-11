/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include "my1image_main.h"
#include "my1image_work.h"
#include "my1libav_grab.h"
/*----------------------------------------------------------------------------*/
#define WFLAG_SHOW_ORIGINAL 0x01
#define WFLAG_VIDEO_MODE 0x10
#define WFLAG_VIDEO_LIVE (WFLAG_VIDEO_MODE|0x20)
/*----------------------------------------------------------------------------*/
typedef struct _my1iwhat_t
{
	my1libav_grab_t avgrab;
	my1image_appw_t awin;
	my1image_t buff, *orig;
	char *list; /* filter list from args */
	int flag;
}
my1iwhat_t;
/*----------------------------------------------------------------------------*/
int init_what(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	libav1_init(&what->avgrab,&mdat->load);
	image_appw_init(&what->awin);
	image_init(&what->buff);
	what->orig = 0x0;
	what->list = 0x0;
	what->flag = 0;
	return 0;
}
/*----------------------------------------------------------------------------*/
int free_what(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	image_free(&what->buff);
	image_appw_free(&what->awin);
	libav1_free(&what->avgrab);
	return 0;
}
/*----------------------------------------------------------------------------*/
int igrab_grab_video(void* data, void* that, void* xtra)
{
	my1itask_t* ptemp = (my1itask_t*) data;
	my1image_grab_t* igrab = (my1image_grab_t*) that;
	my1iwhat_t *what = (my1iwhat_t*)ptemp->data;
	my1libav_grab_t* vgrab = (my1libav_grab_t*)&what->avgrab;
	if (!vgrab->flag)
	{
		if ((what->flag&WFLAG_VIDEO_LIVE)==WFLAG_VIDEO_LIVE)
			libav1_live(vgrab,igrab->pick);
		else
			libav1_file(vgrab,igrab->pick);
		if (!vgrab->flag)
			igrab->flag |= IGRAB_FLAG_LOAD_ERROR;
	}
	libav1_grab(vgrab,igrab->grab);
	return igrab->flag;
}
/*----------------------------------------------------------------------------*/
int args_what(void* data, void* that, void* xtra)
{
	int loop, argc, *temp = (int*) that;
	char** argv = (char**) xtra;
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)task->temp;
	my1igrab_t *grab = (my1igrab_t*)&mdat->grab;
	argc = *temp;
	if (mdat->flag&IFLAG_ERROR) return 0;
	/* re-check parameter for video option */
	loop = 1;
	if (argc>2)
	{
		if (!strncmp(grab->pick,"--video",8))
		{
			grab->pick = argv[++loop];
			what->flag |= WFLAG_VIDEO_MODE;
		}
		else if (!strncmp(grab->pick,"--live",7))
		{
			grab->pick = argv[++loop];
			what->flag |= WFLAG_VIDEO_LIVE;
		}
	}
	/* setup grabber */
	if (what->flag&WFLAG_VIDEO_MODE)
	{
		grab->do_grab.task = igrab_grab_video;
		grab->do_grab.data = (void*)what;
		grab->grab = &mdat->load;
		mdat->flag |= IFLAG_VIDEO_MODE;
	}
	/* continue with the rest */
	for (++loop;loop<argc;loop++)
	{
		if (!strncmp(argv[loop],"--original",10))
			what->flag |= WFLAG_SHOW_ORIGINAL;
		else if (!strncmp(argv[loop],"--filter",9))
			what->list = argv[++loop];
		else
			printf("-- Unknown param '%s'!\n",argv[loop]);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
int prep_what(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	mdat->list = image_work_create_all();
	/** check requested filters */
	if (what->list)
	{
		char* pchk = what->list;
		int loop = 0, curr = 0, stop = 0;
		while (!stop)
		{
			switch (pchk[loop])
			{
				case 0x0: stop = 1;
				case ',':
					pchk[loop] = 0x0;
					printf("-- Loading filter '%s'... ",&pchk[curr]);
					if (imain_filter_doload(mdat,&pchk[curr]))
						printf("OK!\n");
					else printf("not found?!\n");
					if (!stop) pchk[loop] = ',';
					curr = ++loop;
					break;
				default: loop++;
			}
		}
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
int exec_what(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	if (what->flag&WFLAG_SHOW_ORIGINAL)
		what->orig = mdat->show;
	image_copy(&what->buff,mdat->show);
	mdat->show = &what->buff;
	return 0;
}
/*----------------------------------------------------------------------------*/
int what_on_keychk(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1imain_t *mdat = (my1imain_t*)task->temp;
	GdkEventKey *event = (GdkEventKey*)xtra;
	if (event->keyval == GDK_KEY_z)
		imain_menu_filter_enable(mdat,!(mdat->flag&IFLAG_FILTER_EXE));
	return 0;
}
/*----------------------------------------------------------------------------*/
int show_what(void* data, void* that, void* xtra)
{
	my1itask_t *task = (my1itask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)task->data;
	my1imain_t *mdat = (my1imain_t*)that;
	if (what->orig)
	{
		/* show original */
		image_show(what->orig,&what->awin,"Source Image");
		/* modify name for main win */
		image_appw_name(&mdat->iwin,"Processed Image");
	}
	if (!(mdat->flag&IFLAG_VIDEO_MODE))
		image_appw_domenu(&mdat->iwin);
	imain_domenu_filters(mdat);
	image_appw_domenu_quit(&mdat->iwin);
	if (what->flag&WFLAG_VIDEO_MODE)
	{
		itask_make(&mdat->iwin.keychk,what_on_keychk,(void*)what);
		mdat->iwin.keychk.temp = (void*) mdat;
		imain_loop(mdat,0);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1iwhat_t what;
	my1imain_t data;
	my1iwork_t work;
	iwork_make(&work,&what);
	work.init.task = init_what;
	work.free.task = free_what;
	work.args.task = args_what;
	work.prep.task = prep_what;
	work.proc.task = exec_what;
	work.show.task = show_what;
	imain_init(&data,&work);
	imain_args(&data,argc,argv);
	imain_prep(&data);
	imain_proc(&data);
	if (!(data.flag&IFLAG_ERROR)) gtk_init(&argc,&argv);
	imain_show(&data);
	if (!(data.flag&IFLAG_ERROR)) gtk_main();
	imain_free(&data);
	return 0;
}
/*----------------------------------------------------------------------------*/
