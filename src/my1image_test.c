/*----------------------------------------------------------------------------*/
#include "my1image_data.h"
#include "my1image_file.h"
#include "my1image_file_png.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#define DEF_WIDTH 320
#define DEF_HEIGHT 240
/*----------------------------------------------------------------------------*/
#define MY1APP_NAME "my1image_test"
#define MY1APP_INFO "MY1Image Test Program"
#ifndef MY1APP_VERS
#define MY1APP_VERS "build"
#endif
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	int test;
	char *pname;
	my1image_data_t data;
	/* print tool info */
	printf("\n%s - %s (%s)\n",MY1APP_NAME,MY1APP_INFO,MY1APP_VERS);
	printf("  => by azman@my1matrix.org\n\n");
	/* add png support */
	image_format_insert(&ipng);
	/* initialize image_data */
	image_data_init(&data);
	/* check program argument */
	if(argc>1)
	{
		pname = argv[1];
		if((test=image_load(&data.appw.buff,pname))<0)
		{
			printf("Error loading file '%s'!(%x)\n",pname,(unsigned)test);
			return test;
		}
	}
	else
	{
		/* create blank image at default size */
		image_make(&data.appw.buff,DEF_HEIGHT,DEF_WIDTH);
		image_fill(&data.appw.buff,BLACK);
	}
	/* initialize gui */
	gtk_init(&argc,&argv);
	/* setup auto-quit on close */
	data.appw.goquit = 1;
	/* create all filters in my1image_work */
	image_data_work(&data);
	/** check requested filters */
	for (test=2;test<argc;test++)
		image_data_filter_load(&data,argv[test]);
	/* assign image */
	image_data_make(&data,&data.appw.buff);
	/* allow histogram */
	image_hist_make(&data.hist);
	/* event handlers */
	image_data_events(&data);
	/* menu stuff */
	image_data_domenu(&data);
	/* main loop */
	gtk_main();
	/* cleanup image_test */
	image_data_free(&data);
	/* done! */
	putchar('\n');
	return 0;
}
/*----------------------------------------------------------------------------*/
