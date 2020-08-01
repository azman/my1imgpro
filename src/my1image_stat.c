/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_STATC__
#define __MY1IMAGE_STATC__
/*----------------------------------------------------------------------------*/
#include "my1image_stat.h"
/*----------------------------------------------------------------------------*/
void image_get_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop, temp;
	/* clear histogram */
	for (loop=0;loop<GRAYLEVEL;loop++)
		hist->count[loop] = 0;
	/* count! */
	for (loop=0;loop<image->size;loop++)
	{
		temp = image->data[loop];
		hist->count[temp]++;
	}
	/* get index for highest/lowest count */
	temp = hist->count[0];
	hist->chkvalue = temp;
	hist->chkindex = -1;
	hist->maxvalue = temp;
	hist->maxindex = 0;
	hist->minvalue = temp;
	hist->minindex = 0;
	for (loop=1;loop<GRAYLEVEL;loop++)
	{
		temp = hist->count[loop];
		if (temp>hist->maxvalue)
		{
			hist->chkvalue = hist->maxvalue;
			hist->chkindex = hist->maxindex;
			hist->maxvalue = temp;
			hist->maxindex = loop;
		}
		/* in case first value IS maximum value */
		if (temp>hist->chkvalue)
		{
			if (loop!=hist->maxindex)
			{
				hist->chkvalue = temp;
				hist->chkindex = loop;
			}
		}
		/* look for non-zero minimum count */
		if (temp<hist->minvalue||!hist->minvalue)
		{
			hist->minvalue = temp;
			hist->minindex = loop;
		}
	}
}
/*----------------------------------------------------------------------------*/
void image_smooth_histogram(my1image_t *image, my1image_histogram_t *hist)
{
	int loop, size = image->size;
	float alpha = (float)WHITE/size;
	for (loop=1;loop<GRAYLEVEL;loop++)
		hist->tbuff[loop] = (int)(hist->count[loop]*alpha);
	for(loop=0;loop<size;loop++)
		image->data[loop] = hist->tbuff[image->data[loop]];
}
/*----------------------------------------------------------------------------*/
void histogram_get_threshold(my1image_histogram_t *hist)
{
	int loop, last, init, ends;
	int mids, temp, chkl, chkr;
	/* range limits, mids in right side */
	init = 0; ends = GRAYLEVEL-1;
	mids = ends>>1; temp = GRAYLEVEL>>1;
	/* prepare weights */
	chkl = chkr = 0;
	for(loop=init,last=ends;loop<temp;loop++,last--)
	{
		chkl += hist->count[loop];
		chkr += hist->count[last];
	}
	/* balanced histogram thresholding */
	while (init<=ends)
	{
		if (chkr>chkl)
		{
			while (!(temp=hist->count[ends--]));
			chkr -= temp;
			if (((init+ends)>>1)<mids)
			{
				mids--;
				chkr += hist->count[mids];
				chkl -= hist->count[mids];
			}
		}
		else /**if (chkl>=chkr)*/
		{
			while (!(temp=hist->count[init++]));
			chkl -= temp;
			if (((init+ends)>>1)>=mids)
			{
				chkl += hist->count[mids];
				chkr -= hist->count[mids];
				mids++;
			}
		}
	}
	hist->threshold = mids;
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_STATC__ */
/*----------------------------------------------------------------------------*/
