/*----------------------------------------------------------------------------*/
#include "my1image_chsv.h"
/*----------------------------------------------------------------------------*/
my1hsv_t rgb2hsv(my1rgb_t rgb)
{
	my1hsv_t hsv;
	int min, max, chk, del;
	/* get min */
	min = rgb.r < rgb.g ? rgb.r : rgb.g;
	min = rgb.b < min ? rgb.b : min;
	/* get max */
	max = rgb.r > rgb.g ? rgb.r : rgb.g;
	max = rgb.b > max ? rgb.b : max;
	/* get range */
	del = max - min; /* del IS chroma */
	/* get value */
	hsv.v = max;
	if (!max) /* all colors are 0! BLACK! */
	{
		hsv.s = 0;
		hsv.h = 0;
		return hsv;
	}
	/* get saturation */
	hsv.s = WHITE*del/max;
	if (!hsv.s) /* del is zero! all colors at same graylevel! */
	{
		hsv.h = 0; /* should be undefined? */
		return hsv;
	}
	/* get hue */
	if (max==rgb.g) /* green has precedence :) */
	{
		hsv.h = HUE_INIT_G + ((HUE_DIFF*(rgb.b-rgb.r))/del);
	}
	else if (max==rgb.r)
	{
		chk = HUE_INIT_R + ((HUE_DIFF*(rgb.g-rgb.b))/del);
		/* check for wrap-around angle */
		if (chk<0) chk += HUE_FULL;
		hsv.h = chk;
	}
	else /* if (max==rgb.b) */
	{
		hsv.h = HUE_INIT_B + ((HUE_DIFF*(rgb.r-rgb.g))/del);
	}
	/* done! */
	return hsv;
}
/*----------------------------------------------------------------------------*/
my1rgb_t hsv2rgb(my1hsv_t hsv)
{
	my1rgb_t rgb;
	int max, min, del, hue, reg, off, chk;
	rgb.a = 0;
	/* check grayscale? */
	if (hsv.s==0)
	{
		rgb.r = hsv.v;
		rgb.g = hsv.v;
		rgb.b = hsv.v;
		return rgb;
	}
	/* get stats? */
	max = hsv.v;
	del = hsv.s * max / WHITE;
	min = max - del;
	hue = hsv.h;
	/* get region? */
	reg = (hue / HUE_DIFF); /* should be <HUE_PART */
	off = (hue - ((reg>>1) * HUE_COMP)); /* offset from HUE_COMP */
	if (off>HUE_DIFF) off = HUE_COMP-off;
	off = (off * del / HUE_DIFF); /* get diff for third value */
	chk = (off + min); /* third value */
	/* assign based on region */
	switch (reg)
	{
		case 0: /* max is r, chk is g, min is b! */
			rgb.r = max; rgb.g = chk; rgb.b = min; break;
		case 1: /* max is g, chk is r, min is b! */
			rgb.r = chk; rgb.g = max; rgb.b = min; break;
		case 2: /* max is g, chk is b, min is r! */
			rgb.r = min; rgb.g = max; rgb.b = chk; break;
		case 3: /* max is b, chk is g, min is r! */
			rgb.r = min; rgb.g = chk; rgb.b = max; break;
		case 4: /* max is b, chk is r, min is g! */
			rgb.r = chk; rgb.g = min; rgb.b = max; break;
		case 5: /* max is r, chk is b, min is g! */
			rgb.r = max; rgb.g = min; rgb.b = chk; break;
		default:
			rgb.r = 0; rgb.g = 0; rgb.b = 0; break;
	}
	/* done */
	return rgb;
}
/*----------------------------------------------------------------------------*/
