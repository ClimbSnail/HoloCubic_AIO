/*----------------------------------------------------------------------------/
/ TJpgDec - Tiny JPEG Decompressor R0.01c                     (C)ChaN, 2019
/-----------------------------------------------------------------------------/
/ The TJpgDec is a generic JPEG decompressor module for tiny embedded systems.
/ This is a free software that opened for education, research and commercial
/  developments under license policy of following terms.
/
/  Copyright (C) 2019, ChaN, all right reserved.
/
/ * The TJpgDec module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/
/ Oct 04, 2011 R0.01  First release.
/ Feb 19, 2012 R0.01a Fixed decompression fails when scan starts with an escape seq.
/ Sep 03, 2012 R0.01b Added JD_TBLCLIP option.
/ Mar 16, 2019 R0.01c Supprted stdint.h.
/----------------------------------------------------------------------------/
/ May 2019 ～ July 2020  Tweak for ESP32 ( modify by lovyan03 )
/----------------------------------------------------------------------------*/

#pragma GCC optimize ("O3")

#include "tjpgdClass.h"

#include <string.h> // for memcpy memset
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>


/*-----------------------------------------------*/
/* Zigzag-order to raster-order conversion table */
/*-----------------------------------------------*/

#define ZIG(n)	Zig[n]

static const uint8_t Zig[64] = {	/* Zigzag-order to raster-order conversion table */
	 0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};



/*-------------------------------------------------*/
/* Input scale factor of Arai algorithm            */
/* (scaled up 16 bits for fixed point operations)  */
/*-------------------------------------------------*/

#define IPSF(n)	Ipsf[n]

static const uint16_t Ipsf[64] = {	/* See also aa_idct.png */
	(uint16_t)(1.00000*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.17588*8192), (uint16_t)(1.00000*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.27590*8192),
	(uint16_t)(1.38704*8192), (uint16_t)(1.92388*8192), (uint16_t)(1.81226*8192), (uint16_t)(1.63099*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.08979*8192), (uint16_t)(0.75066*8192), (uint16_t)(0.38268*8192),
	(uint16_t)(1.30656*8192), (uint16_t)(1.81226*8192), (uint16_t)(1.70711*8192), (uint16_t)(1.53636*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.02656*8192), (uint16_t)(0.70711*8192), (uint16_t)(0.36048*8192),
	(uint16_t)(1.17588*8192), (uint16_t)(1.63099*8192), (uint16_t)(1.53636*8192), (uint16_t)(1.38268*8192), (uint16_t)(1.17588*8192), (uint16_t)(0.92388*8192), (uint16_t)(0.63638*8192), (uint16_t)(0.32442*8192),
	(uint16_t)(1.00000*8192), (uint16_t)(1.38704*8192), (uint16_t)(1.30656*8192), (uint16_t)(1.17588*8192), (uint16_t)(1.00000*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.27590*8192),
	(uint16_t)(0.78570*8192), (uint16_t)(1.08979*8192), (uint16_t)(1.02656*8192), (uint16_t)(0.92388*8192), (uint16_t)(0.78570*8192), (uint16_t)(0.61732*8192), (uint16_t)(0.42522*8192), (uint16_t)(0.21677*8192),
	(uint16_t)(0.54120*8192), (uint16_t)(0.75066*8192), (uint16_t)(0.70711*8192), (uint16_t)(0.63638*8192), (uint16_t)(0.54120*8192), (uint16_t)(0.42522*8192), (uint16_t)(0.29290*8192), (uint16_t)(0.14932*8192),
	(uint16_t)(0.27590*8192), (uint16_t)(0.38268*8192), (uint16_t)(0.36048*8192), (uint16_t)(0.32442*8192), (uint16_t)(0.27590*8192), (uint16_t)(0.21678*8192), (uint16_t)(0.14932*8192), (uint16_t)(0.07612*8192)
};



/*---------------------------------------------*/
/* Output bayer pattern table                  */
/*---------------------------------------------*/

static const int8_t Bayer[8][32] = {
	{ 0, 4, 1, 5,  0, 4, 1, 5, -2, 2,-1, 3, -2, 2,-1, 3,  1, 5, 0, 4,  1, 5, 0, 4, -1, 3,-2, 2, -1, 3,-2, 2},
	{ 1, 5, 0, 4,  1, 5, 0, 4, -1, 3,-2, 2, -1, 3,-2, 2,  0, 4, 1, 5,  0, 4, 1, 5, -2, 2,-1, 3, -2, 2,-1, 3},
	{ 2,-1, 3,-2,  2,-1, 3,-2,  5, 0, 4, 1,  5, 0, 4, 1,  3,-2, 2,-1,  3,-2, 2,-1,  4, 1, 5, 0,  4, 1, 5, 0},
	{ 3,-2, 2,-1,  3,-2, 2,-1,  4, 1, 5, 0,  4, 1, 5, 0,  2,-1, 3,-2,  2,-1, 3,-2,  5, 0, 4, 1,  5, 0, 4, 1},
	{ 4, 1, 5, 0,  4, 1, 5, 0,  2,-1, 3,-2,  2,-1, 3,-2,  5, 0, 4, 1,  5, 0, 4, 1,  3,-2, 2,-1,  3,-2, 2,-1},
	{ 5, 0, 4, 1,  5, 0, 4, 1,  3,-2, 2,-1,  3,-2, 2,-1,  4, 1, 5, 0,  4, 1, 5, 0,  2,-1, 3,-2,  2,-1, 3,-2},
	{-2, 2,-1, 3, -2, 2,-1, 3,  1, 5, 0, 4,  1, 5, 0, 4, -1, 3,-2, 2, -1, 3,-2, 2,  0, 4, 1, 5,  0, 4, 1, 5},
	{-1, 3,-2, 2, -1, 3,-2, 2,  0, 4, 1, 5,  0, 4, 1, 5, -2, 2,-1, 3, -2, 2,-1, 3,  1, 5, 0, 4,  1, 5, 0, 4}
};

