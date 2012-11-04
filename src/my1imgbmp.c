/*
	my1imgbmp => bmp file utility for my1imgpro
	Author: azman@my1matrix.net
*/

#include "my1imgbmp.h"
#include <stdio.h>

//#define MY1BMP_DEBUG

int get2red(int color)
{
	return (color>>16)&0xff;
}

int get2green(int color)
{
	return (color>>8)&0xff;
}

int get2blue(int color)
{
	return color&0xff;
}

int getrgb(int red, int green, int blue)
{
	return ((red&0xff)<<16)|((green&0xff)<<8)|(blue&0xff);
}

int loadBMPimage(char *filename, my1Image *image)
{
	FILE *bmpfile;
	unsigned int fileSize, tellSize, headerSize;
	/** unsigned int vectorSize; */
	unsigned char *pChar, someChar;
	int row, col, r, g, b, temp;
	int iscolor = (2^24)-1; /* assume 24-bits (rgb) color by default */
	int palette[256];
	char bmpID[2];
	my1BMPInfo info;

	bmpfile = fopen(filename, "rb");
	if(!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */

	/* get and check bitmap id */
	pChar = (unsigned char*) bmpID;
	fread(pChar, 1, 2, bmpfile);
	if(bmpID[0]!='B'||bmpID[1]!='M')
		return BMP_ERROR_VALIDBMP; /* not a bmp format */
	/* get BMP header */
	pChar = (unsigned char*) &info;
	fread(pChar, sizeof(my1BMPInfo), 1, bmpfile);
	fileSize = info.bmpSize;
	/* get actual file size */
	fseek(bmpfile, 0, SEEK_END);
	tellSize = ftell(bmpfile);
#ifdef MY1BMP_DEBUG
	printf("Size: %u\n", fileSize);
	printf("Size: %u\n", tellSize);
#endif
	if(fileSize!=tellSize)
		return BMP_ERROR_FILESIZE; /* mismatched filesize! */

	/* sanity check */
	if((info.bmpBitsPerPixel!=8||info.bmpColorCount!=256)&&
		info.bmpBitsPerPixel!=24)
		return BMP_ERROR_RGBNGRAY; /* only RGB and grayscale */
	else if(createimage(image,info.bmpHeight,info.bmpWidth)==0x0)
		return BMP_ERROR_MEMALLOC; /* cannot allocate memory */

	/* we're good to go! */
	headerSize = BMP_HEADER_SIZE+(4*info.bmpColorCount); /* header + palette */
	/** vectorSize = fileSize - headerSize; */

	if(info.bmpColorCount==256)
	{
		// load palette
		fseek(bmpfile, BMP_HEADER_SIZE, SEEK_SET);
		temp = 0; iscolor = 256;
		for(row=0;row<256;row++)
		{
			pChar = (unsigned char*) &palette[row];
			fread(pChar,sizeof(int),1,bmpfile);
			if(temp==row)
			{
				r = get2red(palette[row]);
				g = get2green(palette[row]);
				b = get2blue(palette[row]);
				if(r==g&&r==b&&g==b&&r==row)
					temp++;
			}
		}
		if(temp==256) iscolor = 0; /* actually a grayscale image! */
	}

#ifdef MY1BMP_DEBUG
	printf("Width: %d Height: %d\n", width, height);
	printf("File size: %u bytes\n", fileSize);
	printf("Vector Size: %u bytes\n", vectorSize);
	printf("Color Image? %s\n", iscolor>0?"Palette/RGB":"Grayscale.");
	printf("Bits per pixel: %d Colors: %d\n", nBitsPP, nColors);
#endif

	fseek(bmpfile, headerSize, SEEK_SET);
	// my origin is topleft but bmp origin is bottomleft!
	pChar = &someChar;
	for(row=image->height-1; row>=0; row--)
	{
		temp = 0;
		for(col=0; col<image->width; col++)
		{
			if(info.bmpBitsPerPixel==24)
			{
				fread(pChar, 1, 1, bmpfile);
				r = (int)someChar & 0xFF;
				fread(pChar, 1, 1, bmpfile);
				g = (int) someChar & 0xFF;
				fread(pChar, 1, 1, bmpfile);
				b = (int) someChar & 0xFF;
				someChar = ((r+g+b)/3) & 0xFF;
				temp += 3;
			}
			else
			{
				fread(pChar, 1, 1, bmpfile);
				r = get2red(palette[someChar]);
				g = get2green(palette[someChar]);
				b = get2blue(palette[someChar]);
				someChar = ((r+g+b)/3) & 0xFF;
				temp++;
			}
			setimagepixel(image,row,col,someChar);
		}
		while(temp%4)
		{
			// read in & ignore 'pad' values
			fread(pChar, 1, 1, bmpfile);
			temp++;
		}
	}
	fclose(bmpfile);

	return iscolor;
}

int saveBMPimage(char *filename, my1Image *image)
{
	FILE *bmpfile;
	unsigned long fileSize;
	unsigned long vectorSize;
	int row, col, length;
	char bmpID[]="BM";
	my1BMPInfo info;
	unsigned char *pChar, someChar;

	bmpfile = fopen(filename,"wb");
	if(!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */

	length = image->width;
	while(length%4) length++;
	vectorSize = length*image->height;
	fileSize = 0x436 + vectorSize; // header+pallette+data

	info.bmpSize = fileSize; // CALCULATE THIS!!
	info.bmpReserved = 0;
	info.bmpOffset = 0x436; // 54-byte header, no palette
	info.bmpInfoSize = 40; // BITMAPINFOHEADER?
	info.bmpWidth = image->width;
	info.bmpHeight = image->height;
	info.bmpPlaneCount = 1;
	info.bmpBitsPerPixel = 8; // grayscale image!!
	info.bmpCompression = 0;
	info.bmpDataSize = vectorSize; // ignored? height x width bytes
	info.bmpHResolution = 0; // not used?
	info.bmpVResolution = 0; // not used?
	info.bmpColorCount = 256; // only if using palette?
	info.bmpIColorCount = 256; // only if using palette?

	pChar = (unsigned char*) &bmpID[0];
	fwrite(pChar,1,1,bmpfile);
	pChar = (unsigned char*) &bmpID[1];
	fwrite(pChar,1,1,bmpfile);
	pChar = (unsigned char*) &info;
	fwrite(pChar,sizeof(my1BMPInfo),1,bmpfile);
	// write pallette
	col = 0x0;
	pChar = (unsigned char*) &col;
	for(row=0;row<256;row++)
	{
		fwrite(pChar,sizeof(col),1,bmpfile);
		col += 0x00010101;
	}
	// my origin is topleft!
	// bmp origin is bottomleft!
	pChar = (unsigned char*) &someChar;
	for(row=image->height-1;row>=0;row--)
	{
		for(col=0;col<image->width;col++)
		{
			someChar = imagepixel(image,row,col) & 0xFF;
			fwrite(pChar,1,1,bmpfile);
		}
		while(col<length)
		{
			// write 'pad' values
			someChar = 0x00;
			fwrite(pChar, 1, 1, bmpfile);
			col++;
		}
	}
	fclose(bmpfile);

	return 0;
}
