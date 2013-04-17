/*----------------------------------------------------------------------------*/
#include "my1imgbmp.h"
#include "my1imgutil.h"
#include <stdio.h> /** for file access */
/*----------------------------------------------------------------------------*/
int loadBMPimage(char *filename, my1Image *image)
{
	FILE *bmpfile;
	unsigned int tellSize;
	unsigned char *pChar, someChar;
	unsigned char bmpID[2];
	int row, col, r, g, b, temp, buff;
	int iscolor = IMASK_COLOR24; /* assumes 24-bit rgb by default */
	int palette[256]; /* 8-bit palette */
	my1BMPHead head;
	my1BMPInfo info;
	/* open file for read */
	bmpfile = fopen(filename, "rb");
	if(!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */
	/* get/check bitmap id */
	pChar = (unsigned char*) bmpID;
	fread(pChar, sizeof(bmpID), 1, bmpfile);
	if(bmpID[0]!='B'||bmpID[1]!='M')
		return BMP_ERROR_VALIDBMP; /* not a bmp format */
	/* get header */
	pChar = (unsigned char*) &head;
	fread(pChar, BMP_HEAD_SIZE, 1, bmpfile);
	/* get info */
	pChar = (unsigned char*) &info;
	fread(pChar, BMP_INFO_SIZE, 1, bmpfile);
#ifdef MY1DEBUG
	printf("\n");
	printf("--------------\n");
	printf("BMP DEBUG INFO\n");
	printf("--------------\n");
	printf("Sizeof my1BMPHead: %lu (%d)\n",sizeof(my1BMPHead),BMP_HEAD_SIZE);
	printf("Sizeof my1BMPInfo: %lu (%d)\n",sizeof(my1BMPInfo),BMP_INFO_SIZE);
	printf("Width: %d Height: %d\n", info.bmpWidth, info.bmpHeight);
	printf("File size: %u bytes\n", head.bmpSize);
	printf("Info size: %u bytes\n", info.bmpInfoSize);
	printf("Data size: %u bytes\n", info.bmpDataSize);
	printf("Data offset: %u bytes\n", head.bmpOffset);
	printf("Bits per pixel: %d, Colors: %d\n",
		info.bmpBitsPerPixel, info.bmpColorCount);
#endif
	/* get actual file size */
	fseek(bmpfile, 0, SEEK_END);
	tellSize = ftell(bmpfile);
	if(tellSize!=head.bmpSize)
		return BMP_ERROR_FILESIZE; /* mismatched filesize! */
	/* sanity check */
	if(info.bmpBitsPerPixel!=8&&info.bmpBitsPerPixel!=24)
		return BMP_ERROR_RGBNGRAY; /* only 24-bit RGB and 8-bit image */
	else if(createimage(image,info.bmpHeight,info.bmpWidth)==0x0)
		return BMP_ERROR_MEMALLOC; /* cannot allocate memory */
	/* check if palette is available */
	if(info.bmpColorCount==256) /* should be this for 8-bit per pixel */
	{
		/* load palette */
		fseek(bmpfile, BMP_HEAD_SIZE+BMP_INFO_SIZE, SEEK_SET);
		for(row=0;row<256;row++)
		{
			pChar = (unsigned char*) &palette[row];
			fread(pChar,sizeof(int),1,bmpfile);
		}
		/** iscolor = IMASK_GRAY8; */
	}
	else
	{
		/* ignore palette */
		info.bmpColorCount = 0;
		if(info.bmpBitsPerPixel!=24) iscolor = 0;
	}
	/* look for data! */
	fseek(bmpfile, head.bmpOffset, SEEK_SET);
	pChar = &someChar;
	/** my origin is topleft but bmp origin is bottomleft! */
	for(row=image->height-1; row>=0; row--)
	{
		temp = 0;
		for(col=0; col<image->width; col++)
		{
			if(info.bmpBitsPerPixel==24)
			{
				fread(pChar, 1, 1, bmpfile);
				b = (int) someChar & 0xFF;
				fread(pChar, 1, 1, bmpfile);
				g = (int) someChar & 0xFF;
				fread(pChar, 1, 1, bmpfile);
				r = (int) someChar & 0xFF;
				temp += 3;
			}
			else
			{
				fread(pChar, 1, 1, bmpfile);
				if(info.bmpColorCount) /* get from pallete? */
				{
					decode_rgb(palette[someChar],(char*)&r,
						(char*)&g, (char*)&b);
				}
				else
				{
					r = someChar; g = someChar; b = someChar;
				}
				temp++;
			}
			/* 'encode' if necessary! */
			if(iscolor) buff = encode_rgb(r,g,b);
			else buff = r; /** just take ANY component */
#ifdef MY1DEBUG
			if(col==0&&row==0)
			{
				printf("First pixel: {%d,%d,%d} => ",r,g,b);
				printf("{%02X,%02X,%02X} => {%08X}\n",r,g,b,buff);
			}
#endif
			setimagepixel(image,row,col,buff);
		}
		while(temp%4)
		{
			/* read in & ignore 'pad' values */
			fread(pChar, 1, 1, bmpfile);
			temp++;
		}
	}
	fclose(bmpfile);
	/* put on mask?? */
	image->mask = iscolor;
	return 0;
}
/*----------------------------------------------------------------------------*/
int saveBMPimage(char *filename, my1Image *image)
{
	FILE *bmpfile;
	unsigned int headSize, fileSize;
	unsigned int vectorSize;
	int row, col, temp, buff, length, bytepp = 1;
	unsigned char *pChar, someChar, r, g, b;
	unsigned char bmpID[2];
	my1BMPHead head;
	my1BMPInfo info;
	/* check if color image - palette NOT possible! */
	if(image->mask==IMASK_COLOR24) bytepp = 3;
	/* calculate filesize */
	length = image->width*bytepp;
	while(length%4) length++;
	vectorSize = length*image->height;
	headSize = BMP_HEAD_SIZE+BMP_INFO_SIZE; /* already includes 2-byte id! */
	fileSize = headSize + vectorSize;
	/* populate BMP header */
	bmpID[0] = 'B';
	bmpID[1] = 'M';
	head.bmpSize = fileSize;
	head.bmpReserved = 0;
	head.bmpOffset = headSize;
	/* populate BMP info */
	info.bmpInfoSize = BMP_INFO_SIZE;
	info.bmpWidth = image->width;
	info.bmpHeight = image->height;
	info.bmpPlaneCount = 1;
	info.bmpBitsPerPixel = bytepp*8;
	info.bmpCompression = 0;
	info.bmpDataSize = vectorSize;
	info.bmpHResolution = 0; /** not used? */
	info.bmpVResolution = 0; /** not used? */
	info.bmpColorCount = 0;
	info.bmpIColorCount = 0;
#ifdef MY1DEBUG
	printf("\n");
	printf("-------------------------\n");
	printf("BMP DEBUG INFO (CREATED!)\n");
	printf("-------------------------\n");
	printf("Sizeof my1BMPHead: %lu (%d)\n",sizeof(my1BMPHead),BMP_HEAD_SIZE);
	printf("Sizeof my1BMPInfo: %lu (%d)\n",sizeof(my1BMPInfo),BMP_INFO_SIZE);
	printf("Width: %d Height: %d\n", info.bmpWidth, info.bmpHeight);
	printf("File size: %u bytes\n", head.bmpSize);
	printf("Info size: %u bytes\n", info.bmpInfoSize);
	printf("Data size: %u bytes\n", info.bmpDataSize);
	printf("Data offset: %u bytes\n", head.bmpOffset);
	printf("Bits per pixel: %d, Colors: %d\n",
		info.bmpBitsPerPixel, info.bmpColorCount);
#endif
	/* try to open file for write! */
	bmpfile = fopen(filename,"wb");
	if(!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */
	/* write bitmap id */
	pChar = (unsigned char*) bmpID;
	fwrite(pChar, sizeof(bmpID), 1, bmpfile);
	/* write bitmap head */
	pChar = (unsigned char*) &head;
	fwrite(pChar,BMP_HEAD_SIZE,1,bmpfile);
	/* write bitmap info */
	pChar = (unsigned char*) &info;
	fwrite(pChar,BMP_INFO_SIZE,1,bmpfile);
	/* write data! */
	/** my origin is topleft but bmp origin is bottomleft! */
	pChar = (unsigned char*) &someChar;
	for(row=image->height-1;row>=0;row--)
	{
		temp = 0;
		for(col=0;col<image->width;col++)
		{
			buff = imagepixel(image,row,col);
			if(image->mask==IMASK_COLOR24)
			{
				decode_rgb(buff,(char*)&r,(char*)&g, (char*)&b);
				someChar = b;
				fwrite(pChar,1,1,bmpfile);
				someChar = g;
				fwrite(pChar,1,1,bmpfile);
				someChar = r;
				fwrite(pChar,1,1,bmpfile);
				temp += 3;
			}
			else
			{
				someChar = buff & 0xFF;
				fwrite(pChar,1,1,bmpfile);
				temp++;
			}
		}
		while(temp<length)
		{
			/* write 'pad' values */
			someChar = 0x00;
			fwrite(pChar, 1, 1, bmpfile);
			temp++;
		}
	}
	fclose(bmpfile);
	return 0;
}
/*----------------------------------------------------------------------------*/
