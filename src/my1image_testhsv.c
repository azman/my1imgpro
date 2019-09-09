/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*----------------------------------------------------------------------------*/
#include "my1image_chsv.h"
/*----------------------------------------------------------------------------*/
#define ERROR_MAX 5
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
	if(argc>1)
	{
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