/*---------------------------------------------*/
/* Conversion table for fast clipping process  */
/*---------------------------------------------*/

#if JD_TBLCLIP

#define BYTECLIP(v) Clip8[(uint16_t)(v) & 0x3FF]

static const uint8_t Clip8[1024] = {
	/* 0..255 */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
	/* 256..511 */
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	/* -512..-257 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* -256..-1 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#else	/* JD_TBLCLIP */

inline uint_fast8_t BYTECLIP (
	int32_t val
)
{
	if (val < 0) val = 0;
	else if (val > 255) val = 255;

	return val;
}

#endif


/*-----------------------------------------------------------------------*/
/* Allocate a memory block from memory pool                              */
/*-----------------------------------------------------------------------*/

static void* alloc_pool (	/* Pointer to allocated memory block (NULL:no memory available) */
	TJpgD* jd,		/* Pointer to the decompressor object */
	uint_fast16_t nd		/* Number of bytes to allocate */
)
{
	char *rp = 0;


	nd = (nd + 3) & ~3;			/* Align block size to the word boundary */

	if (jd->sz_pool >= nd) {
		jd->sz_pool -= nd;
		rp = (char*)jd->pool;			/* Get start of available memory pool */
		jd->pool = (void*)(rp + nd);	/* Allocate requierd bytes */
	}

	return (void*)rp;	/* Return allocated memory block (NULL:no memory to allocate) */
}




/*-----------------------------------------------------------------------*/
/* Create de-quantization and prescaling tables with a DQT segment       */
/*-----------------------------------------------------------------------*/

static int create_qt_tbl (	/* 0:OK, !0:Failed */
	TJpgD* jd,				/* Pointer to the decompressor object */
	const uint8_t* data,	/* Pointer to the quantizer tables */
	uint_fast16_t ndata			/* Size of input data */
)
{
	uint_fast8_t d, z;
	int32_t *pb;

	do {	/* Process all tables in the segment */
		d = *data++;							/* Get table property */
		if (d & 0xF0) return TJpgD::JDR_FMT1;			/* Err: not 8-bit resolution */
		pb = (int32_t*)alloc_pool(jd, 64 * sizeof (int32_t));/* Allocate a memory block for the table */
		if (!pb) return TJpgD::JDR_MEM1;				/* Err: not enough memory */
		jd->qttbl[d & 3] = pb;						/* Register the table */
		for (size_t i = 0; i < 64; ++i) {			/* Load the table */
			z = ZIG(i);							/* Zigzag-order to raster-order conversion */
			pb[z] = (int32_t)((uint32_t)data[i] * IPSF(z));	/* Apply scale factor of Arai algorithm to the de-quantizers */
		}
		data += 64;
	} while (ndata -= 65);

	return TJpgD::JDR_OK;
}




/*-----------------------------------------------------------------------*/
/* Create huffman code tables with a DHT segment                         */
/*-----------------------------------------------------------------------*/

static int create_huffman_tbl (	/* 0:OK, !0:Failed */
	TJpgD* jd,					/* Pointer to the decompressor object */
	const uint8_t* data,		/* Pointer to the packed huffman tables */
	uint_fast16_t ndata				/* Size of input data */
)
{
	uint_fast16_t d, b, np, cls, num, hc;
	uint8_t *pb, *pd;
	uint_fast16_t *ph;


	do {	/* Process all tables in the segment */
		d = *data++;						/* Get table number and class */
		if (d & 0xEE) return TJpgD::JDR_FMT1;		/* Err: invalid class/number */
		cls = d >> 4; num = d & 0x0F;		/* class = dc(0)/ac(1), table number = 0/1 */
		pb = (uint8_t*)alloc_pool(jd, 16);			/* Allocate a memory block for the bit distribution table */
		if (!pb) return TJpgD::JDR_MEM1;			/* Err: not enough memory */
		jd->huffbits[num][cls] = pb - 1;
		np = 0;
		for (size_t i = 0; i < 16; ++i) {		/* Load number of patterns for 1 to 16-bit code */
			np += (pb[i] = data[i]);		/* Get sum of code words for each code */
		}

		ph = (uint_fast16_t*)alloc_pool(jd, (np * sizeof (uint_fast16_t)));/* Allocate a memory block for the code word table */
		if (!ph) return TJpgD::JDR_MEM1;			/* Err: not enough memory */
		jd->huffcode[num][cls] = ph - 1;
		hc = 0;
		for (size_t i = 0; i < 16; ++i) {		/* Re-build huffman code word table */
			b = pb[i];
			while (b--) *ph++ = hc++;
			hc <<= 1;
		}

		pd = (uint8_t*)alloc_pool(jd, np);			/* Allocate a memory block for the decoded data */
		if (!pd) return TJpgD::JDR_MEM1;			/* Err: not enough memory */
		jd->huffdata[num][cls] = pd - 1;

		memcpy(pd, data += 16, np);		/* Load decoded data corresponds to each code ward */
		data += np;
	} while (ndata -= 17 + np);

	return TJpgD::JDR_OK;
}




/*-----------------------------------------------------------------------*/
/* Extract N bits from input stream                                      */
/*-----------------------------------------------------------------------*/

static inline int_fast16_t bitext (	/* >=0: extracted data, <0: error code */
	TJpgD* jd,		/* Pointer to the decompressor object */
	int_fast16_t nbit		/* Number of bits to extract (1 to 11) */
)
{
	uint8_t *dp = jd->dptr;	/* Bit mask, number of data available, read ptr */
	uint_fast8_t s = *dp;
	uint_fast8_t msk = jd->dmsk;
	uint_fast16_t v = 0;

	if (msk) {
		if (msk >= nbit) {
			msk -= nbit;
			jd->dmsk = msk;
			return (s >> msk) & ((1 << nbit) - 1);	/* Get bits */
		}
		nbit -= msk;
		v = (s & ((1 << msk) - 1)) << nbit;	/* Get bits */
	}

	uint8_t *dpend = jd->dpend;

	for (;;) {
		if (++dp == dpend) {			/* No input data is available, re-fill input buffer */
			dp = jd->inbuf;	/* Top of input buffer */
			jd->dpend = dpend = dp + jd->infunc(jd, dp, TJPGD_SZBUF);
			if (dp == dpend) return 0 - (int_fast16_t)TJpgD::JDR_INP;	/* Err: read error or wrong stream termination */
		}
		s = *dp;				/* Get next data byte */
		if (s == 0xFF) {		/* Is start of flag sequence? */
			if (++dp == dpend) {			/* No input data is available, re-fill input buffer */
				dp = jd->inbuf;	/* Top of input buffer */
				jd->dpend = dpend = dp + jd->infunc(jd, dp, TJPGD_SZBUF);
				if (dp == dpend) return 0 - (int_fast16_t)TJpgD::JDR_INP;	/* Err: read error or wrong stream termination */
			}
			if (*dp != 0) return 0 - (int_fast16_t)TJpgD::JDR_FMT1;	/* Err: unexpected flag is detected (may be collapted data) */
			*dp = s;			/* The flag is a data 0xFF */
		}
		if (8 >= nbit) {
			msk = 8 - nbit;
			jd->dmsk = msk; jd->dptr = dp;
			return v + ((s >> msk) & ((1 << nbit) - 1));	/* Get bits */
		}
		nbit -= 8;
		v |= s << nbit;	/* Get bits */
	}
}




/*-----------------------------------------------------------------------*/
/* Extract a huffman decoded data from input stream                      */
/*-----------------------------------------------------------------------*/

static int_fast16_t huffext (	/* >=0: decoded data, <0: error code */
	TJpgD* jd,				/* Pointer to the decompressor object */
	const uint8_t* hbits,	/* Pointer to the bit distribution table */
	const uint_fast16_t* hcode,	/* Pointer to the code word table */
	const uint8_t* hdata	/* Pointer to the data table */
)
{
	uint_fast8_t msk = jd->dmsk;
	uint8_t *dpend = jd->dpend;	/* Bit mask, number of data available, read ptr */
	uint8_t *dp = jd->dptr;
	uint_fast8_t s = *dp;
	uint_fast8_t v = 0;
	uint_fast8_t bl = 16;	/* Max code length */

	for (;;) {
		if (!msk) {				/* Next byte? */
			msk = 8;			/* Read from MSB */
			if (++dp == dpend) {			/* No input data is available, re-fill input buffer */
				dp = jd->inbuf;	/* Top of input buffer */
				jd->dpend = dpend = dp + jd->infunc(jd, dp, TJPGD_SZBUF);
				if (dp == dpend) return 0 - (int_fast16_t)TJpgD::JDR_INP;	/* Err: read error or wrong stream termination */
			}
			s = *dp;				/* Get next data byte */
			if (s == 0xFF) {		/* Is start of flag sequence? */
				if (++dp == dpend) {			/* No input data is available, re-fill input buffer */
					dp = jd->inbuf;	/* Top of input buffer */
					jd->dpend = dpend = dp + jd->infunc(jd, dp, TJPGD_SZBUF);
					if (dp == dpend) return 0 - (int_fast16_t)TJpgD::JDR_INP;	/* Err: read error or wrong stream termination */
				}
				if (*dp != 0) return 0 - (int_fast16_t)TJpgD::JDR_FMT1;	/* Err: unexpected flag is detected (may be collapted data) */
				*dp = s;			/* The flag is a data 0xFF */
			}
		}
		do {
			v = (v << 1) + ((s >> (--msk)) & 1);	/* Get a bit */
			uint_fast8_t nd = *++hbits;
			if (nd) {
				do {
					++hdata;
					if (v == *++hcode) goto huffext_match;	/* Matched? */
				} while (--nd);	/* Search the code word in this bit length */
			}
			if (!--bl) return 0 - (int_fast16_t)TJpgD::JDR_FMT1;	/* Err: code not found (may be collapted data) */
		} while (msk);
	}
huffext_match:
	jd->dmsk = msk;
	jd->dptr = dp;
	return *hdata;					/* Return the decoded data */
}

/*-----------------------------------------------------------------------*/
/* Apply Inverse-DCT in Arai Algorithm (see also aa_idct.png)            */
/*-----------------------------------------------------------------------*/

static void block_idct (
	int32_t* src,	/* Input block data (de-quantized and pre-scaled for Arai Algorithm) */
	uint8_t* dst	/* Pointer to the destination to store the block as byte array */
)
{
	const int32_t M13 = (int32_t)(1.41421*256), M4 = (int32_t)(2.61313*256);
	const float F2 = 1.08239, F5 = 1.84776;

	int32_t v0, v1, v2, v3, v4, v5, v6, v7;
	int32_t t10, t11, t12, t13;

	/* Process columns */
	for (size_t i = 0; i < 8; ++i) {
		/* Get and Process the even elements */
		t12 = src[8 * 0];
		t10 = src[8 * 4];
		t10 += t12;
		t12 = (t12 << 1) - t10;

		t11 = src[8 * 2];
		t13 = src[8 * 6];
		t13 += t11;
		t11 = (t11 << 1) - t13;
		t11 = t11 * M13 >> 8;
		t11 = t11 - t13;

		v0 = t10 + t13;
		v3 = t10 - t13;
		v1 = t12 + t11;
		v2 = t12 - t11;

		/* Get and Process the odd elements */
		v4 = src[8 * 1];
		v5 = src[8 * 7];
		v5 += v4;
		v4 = (v4 << 1) - v5;

		v7 = src[8 * 3];
		v6 = src[8 * 5];
		v6 -= v7;
		v7 = (v7 << 1) + v6;
		v7 += v5;

		t13 = v4 + v6;
		t13 *= F5;
		v6 = v6 * M4 >> 8;
		v6 += v7;
		v6 = t13 - v6;
		v5 = (v5 << 1) - v7;
		v5 = v5 * M13 >> 8;
		v5 -= v6;
		v4 *= F2;
		v4 += v5;
		v4 = t13 - v4;

		/* Write-back transformed values */
		src[8 * 0] = v0 + v7;
		src[8 * 7] = v0 - v7;
		src[8 * 1] = v1 + v6;
		src[8 * 6] = v1 - v6;
		src[8 * 2] = v2 + v5;
		src[8 * 5] = v2 - v5;
		src[8 * 3] = v3 + v4;
		src[8 * 4] = v3 - v4;

		++src;	/* Next column */
	}

	/* Process rows */
	src -= 8;
	for (size_t i = 0; i < 8; ++i) {
		/* Get and Process the even elements */
		t12 = src[0] + (128L << 8);	/* remove DC offset (-128) here */
		t10 = src[4];
		t10 += t12;
		t12 = (t12 << 1) - t10;

		t11 = src[2];
		t13 = src[6];
		t13 += t11;
		t11 = (t11 << 1) - t13;
		t11 = t11 * M13 >> 8;
		t11 -= t13;

		v0 = t10 + t13;
		v3 = t10 - t13;
		v1 = t12 + t11;
		v2 = t12 - t11;

		/* Get and Process the odd elements */
		v4 = src[1];
		v5 = src[7];
		v5 += v4;
		v4 = (v4 << 1) - v5;

		v7 = src[3];
		v6 = src[5];
		v6 -= v7;
		v7 = (v7 << 1) + v6;
		v7 += v5;

		t13 = v4 + v6;
		t13 *= F5;
		v6 = v6 * M4 >> 8;
		v6 += v7;
		v6 = t13 - v6;
		v5 = (v5 << 1) - v7;
		v5 = v5 * M13 >> 8;
		v5 -= v6;
		v4 *= F2;
		v4 += v5;
		v4 = t13 - v4;

		/* Descale the transformed values 8 bits and output */
		dst[0] = BYTECLIP((v0 + v7) >> 8);
		dst[7] = BYTECLIP((v0 - v7) >> 8);
		dst[1] = BYTECLIP((v1 + v6) >> 8);
		dst[6] = BYTECLIP((v1 - v6) >> 8);
		dst[2] = BYTECLIP((v2 + v5) >> 8);
		dst[5] = BYTECLIP((v2 - v5) >> 8);
		dst[3] = BYTECLIP((v3 + v4) >> 8);
		dst[4] = BYTECLIP((v3 - v4) >> 8);

		dst += 8;
		src += 8;	/* Next row */
	}
}




/*-----------------------------------------------------------------------*/
/* Load all blocks in the MCU into working buffer                        */
/*-----------------------------------------------------------------------*/

static TJpgD::JRESULT mcu_load (
	TJpgD* jd,		/* Pointer to the decompressor object */
	uint8_t* bp,		/* mcubuf */
	int32_t* tmp	/* Block working buffer for de-quantize and IDCT */
)
{
	int_fast16_t b, d, e;
	uint_fast8_t blk, nby, nbc, i, z;
	const uint8_t *hb, *hd;
	const uint_fast16_t *hc;

	nby = jd->msx * jd->msy;	/* Number of Y blocks (1, 2 or 4) */
	nbc = 2;					/* Number of C blocks (2) */

	for (blk = 0; blk < nby + nbc; blk++) {
		uint_fast8_t cmp = (blk < nby) ? 0 : blk - nby + 1;	/* Component number 0:Y, 1:Cb, 2:Cr */
		uint_fast8_t id = cmp ? 1 : 0;						/* Huffman table ID of the component */

		/* Extract a DC element from input stream */
		hb = jd->huffbits[id][0];				/* Huffman table for the DC element */
		hc = jd->huffcode[id][0];
		hd = jd->huffdata[id][0];
		b = huffext(jd, hb, hc, hd);			/* Extract a huffman coded data (bit length) */
		if (b < 0) return (TJpgD::JRESULT)(-b);		/* Err: invalid code or input */
		d = jd->dcv[cmp];						/* DC value of previous block */
		if (b) {								/* If there is any difference from previous block */
			e = bitext(jd, b);					/* Extract data bits */
			if (e < 0) return (TJpgD::JRESULT)(-e);	/* Err: input */
			b = 1 << (b - 1);					/* MSB position */
			if (!(e & b)) e -= (b << 1) - 1;	/* Restore sign if needed */
			d += e;								/* Get current value */
			jd->dcv[cmp] = d;			/* Save current DC value for next block */
		}
		const int32_t *dqf = jd->qttbl[jd->qtid[cmp]];			/* De-quantizer table ID for this component */
		tmp[0] = d * dqf[0] >> 8;				/* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */

		/* Extract following 63 AC elements from input stream */
		memset(&tmp[1], 0, 4 * 63);				/* Clear rest of elements */
		hb = jd->huffbits[id][1];				/* Huffman table for the AC elements */
		hc = jd->huffcode[id][1];
		hd = jd->huffdata[id][1];
		i = 1;					/* Top of the AC elements */
		do {
			b = huffext(jd, hb, hc, hd);		/* Extract a huffman coded value (zero runs and bit length) */
			if (b == 0) break;					/* EOB? */
			if (b < 0) return (TJpgD::JRESULT)(-b);	/* Err: invalid code or input error */
			i += b >> 4;
			if (b &= 0x0F) {					/* Bit length */
				d = bitext(jd, b);				/* Extract data bits */
				if (d < 0) return (TJpgD::JRESULT)(-d);/* Err: input device */
				b = 1 << (b - 1);				/* MSB position */
				if (!(d & b)) d -= (b << 1) - 1;/* Restore negative value if needed */
				z = ZIG(i);						/* Zigzag-order to raster-order converted index */
				tmp[z] = d * dqf[z] >> 8;		/* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */
			}
		} while (++i != 64);		/* Next AC element */

		block_idct(tmp, bp);		/* Apply IDCT and store the block to the MCU buffer */

		bp += 64;				/* Next block */
	}

	return TJpgD::JDR_OK;	/* All blocks have been loaded successfully */
}




/*-----------------------------------------------------------------------*/
/* Output an MCU: Convert YCrCb to RGB and output it in RGB form         */
/*-----------------------------------------------------------------------*/

static TJpgD::JRESULT mcu_output (
	TJpgD* jd,		/* Pointer to the decompressor object */
	uint8_t* mcubuf,
	uint8_t* workbuf,
	uint32_t (*outfunc)(TJpgD*, void*, TJpgD::JRECT*),	/* RGB output function */
	uint_fast16_t x,		/* MCU position in the image (left of the MCU) */
	uint_fast16_t y		/* MCU position in the image (top of the MCU) */
)
{
	uint_fast16_t ix, iy, mx, my, rx, ry;
	uint8_t *py, *pc;
	TJpgD::JRECT rect;

	mx = jd->msx * 8; my = jd->msy * 8;					/* MCU size (pixel) */
	rx = (x + mx <= jd->width) ? mx : jd->width - x;	/* Output rectangular size (it may be clipped at right/bottom end) */
	ry = (y + my <= jd->height) ? my : jd->height - y;

	rect.left = x; rect.right = x + rx - 1;				/* Rectangular area in the frame buffer */
	rect.top = y; rect.bottom = y + ry - 1;

	static constexpr float frr = 1.402;
	static constexpr float fgr = 0.71414;
	static constexpr float fgb = 0.34414;
	static constexpr float fbb = 1.772;

	/* Build an RGB MCU from discrete comopnents */
	const int8_t* btbase = Bayer[jd->bayer];
	const int8_t* btbl;
	uint_fast8_t ixshift = (mx == 16);
	uint_fast8_t iyshift = (my == 16);
	iy = 0;
	uint8_t* prgb = workbuf;
	do {
		btbl = &btbase[(iy & 3) << 3];
		py = &mcubuf[((iy & 8) + iy) << 3];
		pc = &mcubuf[((mx << iyshift) + (iy >> iyshift)) << 3];
		ix = 0;
		do {
			do {
				float cb = (pc[ 0] - 128); 	/* Get Cb/Cr component and restore right level */
				float cr = (pc[64] - 128);
				++pc;

				/* Convert CbCr to RGB */
				int32_t gg = fgb * cb + fgr * cr;
				int32_t rr = frr * cr;
				int32_t bb = fbb * cb;
				int32_t yy = btbl[0] + py[0];			/* Get Y component */
				prgb[0] = BYTECLIP(yy + rr);
				prgb[1] = BYTECLIP(yy - gg);
				prgb[2] = BYTECLIP(yy + bb);
				if (ixshift) {
					yy = btbl[1] + py[1];			/* Get Y component */
					prgb[3] = BYTECLIP(yy + rr);
					prgb[4] = BYTECLIP(yy - gg);
					prgb[5] = BYTECLIP(yy + bb);
				}
				prgb += 3 << ixshift;
				btbl += 1 << ixshift;
				py += 1 << ixshift;
				ix += 1 << ixshift;
			} while (ix & 7);
			btbl -= 8;
			py += 64 - 8;	/* Jump to next block if double block heigt */
		} while (ix != mx);
	} while (++iy != my);

	if (rx < mx) {
		uint8_t *s, *d;
		s = d = (uint8_t*)workbuf;
		rx *= 3;
		mx *= 3;
		for (size_t y = 1; y < ry; ++y) {
			memcpy(d += rx, s += mx, rx);	/* Copy effective pixels */
		}
	}
	/* Output the RGB rectangular */
	return outfunc(jd, workbuf, &rect) ? TJpgD::JDR_OK : TJpgD::JDR_INTR; 
}


/*-----------------------------------------------------------------------*/
/* Process restart interval                                              */
/*-----------------------------------------------------------------------*/

static TJpgD::JRESULT restart (
	TJpgD* jd,		/* Pointer to the decompressor object */
	uint_fast16_t rstn	/* Expected restert sequense number */
)
{
	uint_fast16_t d;
	uint8_t *dp, *dpend;


	/* Discard padding bits and get two bytes from the input stream */
	dp = jd->dptr; dpend = jd->dpend;
	d = 0;
	for (size_t i = 0; i < 2; i++) {
		if (++dp == dpend) {	/* No input data is available, re-fill input buffer */
			dp = jd->inbuf;
			jd->dpend = dpend = dp + jd->infunc(jd, dp, TJPGD_SZBUF);
			if (dp == dpend) return TJpgD::JDR_INP;
		}
		d = (d << 8) | *dp;	/* Get a byte */
	}
	jd->dptr = dp; jd->dmsk = 0;

	/* Check the marker */
	if ((d & 0xFFD8) != 0xFFD0 || (d & 7) != (rstn & 7)) {
		return TJpgD::JDR_FMT1;	/* Err: expected RSTn marker is not detected (may be collapted data) */
	}

	/* Reset DC offset */
	jd->dcv[2] = jd->dcv[1] = jd->dcv[0] = 0;

	return TJpgD::JDR_OK;
}




/*-----------------------------------------------------------------------*/
/* Analyze the JPEG image and Initialize decompressor object             */
/*-----------------------------------------------------------------------*/

#define	LDB_WORD(ptr)		(uint16_t)(((uint16_t)*((uint8_t*)(ptr))<<8)|(uint16_t)*(uint8_t*)((ptr)+1))


TJpgD::JRESULT TJpgD::prepare (
	uint32_t (*infunc)(TJpgD*, uint8_t*, uint32_t),	/* JPEG strem input function */
	void* dev			/* I/O device identifier for the session */
)
{
	uint8_t *seg;
	uint_fast8_t b, marker;
	uint_fast16_t i, len;
	TJpgD::JRESULT rc;

	static constexpr uint_fast16_t sz_pool = 3900;
	static uint8_t pool[sz_pool];


	this->pool = pool;		/* Work memroy */
	this->sz_pool = sz_pool;	/* Size of given work memory */
	this->infunc = infunc;	/* Stream input function */
	this->device = dev;		/* I/O device identifier */
	this->nrst = 0;			/* No restart interval (default) */

	inbuf = seg = dptr = (uint8_t*)alloc_pool(this, TJPGD_SZBUF);		/* Allocate stream input buffer */
	if (!seg) return TJpgD::JDR_MEM1;

	uint32_t dctr = infunc(this, dptr, TJPGD_SZBUF);
	seg = dptr;
	if (dctr <= 2) return TJpgD::JDR_INP;/* Check SOI marker */
	if (LDB_WORD(seg) != 0xFFD8) return TJpgD::JDR_FMT1;	/* Err: SOI is not detected */
	dptr += 2; dctr -= 2;

	for (;;) {
		/* Get a JPEG marker */
		if (dctr < 4) {
			if (4 > (TJPGD_SZBUF - (dptr - inbuf))) return TJpgD::JDR_MEM2;
			dctr += infunc(this, dptr + dctr, 4);
			if (dctr < 4) return TJpgD::JDR_INP;
		}
		seg = dptr;
		dptr += 4;
		dctr -= 4;

		if (*seg++ != 0xFF) return TJpgD::JDR_FMT1;
		marker = *(seg++);		/* Marker */
		len = LDB_WORD(seg);	/* Length field */
		if (len <= 2) return TJpgD::JDR_FMT1;
		len -= 2;		/* Content size excluding length field */

		/* Load segment data */
		if (dctr < len) {
			if (len - dctr > (TJPGD_SZBUF - (dptr - inbuf))) return TJpgD::JDR_MEM2;
			dctr += infunc(this, dptr + dctr, len - dctr);
			if (dctr < len) return TJpgD::JDR_INP;
		}
		seg = dptr;
		dptr += len;
		dctr -= len;
		switch (marker) {
		case 0xC0:	/* SOF0 (baseline JPEG) */
			width = LDB_WORD(seg+3);		/* Image width in unit of pixel */
			height = LDB_WORD(seg+1);		/* Image height in unit of pixel */
			if (seg[5] != 3) return TJpgD::JDR_FMT3;	/* Err: Supports only Y/Cb/Cr format */

			/* Check three image components */
			for (i = 0; i < 3; i++) {
				b = seg[7 + 3 * i];							/* Get sampling factor */
				if (!i) {	/* Y component */
					if (b != 0x11 && b != 0x22 && b != 0x21) {	/* Check sampling factor */
						return TJpgD::JDR_FMT3;					/* Err: Supports only 4:4:4, 4:2:0 or 4:2:2 */
					}
					msx = b >> 4; msy = b & 15;		/* Size of MCU [blocks] */
				} else {	/* Cb/Cr component */
					if (b != 0x11) return TJpgD::JDR_FMT3;			/* Err: Sampling factor of Cr/Cb must be 1 */
				}
				b = seg[8 + 3 * i];							/* Get dequantizer table ID for this component */
				if (b > 3) return TJpgD::JDR_FMT3;					/* Err: Invalid ID */
				qtid[i] = b;
			}
			break;

		case 0xDD:	/* DRI */
			/* Get restart interval (MCUs) */
			nrst = LDB_WORD(seg);
			break;

		case 0xC4:	/* DHT */
			/* Create huffman tables */
			rc = (TJpgD::JRESULT)create_huffman_tbl(this, seg, len);
			if (rc) return rc;
			break;

		case 0xDB:	/* DQT */
			/* Create de-quantizer tables */
			rc = (TJpgD::JRESULT)create_qt_tbl(this, seg, len);
			if (rc) return rc;
			break;

		case 0xDA:	/* SOS */
			if (!width || !height) return TJpgD::JDR_FMT1;	/* Err: Invalid image size */

			if (seg[0] != 3) return TJpgD::JDR_FMT3;				/* Err: Supports only three color components format */

			/* Check if all tables corresponding to each components have been loaded */
			for (i = 0; i < 3; i++) {
				b = seg[2 + 2 * i];	/* Get huffman table ID */
				if (b != 0x00 && b != 0x11)	return TJpgD::JDR_FMT3;	/* Err: Different table number for DC/AC element */
				b = i ? 1 : 0;
				if (!huffbits[b][0] || !huffbits[b][1]) {	/* Check dc/ac huffman table for this component */
					return TJpgD::JDR_FMT1;					/* Err: Nnot loaded */
				}
				if (!qttbl[qtid[i]]) {			/* Check dequantizer table for this component */
					return TJpgD::JDR_FMT1;					/* Err: Not loaded */
				}
			}

			/* Allocate working buffer for MCU and RGB */
			if (!msy || !msx) return TJpgD::JDR_FMT1;					/* Err: SOF0 has not been loaded */
			dmsk = 0;
			dpend = dptr + dctr;
			--dptr;

			return TJpgD::JDR_OK;		/* Initialization succeeded. Ready to decompress the JPEG image. */

		case 0xC1:	/* SOF1 */
		case 0xC2:	/* SOF2 */
		case 0xC3:	/* SOF3 */
		case 0xC5:	/* SOF5 */
		case 0xC6:	/* SOF6 */
		case 0xC7:	/* SOF7 */
		case 0xC9:	/* SOF9 */
		case 0xCA:	/* SOF10 */
		case 0xCB:	/* SOF11 */
		case 0xCD:	/* SOF13 */
		case 0xCE:	/* SOF14 */
		case 0xCF:	/* SOF15 */
		case 0xD9:	/* EOI */
			return TJpgD::JDR_FMT3;	/* Unsuppoted JPEG standard (may be progressive JPEG) */

		default:	/* Unknown segment (comment, exif or etc..) */
			break;
		}
	}
}




/*-----------------------------------------------------------------------*/
/* Start to decompress the JPEG picture                                  */
/*-----------------------------------------------------------------------*/

TJpgD::JRESULT TJpgD::decomp (
	uint32_t (*outfunc)(TJpgD*, void*, TJpgD::JRECT*),	/* RGB output function */
	uint32_t (*linefunc)(TJpgD*,uint32_t,uint32_t),
	uint32_t lineskip						/* linefunc skip number */
)
{
	uint16_t x, y, mx, my;
	uint16_t rst, rsc;
	TJpgD::JRESULT rc;
	uint8_t workbuf[768];
	uint8_t mcubuf[384];
	uint8_t yidx = 0;

	bayer = (bayer + 1) & 7;

	mx = msx * 8; my = msy * 8;			/* Size of the MCU (pixel) */
	uint16_t lasty = ((height - 1) / my) * my;

	dcv[2] = dcv[1] = dcv[0] = 0;	/* Initialize DC values */
	rst = rsc = 0;

	rc = TJpgD::JDR_OK;
	for (y = 0; y < height; y += my) {		/* Vertical loop of MCUs */
		for (x = 0; x < width; x += mx) {	/* Horizontal loop of MCUs */
			if (nrst && rst++ == nrst) {	/* Process restart interval if enabled */
				rc = restart(this, rsc++);
				if (rc != TJpgD::JDR_OK) return rc;
				rst = 1;
			}
			rc = mcu_load(this, mcubuf, (int32_t*)workbuf);		/* Load an MCU (decompress huffman coded stream and apply IDCT) */
			if (rc != TJpgD::JDR_OK) return rc;
			rc = mcu_output(this, mcubuf, (uint8_t*)workbuf, outfunc, x, y);	/* Output the MCU (color space conversion, scaling and output) */
			if (rc != TJpgD::JDR_OK) return rc;
		}
		if (linefunc && (yidx == lineskip || y == lasty)) {
			linefunc(this, y - yidx * my, yidx * my + ((height < y + my) ? height - y : my));
			yidx = 0;
		} else {
			++yidx;
		}
	}

	return rc;
}






typedef struct {
	uint8_t* mcubuf = NULL;
	uint_fast16_t x = 0;
	uint_fast16_t y = 0;
	uint_fast8_t h = 0;
	volatile uint_fast8_t queue = false;
} queue_t;

typedef struct {
	TJpgD* jd;
	uint32_t (*outfunc)(TJpgD*, void*, TJpgD::JRECT*);
	uint32_t (*linefunc)(TJpgD*,uint32_t,uint32_t);
	QueueHandle_t sem;
	TaskHandle_t task;
} param_task_output;

static constexpr uint_fast8_t queue_max = 20;
static param_task_output param;
static uint8_t mcubufs[queue_max + 1][384];
static queue_t qwrites[queue_max];
static queue_t qline;
static uint_fast8_t qidx = 0;
static uint_fast8_t mcuidx = 0;

static void task_output(void* arg)
{
	uint8_t workbuf[768];
	param_task_output* p = (param_task_output*)arg;
	queue_t* q;
//Serial.println("task_output start");
	for (;;) {
		if (!xQueueReceive(p->sem, &q, portMAX_DELAY)) continue;
		if (!q) break;
//Serial.printf("task work: X=%d,Y=%d\r\n",q->x,q->y);
		if (q->h == 0) {
			mcu_output(p->jd, q->mcubuf, workbuf, p->outfunc, q->x, q->y);
		} else {
			p->linefunc(p->jd, q->y, q->h);
		}
		q->queue = false;
//Serial.println("task work done");
	}
	vQueueDelete(p->sem);
//Serial.println("task_output end");
	vTaskDelete(NULL);
}

void TJpgD::multitask_begin ()
{
	param.sem = xQueueCreate(queue_max + 1, sizeof(queue_t*));

	xTaskCreatePinnedToCore(task_output, "task_output", 1600, &param, 1, &param.task, 0);
}

void TJpgD::multitask_end ()
{
	queue_t* q = NULL;
	xQueueSend(param.sem, &q, 0);
	vTaskDelay(10);
}

TJpgD::JRESULT TJpgD::decomp_multitask (
	uint32_t (*outfunc)(TJpgD*, void*, TJpgD::JRECT*),	/* RGB output function */
	uint32_t (*linefunc)(TJpgD*,uint32_t,uint32_t),
	uint32_t lineskip						/* linefunc skip number */
)
{
	uint_fast16_t x, y, mx, my;
	uint_fast16_t rst, rsc;
	TJpgD::JRESULT rc;
	uint8_t workbuf[768];
	uint_fast16_t yidx = 0;


	bayer = (bayer + 1) & 7;

	param.jd = this;
	param.outfunc = outfunc;
	param.linefunc = linefunc;
	queue_t* q = &qwrites[qidx];
	queue_t* ql = &qline;
	queue_t* qtmp = NULL;

	mx = msx * 8; my = msy * 8;			/* Size of the MCU (pixel) */

	dcv[2] = dcv[1] = dcv[0] = 0;	/* Initialize DC values */
	rst = rsc = 0;
	uint_fast16_t lasty = ((height - 1) / my) * my;

	rc = TJpgD::JDR_OK;
	y = 0;
	do {		/* Vertical loop of MCUs */
		x = 0;
		do {	/* Horizontal loop of MCUs */
			if (nrst && rst++ == nrst) {	/* Process restart interval if enabled */
				rc = restart(this, rsc++);
				if (rc != TJpgD::JDR_OK) break;
				rst = 1;
			}
			rc = mcu_load(this, mcubufs[mcuidx], (int32_t*)workbuf);
			if (rc != TJpgD::JDR_OK) break;
			if (!q->queue) {
//mcubufs[mcuidx][0] = 0;
//mcubufs[mcuidx][1] = 0;
				q->mcubuf  = mcubufs[mcuidx];
				q->x = x;
				q->y = y;
				q->queue = true;
				xQueueSend(param.sem, &q, 0);
				mcuidx = (1 + mcuidx) % (queue_max + 1);
				qidx = (1 + qidx) % queue_max;
				q = &qwrites[qidx];
			} else {
				while (ql->queue) taskYIELD();
//mcubufs[mcuidx][0] = 0xFF;
//mcubufs[mcuidx][1] = 0xFF;
				rc = mcu_output(this, mcubufs[mcuidx], workbuf, outfunc, x, y);
			}
		} while ((x += mx) < width);
		if (rc != TJpgD::JDR_OK) break;
		if (linefunc && (yidx == lineskip || y == lasty)) {
			while (ql->queue) taskYIELD();
			while (xQueueReceive(param.sem, &qtmp, 0)) {
//qtmp->mcubuf[0] = 0xFF;
//qtmp->mcubuf[1] = 0xFF;
				mcu_output(this, qtmp->mcubuf, workbuf, outfunc, qtmp->x, qtmp->y);
				qtmp->queue = false;
			}
			ql->h = (y == lasty) ? (yidx * my + height - y) : ((lineskip + 1) * my);
			ql->y = y - yidx * my;
			ql->queue = true;
			xQueueSend(param.sem, &ql, 0);
			yidx = 0;
		} else {
			++yidx;
		}
	} while ((y += my) < height);
	return rc;
}


