/*----------------------------------------------------------------------------*/
#ifndef __MY1IMAGE_ARGBC__
#define __MY1IMAGE_ARGBC__
/*----------------------------------------------------------------------------*/
#include "my1image_argb.h"
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b)
{
	my1rgb_t *buff = (my1rgb_t*) &data;
	*r =  buff->r; *g =  buff->g; *b =  buff->b;
	/***r=(data&0xff0000)>>16; *g=(data&0xff00)>>8; *b=(data&0xff);*/
}
/*----------------------------------------------------------------------------*/
int encode_rgb(cbyte r, cbyte g, cbyte b)
{
	my1rgb_t temp;
	int *buff = (int*) &temp;
	temp.a = 0; temp.r = r; temp.g = g; temp.b = b;
	/**return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);*/
	return *buff;
}
/*----------------------------------------------------------------------------*/
int gray4rgb(cbyte r, cbyte g, cbyte b)
{
	return (((int)g<<1)+(int)r+(int)b)>>2; /* average: (g+g+r+b)/4 */
}
/*----------------------------------------------------------------------------*/
int gray2color(int data)
{
	return encode_rgb(data,data,data);
}
/*----------------------------------------------------------------------------*/
int color2gray(int data)
{
	cbyte r, g, b;
	decode_rgb(data,&r,&g,&b);
	/** consider luminosity? */
	/*0.21 R + 0.71 G + 0.07 B*/
	/** go for speed? */
	return gray4rgb(r,g,b);
}
/*----------------------------------------------------------------------------*/
#endif /** __MY1IMAGE_ARGBC__ */
/*----------------------------------------------------------------------------*/
