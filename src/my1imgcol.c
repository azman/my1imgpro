/*----------------------------------------------------------------------------*/
#include "my1imgcol.h"
/*----------------------------------------------------------------------------*/
int image_assign_rgb(my1image_t *image, cbyte *rgb)
{
	char chkr, chkg, chkb;
	int loop, index = 0;
	switch (image->mask)
	{
		case IMASK_COLOR24:
			for(loop=0;loop<image->length;loop++)
			{
				image->data[loop] = encode_rgb(rgb[index+2],
					rgb[index+1],rgb[index+0]);
				index += 3;
			}
			break;
		default: /* assume grayscale */
			for(loop=0;loop<image->length;loop++)
			{
				chkr = rgb[index++];
				chkg = rgb[index++];
				chkb = rgb[index++];
				image->data[loop] = ((int)chkr+chkg+chkb)/3;
			}
			break;
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
int image_extract_rgb(my1image_t *image, cbyte *rgb)
{
	int loop, index = 0;
	switch (image->mask)
	{
		case IMASK_COLOR24:
			for(loop=0;loop<image->length;loop++)
			{
				decode_rgb(image->data[loop],&rgb[index+2],
					&rgb[index+1],&rgb[index+0]);
				index += 3;
			}
			break;
		default: /* assume grayscale */
			for(loop=0;loop<image->length;loop++)
			{
				rgb[index++] = (image->data[loop]&0xff);
				rgb[index++] = (image->data[loop]&0xff);
				rgb[index++] = (image->data[loop]&0xff);
			}
			break;
	}
	return image->mask;
}
/*----------------------------------------------------------------------------*/
void image_grayscale(my1image_t *image)
{
	int loop;
	switch (image->mask)
	{
		case IMASK_COLOR24:
		{
			cbyte r, g, b;
			for(loop=0;loop<image->length;loop++)
			{
				decode_rgb(image->data[loop],&r,&g,&b);
				image->data[loop] = (((unsigned int)r+g+b)/3)&0xFF;
				/** consider luminosity? */
				/*0.21 R + 0.71 G + 0.07 B*/
			}
			image->mask = IMASK_GRAY;
			break;
		}
		case IMASK_BINARY1:
		{
			for(loop=0;loop<image->length;loop++)
			{
				if (image->data[loop]) image->data[loop] = WHITE;
				else image->data[loop] = BLACK;
			}
			image->mask = IMASK_GRAY;
			break;
		}
	}
}
/*----------------------------------------------------------------------------*/
int encode_rgb(cbyte r, cbyte g, cbyte b)
{
	return (((int)r&0xff)<<16) | (((int)g&0xff)<<8) | ((int)b&0xff);
}
/*----------------------------------------------------------------------------*/
void decode_rgb(int data, cbyte *r, cbyte *g, cbyte *b)
{
	*r = (data&0xff0000)>>16;
	*g = (data&0xff00)>>8;
	*b = (data&0xff);
}
/*----------------------------------------------------------------------------*/
