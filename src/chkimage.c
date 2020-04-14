/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include "my1image_main.h"
#include "my1image_work.h"
/*----------------------------------------------------------------------------*/
#define WFLAG_SHOW_ORIGINAL 0x01
/*----------------------------------------------------------------------------*/
typedef struct _my1iwhat_t
{
	my1image_appw_t awin;
	my1image_t buff, *orig;
	int flag;
}
my1iwhat_t;
/*----------------------------------------------------------------------------*/
int init_what(void* data, void* that, void* xtra)
{
	my1dotask_t *dotask = (my1dotask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)dotask->data;
	image_appw_init(&what->awin);
	image_init(&what->buff);
	what->orig = 0x0;
	what->flag = 0;
	return 0;
}
/*----------------------------------------------------------------------------*/
int free_what(void* data, void* that, void* xtra)
{
	my1dotask_t *dotask = (my1dotask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)dotask->data;
	image_appw_free(&what->awin);
	image_free(&what->buff);
	return 0;
}
/*----------------------------------------------------------------------------*/
int args_what(void* data, void* that, void* xtra)
{
	int loop, argc, *temp = (int*) that;
	char** argv = (char**) xtra;
	my1dotask_t *dotask = (my1dotask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)dotask->data;
	/**my1imain_t *mdat = (my1imain_t*)dotask->xtra;*/
	argc = *temp;
	/* check parameter? */
	for (loop=2;loop<argc;loop++)
	{
		if (!strncmp(argv[loop],"--original",10))
			what->flag |= WFLAG_SHOW_ORIGINAL;
		else
			printf("-- Unknown param '%s'!\n",argv[loop]);
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
int exec_what(void* data, void* that, void* xtra)
{
	my1dotask_t *dotask = (my1dotask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)dotask->data;
	my1imain_t *mdat = (my1imain_t*)that;
	mdat->list = image_work_create_all();
	if (what->flag&WFLAG_SHOW_ORIGINAL)
		what->orig = mdat->show;
	image_copy(&what->buff,mdat->show);
	mdat->show = &what->buff;
	return 0;
}
/*----------------------------------------------------------------------------*/
int show_what(void* data, void* that, void* xtra)
{
	my1dotask_t *dotask = (my1dotask_t*)data;
	my1iwhat_t *what = (my1iwhat_t*)dotask->data;
	my1imain_t *mdat = (my1imain_t*)that;
	if (what->orig)
	{
		/* show original */
		image_show(what->orig,&what->awin,"Source Image");
		/* modify name for main win */
		image_appw_name(&mdat->iwin,"Processed Image");
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
