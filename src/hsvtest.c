/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_chsv.h"
/*----------------------------------------------------------------------------*/
#define ERROR_MAX 5
/*----------------------------------------------------------------------------*/
typedef struct _hsv_stat_t
{
	int col, max, min, del, hue, reg, off, sub, chk;
}
hsv_stat_t;
/*----------------------------------------------------------------------------*/
void hsv_get_stat(my1hsv_t hsv, hsv_stat_t* hsv_stat)
{
	int max, min, del, hue, reg, off, sub, chk;
	/* check zero saturation => pure grayscale! */
	if (!hsv.s)
	{
		hsv_stat->col = 0;
		hsv_stat->max = hsv.v;
		return;
	}
	/* max & min for rgb are 100% reproducible! */
	max = hsv.v;
	del = hsv.s * max / WHITE;
	min = max - del;
	/* third value */
	hue = hsv.h;
	reg = (hue / HUE_DIFF); /* should be <HUE_PART */
	off = (hue - ((reg>>1) * HUE_COMP)); /* offset from HUE_COMP */
	if (off>HUE_DIFF) off = HUE_COMP-off;
	sub = (off * del / HUE_DIFF); /* get diff for third value */
	chk = (sub + min); /* third value */
	/* assign to struct */
	hsv_stat->col = 1;
	hsv_stat->max = max;
	hsv_stat->min = min;
	hsv_stat->del = del;
	hsv_stat->hue = hue;
	hsv_stat->reg = reg;
	hsv_stat->off = off;
	hsv_stat->sub = sub;
	hsv_stat->chk = chk;
}
/*----------------------------------------------------------------------------*/
int abs(int val)
{
	if (val<0) val = -val;
	return val;
}
/*----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
	my1rgb_t test, buff; my1hsv_t temp;
	int loop, check = 0, ckmax = -1, ckmin = -1;
	int error = 0, domax = ERROR_MAX;
	/* check program arguments */
	for(loop=1;loop<argc;loop++)
	{
		if(argv[loop][0]=='-')
		{
			if(!strcmp(argv[loop],"--domax"))
			{
				loop++;
				if(loop<argc)
					domax = atoi(argv[loop]);
				else
					printf("Cannot get max error! Using {%d}\n",domax);
			}
			else
				printf("Unknown option '%s'!\n",argv[loop]);
		}
		else
			printf("Unknown parameter %s!\n",argv[loop]);
	}
	/* run test */
	test.g = BLACK; test.a = 0;
	printf("Testing RGB <-> HSV Color Conversion [BEGIN]");
	printf(" => {MaxError:%d}\n",domax);
	for (;;)
	{
		test.r = BLACK;
		for (;;)
		{
			test.b = BLACK;
			for (;;)
			{
				temp = rgb2hsv(test);
				buff = hsv2rgb(temp);
				if (abs(test.r-buff.r)>domax||
					abs(test.g-buff.g)>domax||
					abs(test.b-buff.b)>domax)
				{
					hsv_stat_t stat;
					printf("[CHECK](%d,%d,%d)->{%d,%d,%d}->"
						"(%d,%d,%d)=>",test.r,test.g,test.b,
						temp.h,temp.s,temp.v,buff.r,buff.g,buff.b);
					hsv_get_stat(temp,&stat);
					printf("{max:%d,del:%d,min:%d,hue:%d,"
						"reg:%d,off:%d,sub:%d,chk:%d}\n",
						stat.max,stat.del,stat.min,stat.hue,
						stat.reg,stat.off,stat.sub,stat.chk);
					check += stat.del;
					if (ckmax<0||stat.del>ckmax) ckmax = stat.del;
					if (ckmin<0||stat.del<ckmin) ckmin = stat.del;
					error++;
				}
				if (++test.b==0) break;
			}
			if (++test.r==0) break;
		}
		if (++test.g==0) break;
	}
	printf("Testing RGB <-> HSV Color Conversion [DONE]");
	printf(" => {DoMax:%d,Error:%d,DifAv:%d,DifMx:%d,DifMn:%d}\n",
		domax,error,error>0?(check/error):0,ckmax,ckmin);
	return error;
}
/*----------------------------------------------------------------------------*/
