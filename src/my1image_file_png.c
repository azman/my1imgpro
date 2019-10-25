/*----------------------------------------------------------------------------*/
#include "my1image_file_png.h"
#include "my1image_crgb.h"
/*----------------------------------------------------------------------------*/
#include <string.h> /** image_write needs string functions */
#include <stdlib.h>
/*----------------------------------------------------------------------------*/
my1imgfmt_t ipng = {PNG_ERROR_FLAG,"png",image_load_png,0x0,0x0};
/*----------------------------------------------------------------------------*/
typedef unsigned char b08_t;
typedef unsigned short w16_t;
typedef unsigned int w32_t;
/*----------------------------------------------------------------------------*/
/** huffman table generation requirements (reqs) */
#define HUFF_KO_NO_SYMBOLS 0
#define HUFF_OK_NO_SYMBOLS 1
/**---------------------------------------------------------------------------*/
/**
 *  PNG-related codes are based on public domain code found at
 *  http://achurch.org/tinflate.c
 *  - decompression state taken out (my code always provide whole input)
**/
/**---------------------------------------------------------------------------*/
int png_make_huffman(w32_t scnt, b08_t *lens, int reqs, short *tabs)
{
	w16_t lcnt[16];
	w16_t tcnt;
	w16_t init[16];
	w32_t sidx, nidx;
	w32_t loop, iter;
	w32_t clim, next;
	/* initialize counts */
	for (loop=0;loop<16;loop++)
		lcnt[loop] = 0;
	/* update counts */
	for (loop=0;loop<scnt;loop++)
		if (lens[loop]>0)
			lcnt[lens[loop]]++;
	/* check size to generate? */
	tcnt = 0;
	for (loop=1;loop<16;loop++)
		tcnt += lcnt[loop];
	/* resolve some... */
	if (tcnt == 0)
		return reqs;
	else if (tcnt == 1)
	{
		for (loop=0;loop<scnt;loop++)
			if (lens[loop] != 0)
				tabs[0] = tabs[1] = loop;
		return 1;
	}
	/* encoding */
	init[0] = 0;
	for (loop=1;loop<16;loop++)
	{
		init[loop] = (init[loop-1]+lcnt[loop-1]) << 1;
		if ((init[loop]+lcnt[loop]) > (w16_t)1<<loop)
			return 0;
	}
	if ((init[15]+lcnt[15])!=(w16_t)1<<15)
		return 0;
	sidx = 0;
	for (loop=1;loop<16;loop++)
	{
		clim = 1U <<loop;
		next = init[loop] + lcnt[loop];
		nidx = sidx + (clim - init[loop]);
		for (iter=0;iter<scnt;iter++)
		{
			if (lens[iter]==loop)
				tabs[sidx++] = iter;
		}
		for (iter=next;iter<clim;iter++)
		{
			tabs[sidx++] = ~nidx;
			nidx += 2;
		}
	}
	return 1;
}
/**---------------------------------------------------------------------------*/
void png_get_bits(b08_t** buff, w32_t *bits, w32_t *bacc,
	w32_t *temp, w32_t size)
{
	while (*bits<size)
	{
		/* assume never run out of data */
		(*bacc) |= ((w32_t)**buff) << *bits;
		(*bits) += 8;
		(*buff)++;
	}
	/* mask only bits we need */
	(*temp) = (*bacc) & ((1UL<<size)-1);
	/* prepare for next */
	(*bacc) >>= size;
	(*bits) -= size;
	return;
}
/**---------------------------------------------------------------------------*/
void png_put_byte_safe(b08_t* obuf, w32_t *ocnt, b08_t byte)
{
	obuf[*ocnt] = byte;
	(*ocnt)++;
}
/**---------------------------------------------------------------------------*/
void png_put_byte(b08_t* obuf, w32_t *ocnt, w32_t omax, b08_t byte)
{
	if (*ocnt<omax)
		obuf[*ocnt] = byte;
	(*ocnt)++;
}
/**---------------------------------------------------------------------------*/
void png_get_huff(b08_t** buff, w32_t *bits, w32_t *bacc,
	w32_t *temp, short* tabs)
{
	w32_t bits_used = 0;
	w32_t index = 0;
	while (1)
	{
		/* assume never run out of data */
		if (*bits<=bits_used)
		{
			(*bacc) |= ((w32_t)**buff) << *bits;
			(*bits) += 8;
			(*buff)++;
		}
		index += ((*bacc)>>bits_used)&1;
		bits_used++;
		if (tabs[index]>=0)
			break;
		index = ~tabs[index];
	}
	(*bacc) >>= bits_used;
	(*bits) -= bits_used;
	(*temp) = tabs[index];
}
/**---------------------------------------------------------------------------*/
w32_t png_inflate(void *idat, w32_t idat_size, void *obuf, w32_t obuf_size)
{
	w32_t obuf_step;
	b08_t *idat_init;
	b08_t *obuf_init;
	w32_t done, temp, loop, prev, buff, peat, dist;
	w32_t bits_made;
	b08_t bits_step;
	b08_t blok_type;
	w32_t blok_size;
	/* huffman tables... */
	short litt_tabs[288*2-2];
	short dist_tabs[32*2-2];
	short clen_tabs[19*2-2];
	w32_t litt_count;
	w32_t dist_count;
	w32_t clen_count;
	b08_t litt_len[288], dist_len[32], clen_len[19];
	/* initialize stuffs */
	idat_init = (b08_t*)idat;
	obuf_init = (b08_t*)obuf;
	obuf_step = 0;
	done = 0;
	bits_made = 0;
	bits_step = 0;
	/* check zlib header? */
	temp = idat_init[0]<<8 | idat_init[1];
	if ((temp & 0x8F00) == 0x0800 && temp % 31 == 0)
	{
		if (temp & 0x0020) return -1;
		idat_init += 2;
	}
	/* process blocks */
	do
	{
		b08_t *curr = idat_init;
		w32_t bacc = bits_made;
		w32_t bnum = bits_step;
		/* get block type */
		png_get_bits(&curr,&bnum,&bacc,(w32_t*)&blok_type,3);
		done = blok_type & 1;
		blok_type >>= 1;
		if (blok_type == 3) return -1; /* not supported? find this! */
		if (blok_type == 0)
		{
			bnum = 0;  /* dump any previous bits - was part of type? */
			png_get_bits(&curr,&bnum,&bacc,(w32_t*)&blok_size,16);
			png_get_bits(&curr,&bnum,&bacc,(w32_t*)&temp,16);
			/* temp is bit-inverted value of blok_size */
			if (temp != (~blok_size & 0xFFFF))
				return -1;
			/* directly transfer bytes based on given size */
			temp = 0;
			while (temp < blok_size)
			{
				png_put_byte(obuf_init,&obuf_step,obuf_size,*curr++);
				temp++;
			}
			/* Update the state buffer and return success. */
			idat_init = curr;
			bits_made = bacc;
			bits_step = bnum;
			continue;
		}
		if (blok_type == 2)
		{
			/* dynamic tabs */
			static const b08_t clen_sequ[19] =
			{ 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
			/* get counts */
			png_get_bits(&curr,&bnum,&bacc,&litt_count,5);
			litt_count += 257;
			png_get_bits(&curr,&bnum,&bacc,&dist_count,5);
			dist_count += 1;
			png_get_bits(&curr,&bnum,&bacc,&clen_count,4);
			clen_count += 4;
			/* loop through code lengths? */
			loop = 0;
			while (loop < clen_count)
			{
				png_get_bits(&curr,&bnum,&bacc,&temp,3);
				clen_len[clen_sequ[loop]] = temp;
				loop++;
			}
			/* reset the unused */
			for (; loop < 19; loop++)
				clen_len[clen_sequ[loop]] = 0;
			/* make huffman table! */
			if (!png_make_huffman(19, clen_len, 0,clen_tabs))
				return -1;
			/* resolve code lengths */
			prev = 0; temp = 0; loop = 0;
			while (loop < litt_count+dist_count)
			{
				if (temp == 0)
				{
					png_get_huff(&curr,&bnum,&bacc,&buff,clen_tabs);
					if (buff < 16)
					{
						prev = buff;
						temp = 1;
					}
					else if (buff == 16)
					{
						png_get_bits(&curr,&bnum,&bacc,&temp,2);
						temp += 3;
					}
					else if (buff == 17)
					{
						prev = 0;
						png_get_bits(&curr,&bnum,&bacc,&temp,3);
						temp += 3;
					}
					else /* buff == 18 */
					{
						prev = 0;
						png_get_bits(&curr,&bnum,&bacc,&temp,7);
						temp += 11;
					}
				}
				if (loop < litt_count)
					litt_len[loop] = prev;
				else
					dist_len[loop-litt_count] = prev;
				loop++;
				temp--;
			}
			/* now, make huffman for literal & distance */
			if (!png_make_huffman(litt_count, litt_len, 0,litt_tabs) ||
					!png_make_huffman(dist_count, dist_len, 1,dist_tabs))
				return -1;
		}
		else
		{
			/* static tabs - literals */
			for (loop=0;loop<0x7E;loop++)
			{
				litt_tabs[loop] = ~temp;
				temp += 2;
			}
			for (;loop<0x96;loop++)
				litt_tabs[loop] = (short)loop + (256 - 0x7E);
			for (;loop<0xFE;loop++)
			{
				litt_tabs[loop] = ~temp;
				temp += 2;
			}
			for (;loop<0x18E;loop++)
				litt_tabs[loop] = (short)loop + (0 - 0xFE);
			for (;loop<0x196;loop++)
				litt_tabs[loop] = (short)loop + (280 - 0x18E);
			for (;loop<0x1CE;loop++)
			{
				litt_tabs[loop] = ~temp;
				temp += 2;
			}
			for (;loop<0x23E;loop++)
				litt_tabs[loop] = (short)loop + (144 - 0x1CE);
			/* distance */
			for (loop=0;loop<0x1E;loop++)
				dist_tabs[loop] = ~(loop*2+2);
			for (loop=0x1E;loop<0x3E;loop++)
				dist_tabs[loop] = (short)loop - 0x1E;
		}
		for (;;)
		{
			if ((long)obuf_step < 0) /* is this possible? */
				return -1;
			png_get_huff(&curr,&bnum,&bacc,&buff,litt_tabs);
			if (buff < 256)
			{
				png_put_byte(obuf_init,&obuf_step,obuf_size,buff);
				continue;
			}
			if (buff == 256)
				break;
			if (buff <= 264)
				peat = (buff-257) + 3;
			else if (buff <= 284)
			{
				temp = (buff-261) / 4;
				png_get_bits(&curr,&bnum,&bacc,&peat,temp);
				peat += 3 + ((4 + ((buff-265) & 3)) << temp);
			}
			else if (buff == 285)
				peat = 258;
			else /* Invalid symbol. */
				return -1;
			png_get_huff(&curr,&bnum,&bacc,&buff,dist_tabs);
			if (buff <= 3)
				dist = buff + 1;
			else if (buff <= 29)
			{
				temp = (buff-2)/2;
				png_get_bits(&curr,&bnum,&bacc,&dist,temp);
				dist += 1 + ((2 + (buff & 1)) << temp);
			}
			else /* invalid */
				return -1;
			if (obuf_step < dist)
				return -1;
			loop = peat;
			temp = 0;
			if ((obuf_step+loop) > obuf_size)
			{
				if (obuf_step > obuf_size)
					temp = loop;
				else
					temp = (obuf_step-obuf_size) + loop;
				loop -= temp;
			}
			for (;loop>0;loop--)
			{
				png_put_byte_safe(obuf_init,&obuf_step,
					obuf_init[obuf_step-dist]);
			}
			obuf_step += temp;
		}  /* End of decompression loop. */
		idat_init = curr;
		bits_made = bacc;
		bits_step = bnum;
	}
	while (!done);
	return (long)obuf_step;
}
/**---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
#define PNG_HEAD_SIZE 8
#define PNG_HEAD_DATA { 0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a }
/* png color type masks */
#define PNG_CTM_EMPTY 0x00
#define PNG_CTM_PALET 0x01
#define PNG_CTM_COLOR 0x02
#define PNG_CTM_ALPHA 0x04
/* png color types */
#define PNG_CTYPE_GRAYSCALE (PNG_CTM_EMPTY)
#define PNG_CTYPE_TRUECOLOR (PNG_CTM_COLOR)
#define PNG_CTYPE_RGB PNG_CTYPE_TRUECOLOR
#define PNG_CTYPE_INDEXED (PNG_CTM_COLOR|PNG_CTM_PALET)
#define PNG_CTYPE_GRAYALPHA (PNG_CTM_ALPHA)
#define PNG_CTYPE_TRUEALPHA (PNG_CTM_COLOR|PNG_CTM_ALPHA)
#define PNG_CTYPE_RGBA PNG_CTYPE_TRUEALPHA
/* compression info:7 (32kwin), compression method:8 (deflate) */
#define PNG_ZLIB_CHK1 0x78
/* flag dictionary - not used? */
#define PNG_ZLIB_FLAG_DICT 0x20
/* compression levels 4 levels */
#define PNG_ZLIB_MASK_CLVL 0xC0
/* compression lvl 1 */
#define PNG_ZLIB_LVL1 0x01
/* compression lvl 2-5 */
#define PNG_ZLIB_LVL5 0x5e
/* compression lvl 6 */
#define PNG_ZLIB_LVL6 0x9c
/* compression lvl 7-9 */
#define PNG_ZLIB_LVL9 0xda
/*----------------------------------------------------------------------------*/
typedef struct _my1image_png_chunk_t
{
	w32_t pngSize; /* 4 bytes */
	w32_t pngCRC_; /* 4 bytes */
	char pngType[4]; /* 4 bytes */
	b08_t* pngData;
}
my1image_png_chunk_t;
/*----------------------------------------------------------------------------*/
typedef struct _my1image_png_t
{
	int height, width, depth, color, compress, filter, interlace;
	int cols, maxclr, channel;
	int pnum, plen; /* number of pallette entries , pallette length */
	b08_t *plte; /* pallette data */
	int dcnt, dprv, dlen; /* idat count, prev idat, pdat len */
	b08_t *pdat; /* pixel data */
}
my1image_png_t;
/*----------------------------------------------------------------------------*/
void pchunk_init(my1image_png_chunk_t* chunk)
{
	chunk->pngSize = 0; /* big-endian! */
	chunk->pngType[0] = 0x0;
	chunk->pngCRC_ = 0;
	chunk->pngData = 0x0;
}
/*----------------------------------------------------------------------------*/
void pchunk_make(my1image_png_chunk_t* chunk)
{
	if (chunk->pngData) free((void*)chunk->pngData);
	chunk->pngData = (b08_t*) malloc(chunk->pngSize);
}
/*----------------------------------------------------------------------------*/
void pchunk_free(my1image_png_chunk_t* chunk)
{
	if (chunk->pngData) free((void*)chunk->pngData);
	chunk->pngData = 0x0;
}
/*----------------------------------------------------------------------------*/
w32_t change_order(w32_t ebig)
{
	int loop;
	w32_t elit = 0;
	for (loop=0;loop<4;loop++)
	{
		elit <<= 8;
		elit |= ebig & 0xff;
		ebig >>= 8;
	}
	return elit;
}
/*----------------------------------------------------------------------------*/
#define CRC32_POLY_REVERSE 0xEDB88320
/*----------------------------------------------------------------------------*/
w32_t crc_byte_reflected(w32_t ccrc, w32_t data)
{
	int loop;
	/* reflected! */
	ccrc = ccrc ^ data;
	/* for every data bit! */
	for (loop=0;loop<8;loop++)
	{
		if (ccrc&0x01) ccrc = (ccrc >> 1) ^ CRC32_POLY_REVERSE;
		else ccrc = ccrc >> 1;
	}
	return ccrc;
}
/*----------------------------------------------------------------------------*/
int pchunk_load(my1image_png_chunk_t* chunk, FILE* pfile)
{
	w32_t temp, crcr;
	/* read chunk size */
	int test = fread((void*)&temp,4,1,pfile);
	if (test!=1) return -1;
	/* change endian */
	chunk->pngSize = change_order(temp);
	/* read type */
	test = fread((void*)chunk->pngType,1,4,pfile);
	if (test!=4) return -2;
	/* prepare data storage */
	pchunk_make(chunk);
	/* read chunk data */
	test = fread((void*)chunk->pngData,1,chunk->pngSize,pfile);
	if (test!=chunk->pngSize) return -3;
	/* read chunk crc32 */
	test = fread((void*)&temp,4,1,pfile);
	if (test!=1) return -3;
	/* change endian */
	chunk->pngCRC_ = change_order(temp);
	/* check crc */
	crcr = ~0U;
	for (temp=0;temp<4;temp++)
		crcr = crc_byte_reflected(crcr,chunk->pngType[temp]&0xff);
	for (temp=0;temp<chunk->pngSize;temp++)
		crcr = crc_byte_reflected(crcr,chunk->pngData[temp]&0xff);
	crcr = ~crcr;
	if (crcr!=chunk->pngCRC_) return -4;
	return 0;
}
/*----------------------------------------------------------------------------*/
b08_t png_paeth_predict(b08_t pLL, b08_t pTT, b08_t pTL)
{
	int tmpa, tmpb, tmpc, temp = (int) pLL + pTT - pTL;
	tmpa = temp - pLL; if (tmpa<0) tmpa = -tmpa;
	tmpb = temp - pTT; if (tmpb<0) tmpb = -tmpb;
	tmpc = temp - pTL; if (tmpc<0) tmpc = -tmpc;
	if (tmpa<=tmpb&&tmpa<=tmpc) return pLL;
	else if (tmpb<=tmpc) return pTT;
	else return pTL;
}
/*----------------------------------------------------------------------------*/
int png_make_image(my1image_png_t* pngd, my1image_t* image)
{
	b08_t dobuff[2], *dotemp, *doplte, *idat, *ibuf, *iprv;
	b08_t diff, doll, dott, dotl;
	w32_t *that, chka, chkb, chkc;
	long ires, ichk, ilen;
	int loop, size, type, argb, test = 0;
	my1rgb_t *rgbd = (my1rgb_t*)&argb;
	/* prepare for inflate - consider prefixed filter type byte */
	ilen = pngd->height*pngd->width*(sizeof(int)+1);
	idat = (b08_t*)malloc(ilen);
	ichk = pngd->dlen;
#ifdef MY1DEBUG
	printf("-- inflating... ");
	fflush(stdout);
#endif
	/* inflate data */
	ires = png_inflate(pngd->pdat,ichk,idat,ilen);

#ifdef MY1DEBUG
	printf("done (%ld/%ld)\n.",ires,ilen);
	fflush(stdout);
#endif

	if (ires<ilen)
	{
		do /* dummy loop */
		{
			/* calc adler32-sum */
			chka = 1; chkb = 0;
			for (loop=0;loop<ires;loop++)
			{
				chkc = idat[loop]&0xff;
				chka = (chka+chkc)%65521;
				chkb = (chka+chkb)%65521;
			}
			chkc = 65536*chkb + chka;
			that = (w32_t*) &pngd->pdat[pngd->dlen-4];
			chka = change_order(*that);
			/* checks */
			if (chka!=chkc||ires%pngd->height)
			{
				test = PNG_ERROR_ZLIBCOMP;
				break;
			}
			pngd->cols = ires/pngd->height;
			pngd->maxclr = 1 << pngd->depth;
			/* calc line pixels... reuse filter var */
			/* pngd->depth ensured as 1,2,4,8 */
			pngd->filter = (pngd->cols-1)*8/pngd->depth;
			pngd->filter /= pngd->channel;
			if (pngd->filter<pngd->width)
			{
				test = PNG_ERROR_ZLIBCOMP;
				break;
			}
			/* create image - always in color? */
			image_make(image,pngd->height,pngd->width);
			image->mask = IMASK_COLOR;
			/* prepare line filter stuffs */
			size = pngd->cols-1;
			iprv = 0x0;
#ifdef MY1DEBUG
			printf("-- CHECK => cols:%d [%d]\n",pngd->cols,pngd->height);
			fflush(stdout);
#endif
			/* fill in image */
			for (chka=0;chka<pngd->height;chka++)
			{
				/* for each line */
				dotemp = &idat[chka*pngd->cols];
				type = (int)dotemp[0];
				dotemp++; /* skip type */
				ibuf = dotemp;
				switch (type)
				{
					case 4: /* paeth */
						for (loop=0;loop<size;loop++)
						{
							if (loop<pngd->channel)
							{
								/* both lefties are 0! */
								doll = 0;
								dotl = 0;
							}
							else
							{
								argb = loop-pngd->channel;
								doll = ibuf[argb];
								dotl = iprv?iprv[argb]:0;
							}
							dott = iprv?iprv[loop]:0;
							diff = png_paeth_predict(doll,dott,dotl);
							ibuf[loop] += diff;
						}
						break;
					case 3: /* average */
						for (loop=0;loop<size;loop++)
						{
							if (loop<pngd->channel) argb = 0;
							else argb = (int)ibuf[loop-pngd->channel];
							if (iprv) argb += (int)iprv[loop];
							argb /= 2;
							ibuf[loop] += (b08_t)argb;
						}
						break;
					case 2: /* up */
						for (loop=0;loop<size;loop++)
						{
							if (!iprv) diff = 0;
							else diff = iprv[loop];
							ibuf[loop] += diff;
						}
						break;
					case 1: /* sub */
						for (loop=0;loop<size;loop++)
						{
							if (loop<pngd->channel) diff = 0;
							else diff = ibuf[loop-pngd->channel];
							ibuf[loop] += diff;
						}
						break;
					case 0: /* none - unmodified */
					default:
						break;
				}
				iprv = ibuf;
				chkb = 0;
				while (chkb<pngd->width)
				{
					if (pngd->depth==8)
					{
						if (pngd->color==PNG_CTYPE_INDEXED)
						{
							doplte = (b08_t*)&pngd->plte[(*dotemp)*3];
							rgbd->r = doplte[0];
							rgbd->g = doplte[1];
							rgbd->b = doplte[2];
							rgbd->a = 0;
						}
						else if (pngd->color&PNG_CTM_COLOR) /**PNG_CTYPE_RGB*/
						{
							rgbd->r = *dotemp++;
							rgbd->g = *dotemp++;
							rgbd->b = *dotemp;
							if (pngd->color&PNG_CTM_ALPHA) /**PNG_CTYPE_RGBA*/
								rgbd->a = *(++dotemp);
							else
								rgbd->a = 0;
						}
						else
						{
							rgbd->r = *dotemp;
							rgbd->g = *dotemp;
							rgbd->b = *dotemp;
							rgbd->a = 0;
						}
						image_set_pixel(image,chka,chkb,argb);
					}
					else if (pngd->depth==4)
					{
						dobuff[0] = ((*dotemp)&0xf0)>>4;
						dobuff[1] = ((*dotemp)&0x0f);
						if (pngd->color==PNG_CTYPE_INDEXED)
						{
							doplte = (b08_t*)&pngd->plte[dobuff[0]*3];
							rgbd->r = doplte[0];
							rgbd->g = doplte[1];
							rgbd->b = doplte[2];
						}
						else
						{
							rgbd->r = dobuff[0];
							rgbd->g = dobuff[0];
							rgbd->b = dobuff[0];
						}
						rgbd->a = 0;
						image_set_pixel(image,chka,chkb,argb);
						chkb++;
						if (pngd->color==PNG_CTYPE_INDEXED)
						{
							doplte = (b08_t*)&pngd->plte[dobuff[1]*3];
							rgbd->r = doplte[0];
							rgbd->g = doplte[1];
							rgbd->b = doplte[2];
						}
						else
						{
							rgbd->r = dobuff[1];
							rgbd->g = dobuff[1];
							rgbd->b = dobuff[1];
						}
						rgbd->a = 0;
						image_set_pixel(image,chka,chkb,argb);
					}
					chkb++;
					dotemp++;
				}
			}
		}
		while(0); /* end dummy loop */
	}
	free((void*)idat);
	return test;
}
/*----------------------------------------------------------------------------*/
int image_load_png(my1image_t *image, FILE* pngfile)
{
	int loop, test, step;
	my1image_png_chunk_t buff;
	my1image_png_t temp;
	w32_t *that, dosize;
	b08_t dobuff[PNG_HEAD_SIZE], *dotemp;
	b08_t idbuff[PNG_HEAD_SIZE] = PNG_HEAD_DATA;
	/* get and check png header */
	fread((void*)dobuff,PNG_HEAD_SIZE,1,pngfile);
	for (loop=0;loop<PNG_HEAD_SIZE;loop++)
	{
		if (dobuff[loop]!=idbuff[loop])
			return FILE_NOT_FORMAT; /* not a png format */
	}
	/* get header chunk */
	pchunk_init(&buff);
	test = pchunk_load(&buff,pngfile);
	if (test||strncasecmp(buff.pngType,"IHDR",4)||buff.pngSize!=13)
	{
		/* first chunk MUST be IHDR? */
		pchunk_free(&buff);
		return PNG_ERROR_NOHEADER;
	}
	/* read header! */
	that = (w32_t*) &buff.pngData[0];
	temp.width = (int) change_order(*that);
	that = (w32_t*) &buff.pngData[4];
	temp.height = (int) change_order(*that);
	temp.depth = (w32_t) buff.pngData[8];
	temp.color = (w32_t) buff.pngData[9];
	temp.compress = (w32_t) buff.pngData[10];
	temp.filter = (w32_t) buff.pngData[11];
	temp.interlace = (w32_t) buff.pngData[12];
	temp.plte = 0x0;
	temp.pnum = 0;
	temp.plen = 0;
	temp.pdat = 0x0;
	temp.dcnt = 0;
	temp.dprv = 0;
	temp.dlen = 0;
	/* resolve channel count */
	switch (temp.color)
	{
		case PNG_CTYPE_GRAYSCALE: temp.channel = 1; break;
		case PNG_CTYPE_INDEXED: temp.channel = 1; break;
		case PNG_CTYPE_RGB: temp.channel = 3; break;
		case PNG_CTYPE_RGBA: temp.channel = 4; break;
		case PNG_CTYPE_GRAYALPHA: temp.channel = 2; break;
		default: temp.channel = 0; break;
	}
#ifdef MY1DEBUG
	printf("\n");
	printf("--------------\n");
	printf("PNG DEBUG INFO\n");
	printf("--------------\n");
	printf("Width: %d, Height: %d\n", temp.width, temp.height);
	printf("Depth: %d, Color: 0x%x, Channel: %d\n",
		temp.depth, temp.color, temp.channel);
	printf("Compress: %d, Filter: %d, Interlace: %d\n",
		temp.compress, temp.filter, temp.interlace);
	printf("\n");
#endif
	switch (temp.color)
	{
		case PNG_CTYPE_GRAYSCALE:
		case PNG_CTYPE_INDEXED:
		case PNG_CTYPE_RGB:
		case PNG_CTYPE_RGBA:
			break;
		/* NOT supporting these for now... */
		case PNG_CTYPE_GRAYALPHA:
		default:
			pchunk_free(&buff);
			return PNG_ERROR_NOSUPPORT;
	}
	if (temp.depth>8||((temp.color==PNG_CTYPE_INDEXED)&&temp.depth<4)||
			temp.compress||temp.filter||temp.interlace)
	{
		pchunk_free(&buff);
		return PNG_ERROR_NOSUPPORT;
	}
	/* get other chunk(s) */
	step = 1;
	do
	{
		/* critical chunk: first letter uppercase */
		/* public chunk: second letter uppercase */
		/* PNG id chunk: third letter MUST BE uppercase */
		/* not-safe2copy chunk: fourth letter uppercase */
		test = pchunk_load(&buff,pngfile);
		if (!test)
		{
#ifdef MY1DEBUG
			printf("-- PNG_CHUNK => '%c%c%c%c'\n",buff.pngType[0],
				buff.pngType[1],buff.pngType[2],buff.pngType[3]);
#endif
			/* for colortype:3, tRNS chunk provide alpha pallette! */
			/* check crc? */
			if (!strncasecmp(buff.pngType,"IEND",4))
			{
				if (temp.dlen<2||temp.pdat[0]!=PNG_ZLIB_CHK1||
						(temp.pdat[1]!=PNG_ZLIB_LVL1&&
						temp.pdat[1]!=PNG_ZLIB_LVL5&&
						temp.pdat[1]!=PNG_ZLIB_LVL6&&
						temp.pdat[1]!=PNG_ZLIB_LVL9))
					test = PNG_ERROR_ZLIBCOMP;
				else if (temp.color==PNG_CTYPE_INDEXED&&!temp.plte)
					test = PNG_ERROR_MISSCHUNK;
				else /* OK to go! */
					test = png_make_image(&temp,image);
				/* done! */
				break;
			}
			else if (!strncasecmp(buff.pngType,"IDAT",4))
			{
				if (buff.pngSize>0)
				{
					/* check continuity */
					if (temp.dcnt)
					{
						if (temp.dprv!=step-1)
						{
							test = PNG_ERROR_DATACHUNK;
							break;
						}
					}
					temp.dprv = step;
					/* accumulate */
					dosize = temp.dlen + buff.pngSize;
					dotemp = (b08_t*) realloc(temp.pdat,dosize);
					if (!dotemp)
					{
						test = PNG_ERROR_MEMALLOC;
						break;
					}
					temp.pdat = dotemp;
					dotemp = &temp.pdat[temp.dlen];
					/* copy data */
					for (loop=0;loop<buff.pngSize;loop++)
						dotemp[loop] = buff.pngData[loop];
					temp.dlen = dosize;
					temp.dcnt++;
				}
			}
			else if (!strncasecmp(buff.pngType,"PLTE",4))
			{
				/* load pallette! data must be divisible by 3! */
				if (buff.pngSize%3!=0)
				{
					test = PNG_ERROR_PALLETTE;
					break;
				}
				if (temp.plte) /* multiple pallette? use latest... */
					free((void*)temp.plte);
				temp.plen = buff.pngSize;
				temp.pnum = buff.pngSize/3;
				temp.plte = (b08_t*) malloc(buff.pngSize);
				for (loop=0;loop<buff.pngSize;loop++)
					temp.plte[loop] = buff.pngData[loop];
			}
			else
			{
				/* ignored chunk! */
			}
		}
		else
		{
			test = PNG_ERROR_MISSCHUNK;
			break;
		}
		pchunk_free(&buff);
		step++;
	}
	while (test==FILE_OK);
	pchunk_free(&buff);
	if (temp.plte) free((void*)temp.plte);
	if (temp.pdat) free((void*)temp.pdat);
	return test;
}
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
