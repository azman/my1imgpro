/*----------------------------------------------------------------------------*/
#include "my1image_mono.h"
/*----------------------------------------------------------------------------*/
#include "my1image_scan.h"
/*----------------------------------------------------------------------------*/
int morph_fit(my1image_scan_t* scan,  int *elem)
{
	int loop, step;
	int *ele1 = &elem[4];
	int *ele2 = &elem[6];
	for (loop=0,step=scan->icol-1;loop<3;loop++,step++)
	{
		if (elem[loop]&&!scan->prev[step]) return 0;
		if (ele1[loop]&&!scan->curr[step]) return 0;
		if (ele2[loop]&&!scan->next[step]) return 0;
	}
	return 1; /* fit */
}
/*----------------------------------------------------------------------------*/
int morph_hit(my1image_scan_t* scan,  int *elem)
{
	int loop, step;
	int *ele1 = &elem[4];
	int *ele2 = &elem[6];
	for (loop=0,step=scan->icol-1;loop<3;loop++,step++)
	{
		if (elem[loop]&&scan->prev[step]) return 1;
		if (ele1[loop]&&scan->curr[step]) return 1;
		if (ele2[loop]&&scan->next[step]) return 1;
	}
	return 0; /* no hit */
}
/*----------------------------------------------------------------------------*/
void image_erode(my1image_t *img, my1image_t *res, int* elem)
{
	my1image_scan_t scan;
	image_make(res,img->height,img->width);
	scan_init(&scan,img->data,img->height,img->width,MY1IMAGE_SCAN_SKIP);
	scan_prep(&scan);
	while (scan_next(&scan))
	{
		if (scan_skip(&scan)) res->data[scan.loop] = 0;
		else res->data[scan.loop] = morph_fit(&scan,elem);
	}
}
/*----------------------------------------------------------------------------*/
void image_dilate(my1image_t *img, my1image_t *res, int* elem)
{
	my1image_scan_t scan;
	image_make(res,img->height,img->width);
	scan_init(&scan,img->data,img->height,img->width,MY1IMAGE_SCAN_SKIP);
	scan_prep(&scan);
	while (scan_next(&scan))
	{
		if (scan_skip(&scan)) res->data[scan.loop] = 0;
		else res->data[scan.loop] = morph_hit(&scan,elem);
	}
}
/*----------------------------------------------------------------------------*/
