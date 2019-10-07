/*----------------------------------------------------------------------------*/
#include "my1image_file.h"
/*----------------------------------------------------------------------------*/
#include <stdio.h> /** for file access */
#include <string.h> /** image_write needs string functions */
/*----------------------------------------------------------------------------*/
#define BMP_ID_SIZE 2
#define BMP_HEAD_SIZE 12
/*----------------------------------------------------------------------------*/
typedef struct _my1image_bmp_head_t
{
	/* unsigned char bmpID[2]; / * excluded! 2 bytes */
	unsigned int bmpSize; /* 4 bytes */
	unsigned int bmpReserved; /* 4 bytes */
	unsigned int bmpOffset; /* 4 bytes */
}
my1image_bmp_head_t;
/*----------------------------------------------------------------------------*/
#define BMP_INFO_SIZE 40
#define BMP_INFO_SIZE_V4 108
#define BMP_INFO_SIZE_V5 124
/*----------------------------------------------------------------------------*/
typedef struct _my1image_bmp_info_t
{
	unsigned int bmpInfoSize; /* 4 bytes */
	unsigned int bmpWidth; /* 4 bytes */
	unsigned int bmpHeight; /* 4 bytes */
	unsigned short bmpPlaneCount; /* 2 bytes */
	unsigned short bmpBitsPerPixel; /* 2 bytes - 1, 4, 8, 24*/
	unsigned int bmpCompression; /* 4 bytes */
	unsigned int bmpDataSize; /* 4 bytes - rounded to next 4 bytesize */
	unsigned int bmpHResolution; /* 4 bytes - pix per meter */
	unsigned int bmpVResolution; /* 4 bytes - pix per meter */
	unsigned int bmpColorCount; /* 4 bytes */
	unsigned int bmpIColorCount; /* 4 bytes */
}
my1image_bmp_info_t;
/*----------------------------------------------------------------------------*/
#define GRAYLEVEL_COUNT 256
/*----------------------------------------------------------------------------*/
int image_load_bmp(my1image_t *image, char *filename)
{
	FILE *bmpfile;
	unsigned int tellSize, fileSize;
	unsigned char *pChar, someChar;
	unsigned char bmpID[BMP_ID_SIZE];
	int row, col, temp, buff;
	unsigned char r, g, b;
	int iscolor = IMASK_COLOR; /* assumes 24-bit rgb by default */
	int palette[GRAYLEVEL_COUNT]; /* 8-bit palette */
	my1image_bmp_head_t head;
	my1image_bmp_info_t info;
	/* open file for read */
	bmpfile = fopen(filename, "rb");
	if (!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */
	/* get/check bitmap id */
	pChar = (unsigned char*) bmpID;
	fread(pChar, sizeof(bmpID), 1, bmpfile);
	if (bmpID[0]!='B'||bmpID[1]!='M')
		return BMP_ERROR_VALIDBMP; /* not a bmp format */
	/* get header */
	pChar = (unsigned char*) &head;
	fread(pChar, BMP_HEAD_SIZE, 1, bmpfile);
	/* get info */
	pChar = (unsigned char*) &info;
	fread(pChar, BMP_INFO_SIZE, 1, bmpfile);
	/* calculate fileSize */
	fileSize = head.bmpSize; /** include 2 id bytes */
	/* get actual file size */
	fseek(bmpfile, 0, SEEK_END);
	tellSize = ftell(bmpfile);
#ifdef MY1DEBUG
	printf("\n");
	printf("--------------\n");
	printf("BMP DEBUG INFO\n");
	printf("--------------\n");
	printf("Sizeof my1image_bmp_head_t: %lu (%d)\n",
		(long unsigned)sizeof(my1image_bmp_head_t),BMP_HEAD_SIZE);
	printf("Sizeof my1image_bmp_info_t: %lu (%d/%d)\n",
		(long unsigned)sizeof(my1image_bmp_info_t),
		BMP_INFO_SIZE,BMP_INFO_SIZE_V4);
	printf("Width: %d Height: %d\n", info.bmpWidth, info.bmpHeight);
	printf("File size: %u bytes\n", head.bmpSize);
	printf("Info size: %u bytes\n", info.bmpInfoSize);
	printf("Data size: %u bytes\n", info.bmpDataSize);
	printf("Data offset: %u bytes\n", head.bmpOffset);
	printf("Bits per pixel: %d, Colors: %d, Compression: %d\n",
		info.bmpBitsPerPixel, info.bmpColorCount,info.bmpCompression);
	printf("File: %u bytes, Tell: %u bytes\n", fileSize, tellSize);
	printf("\n");
#endif
	/* sanity checks */
	if (tellSize!=fileSize)
		return BMP_ERROR_FILESIZE; /* mismatched filesize! */
	if (info.bmpBitsPerPixel!=8&&info.bmpBitsPerPixel!=24)
		return BMP_ERROR_RGBNGRAY; /* only 24-bit RGB and 8-bit image */
	if (image_make(image,info.bmpHeight,info.bmpWidth)==0x0)
		return BMP_ERROR_MEMALLOC; /* cannot allocate memory */
	switch (info.bmpInfoSize)
	{
		case BMP_INFO_SIZE:
		case BMP_INFO_SIZE_V4:
		case BMP_INFO_SIZE_V5:
			break;
		default:
			return BMP_ERROR_DIBINVAL; /* unsupported BMP header format? */
	}
	if (info.bmpCompression>0)
		return BMP_ERROR_COMPRESS; /* compression NOT supported! */
	/* check if palette is available */
	if (info.bmpColorCount==GRAYLEVEL_COUNT)
	{
		/* load palette */
		fseek(bmpfile, BMP_ID_SIZE+BMP_HEAD_SIZE+info.bmpInfoSize, SEEK_SET);
		for (row=0;row<GRAYLEVEL_COUNT;row++)
		{
			pChar = (unsigned char*) &palette[row];
			fread(pChar,sizeof(int),1,bmpfile);
		}
	}
	else
	{
		/* ignore palette */
		info.bmpColorCount = 0;
		if (info.bmpBitsPerPixel!=24) iscolor = 0;
	}
	/* look for data! */
	fseek (bmpfile, head.bmpOffset, SEEK_SET);
	pChar = &someChar;
	/** my origin is topleft but bmp origin is bottomleft! */
	for (row=image->height-1; row>=0; row--)
	{
		temp = 0;
		for (col=0; col<image->width; col++)
		{
			if (info.bmpBitsPerPixel==24)
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
				if (info.bmpColorCount) /* get from pallete? */
				{
					decode_rgb(palette[someChar],&r,&g,&b);
				}
				else
				{
					r = someChar; g = someChar; b = someChar;
				}
				temp++;
			}
			/* 'encode' if necessary! */
			if (iscolor) buff = encode_rgb(r,g,b);
			else buff = r; /** just take ANY component */
			image_set_pixel(image,row,col,buff);
		}
		while (temp%4)
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
int image_save_bmp(my1image_t *image, char *filename)
{
	FILE *bmpfile;
	unsigned int headSize, fileSize;
	unsigned int vectorSize, paletteSize = 0;
	int row, col, temp, buff, length, bytepp = 1;
	unsigned char *pChar, someChar, r, g, b;
	unsigned char bmpID[BMP_ID_SIZE];
	my1image_bmp_head_t head;
	my1image_bmp_info_t info;
	/* check if color image */
	if (image->mask==IMASK_COLOR)
	{
		bytepp = 3;
	}
	else
	{
		/* use palette for grayscale */
		paletteSize = sizeof(int)*GRAYLEVEL_COUNT;
	}
	/* calculate filesize */
	length = image->width*bytepp;
	while (length%4) length++;
	vectorSize = length*image->height;
	headSize = BMP_ID_SIZE+BMP_HEAD_SIZE+BMP_INFO_SIZE;
	fileSize = headSize + vectorSize + paletteSize;
	/* populate BMP header */
	bmpID[0] = 'B';
	bmpID[1] = 'M';
	head.bmpSize = fileSize;
	head.bmpReserved = 0;
	head.bmpOffset = headSize + paletteSize;
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
	info.bmpColorCount = paletteSize>0?GRAYLEVEL_COUNT:0;
	info.bmpIColorCount = 0;
#ifdef MY1DEBUG
	printf("\n");
	printf("-------------------------\n");
	printf("BMP DEBUG INFO (CREATED!)\n");
	printf("-------------------------\n");
	printf("Sizeof my1image_bmp_head_t: %lu (%d)\n",
		(long unsigned)sizeof(my1image_bmp_head_t),BMP_HEAD_SIZE);
	printf("Sizeof my1image_bmp_info_t: %lu (%d)\n",
		(long unsigned)sizeof(my1image_bmp_info_t),BMP_INFO_SIZE);
	printf("Width: %d Height: %d\n", info.bmpWidth, info.bmpHeight);
	printf("File size: %u bytes\n", head.bmpSize);
	printf("Info size: %u bytes\n", info.bmpInfoSize);
	printf("Data size: %u bytes\n", info.bmpDataSize);
	printf("Data offset: %u bytes\n", head.bmpOffset);
	printf("Bits per pixel: %d, Colors: %d\n",
		info.bmpBitsPerPixel, info.bmpColorCount);
	printf("Palette Size: %d bytes\n", paletteSize);
	printf("\n");
#endif
	/* try to open file for write! */
	bmpfile = fopen(filename,"wb");
	if (!bmpfile) return BMP_ERROR_FILEOPEN; /* cannot open file */
	/* write bitmap id */
	pChar = (unsigned char*) bmpID;
	fwrite(pChar, sizeof(bmpID), 1, bmpfile);
	/* write bitmap head */
	pChar = (unsigned char*) &head;
	fwrite(pChar,BMP_HEAD_SIZE,1,bmpfile);
	/* write bitmap info */
	pChar = (unsigned char*) &info;
	fwrite(pChar,BMP_INFO_SIZE,1,bmpfile);
	/* write palette if created */
	if (paletteSize>0)
	{
		pChar = (unsigned char*) &col;
		for (row=0;row<GRAYLEVEL_COUNT;row++)
		{
			col = gray2color(row);
			fwrite(pChar,sizeof(int),1,bmpfile);
		}
	}
	/* write data! */
	/** my origin is topleft but bmp origin is bottomleft! */
	pChar = (unsigned char*) &someChar;
	for (row=image->height-1;row>=0;row--)
	{
		temp = 0;
		for (col=0;col<image->width;col++)
		{
			buff = image_get_pixel(image,row,col);
			if (image->mask==IMASK_COLOR)
			{
				decode_rgb(buff,&r,&g,&b);
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
		while (temp<length)
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
#define PNM_MAGIC_CHAR 2
#define PNM_MAGIC_SIZE (PNM_MAGIC_CHAR+1)
/*----------------------------------------------------------------------------*/
int image_load_pnm(my1image_t *image, char *filename)
{
	FILE *pnmfile;
	char buffer[PNM_MAGIC_SIZE];
	int r, g, b, buff, loop, size;
	int width, height, maxval = 1, error = 0, version = 0;
	/* open file for read */
	pnmfile = fopen(filename, "rt");
	if (!pnmfile) return PNM_ERROR_FILEOPEN; /* cannot open file */
	/* get and check magic number - will read PNM_MAGIC_SIZE-1 char */
	fgets(buffer, PNM_MAGIC_SIZE, pnmfile);
	if (buffer[0]=='P'&&(buffer[1]>0x30&&buffer[1]<0x37))
	{
		version = (int)buffer[1] - 0x30; /** P1 - P6 */
		if (version>3)
		{
			fclose(pnmfile);
			return PNM_ERROR_NOSUPPORT; /* not supported */
		}
	}
	if (!version)
	{
		fclose(pnmfile);
		return PNM_ERROR_VALIDPNM; /* not a pnm format */
	}
	/** get to end of magic number line */
	while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
	/** skip any comment lines */
	do {
		buff = fgetc(pnmfile);
		if (buff=='#') while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
		else ungetc(buff,pnmfile);
	} while (buff=='#');
	/** get width and height */
	fscanf(pnmfile,"%d %d",&width,&height);
	/** maxval for version 2/3 only! */
	if (version>1) fscanf(pnmfile,"%d",&maxval);
#ifdef MY1DEBUG
	printf("Image format: P%d, maxval: %d\n",version,maxval);
	printf("Image width: %d, height: %d\n",width,height);
#endif
	/** try to create storage */
	if (image_make(image,height,width)==0x0)
	{
		fclose(pnmfile);
		return PNM_ERROR_MEMALLOC; /* cannot allocate memory */
	}
	/** skip any comment lines */
	do {
		buff = fgetc(pnmfile);
		if (buff=='#') while (!feof(pnmfile)&&fgetc(pnmfile)!='\n');
		else ungetc(buff,pnmfile);
	} while (buff=='#');
	/** read in the pixels - depending on version (validated as 1-3)! */
	size = image->length;
	switch (version)
	{
		case 3:
		{
			for (loop=0;loop<size;loop++)
			{
				if (fscanf(pnmfile,"%d %d %d",&r,&g,&b)==EOF)
				{
					error = PNM_ERROR_FILESIZE;
					break;
				}
				if ((r<0||r>maxval)||(g<0||g>maxval)||(b<0||b>maxval))
				{
					error = PNM_ERROR_LEVELPNM;
					break;
				}
				buff = encode_rgb(r,g,b);
				image->data[loop] = buff;
			}
			break;
		}
		case 2:
		{
			for (loop=0;loop<size;loop++)
			{
				if (fscanf(pnmfile,"%d",&buff)==EOF)
				{
					error = PNM_ERROR_FILESIZE;
					break;
				}
				if (buff<0||buff>maxval)
				{
					error = PNM_ERROR_LEVELPNM;
					break;
				}
				image->data[loop] = buff;
			}
			break;
		}
		case 1:
		{
			for (loop=0;loop<size;loop++)
			{
				if (feof(pnmfile))
				{
					error = PNM_ERROR_FILESIZE;
					break;
				}
				buff = fgetc(pnmfile);
				switch (buff)
				{
					/* PBM is 'ink on' coded => 1 is BLACK! */
					case '1': buff = BLACK; break;
					case '0': buff = WHITE; break;
					default: buff = -1; break;
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						continue;
				}
				if (buff<0)
				{
					error = PNM_ERROR_LEVELPNM;
					break;
				}
				image->data[loop] = buff;
			}
			break;
		}
	}
	fclose(pnmfile);
	if (!error)
		image->mask = (version==3) ? IMASK_COLOR : IMASK_GRAY;
	return error;
}
/*----------------------------------------------------------------------------*/
int image_save_pnm(my1image_t *image, char *filename)
{
	FILE *pnmfile;
	char buffer[PNM_MAGIC_SIZE];
	unsigned char r, g, b;
	int loop, next, buff, version = 2;
	/* check if color image */
	if (image->mask==IMASK_COLOR) version = 3;
	/* try to open file for write! */
	pnmfile = fopen(filename,"wt");
	if (!pnmfile) return PNM_ERROR_FILEOPEN; /* cannot open file */
	/* write magic number */
	buffer[0] = 'P'; buffer[1] = (char)(0x30+version); buffer[2] = 0x0;
	fprintf(pnmfile,"%s\n",buffer);
	/* write comment? */
	fprintf(pnmfile,"# Written by my1imgpro library\n");
	/* write size */
	fprintf(pnmfile,"%d %d\n",image->width,image->height);
	/* write max value - ALWAYS 8-bit {gray,color} max value */
	fprintf(pnmfile,"255\n");
#ifdef MY1DEBUG
	printf("Version: %d, ",version);
	printf("Image width: %d, height: %d\n",image->width,image->height);
#endif
	/* write data! */
	for (loop=0,next=0;loop<image->length;loop++)
	{
		buff = image->data[loop];
		if (image->mask==IMASK_COLOR)
		{
			decode_rgb(buff,&r,&g,&b);
			fprintf(pnmfile,"%d %d %d ",r,g,b);
		}
		else
		{
			fprintf(pnmfile,"%d ",buff);
		}
		/* newline for every row */
		if (++next==image->width)
		{
			next = 0;
			fprintf(pnmfile,"\n");
		}
	}
	fclose(pnmfile);
	return 0;
}
/*----------------------------------------------------------------------------*/
int image_load(my1image_t* image, char *pfilename)
{
	int test, flag = 0;
	do
	{
		if (!(test=image_load_bmp(image,pfilename))) { flag = 0; break; }
		flag += test;
		if (test&FILE_ERROR_OPEN) break; /* only need to check once? */
		if (test!=BMP_ERROR_VALIDBMP) break; /* continue if NOT format error */
		if (!(test=image_load_pnm(image,pfilename))) { flag = 0; break; }
		flag += test;
		if (test!=PNM_ERROR_VALIDPNM) break;
	}
	while (0);
	return flag;
}
/*----------------------------------------------------------------------------*/
int image_save(my1image_t* image, char *pfilename)
{
	int flag, size;
	char *ptest;
	size = strlen(pfilename);
	ptest = &pfilename[size-4];
	if (strcmp(ptest,".bmp")==0)
		flag = image_save_bmp(image,pfilename);
	else /* default is pnm! */
		flag = image_save_pnm(image,pfilename);
	return flag;
}
/*----------------------------------------------------------------------------*/
int image_cdat(my1image_t* image, char *pfilename)
{
	int loop;
	FILE *cfile = fopen(pfilename,"wt");
	if(!cfile) return FILE_ERROR_OPEN; /* cannot open file */
	fprintf(cfile,"unsigned char image[%d] = {",image->length);
	/* write data! */
	for (loop=0;loop<image->length;loop++)
	{
		if (loop%16==0)
			fprintf(cfile,"\n");
		fprintf(cfile,"0x%02X",image->data[loop]);
		if (loop<image->length-1)
			fprintf(cfile,",");
	}
	fprintf(cfile,"};");
	fclose(cfile);
	return 0;
}
/*----------------------------------------------------------------------------*/
