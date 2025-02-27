/*
 * Autor: Stanislav Sigmund (xsigmu02)
 * Datum: 20. 4. 2008
 * Soubor: gif2bmp.c
 * Komentar: Jednotka gif2bmp - pro konverzi statickych GIF obrazku na BMP
 */ 

#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct{
	uint8_t bfType[2]; // "BM"
	uint32_t bfSize; // velikost souboru
	uint32_t bfRes; // 0
	uint32_t bfOffBits; // offset k datum
	uint32_t biSize; // 40 - velikost hlavicky
	uint32_t biWidth; // sirka + vyska
	uint32_t biHeight;
	uint16_t biPlanes; // 1
	uint16_t biBitCount; // 8
	uint32_t biCompression; // 0 - bez komprese
	uint32_t biSizeImage; // velikost, validni je 0
	uint32_t biXPelsPerMeter; // velikost, validni je 0
	uint32_t biYPelsPerMeter; // velikost, validni je 0
	uint32_t biClrUsed; // velikost, validni je 0
	uint32_t biClrImportant; // velikost, validni je 0
} tBMPHead;

typedef struct{
	uint8_t type[4]; // "RAFF"
	uint32_t version; // zatim jen 0x00010000
	uint16_t headerSize; // velikost hlavicky
	uint16_t width;
	uint16_t height;
	uint16_t speed; // 0-bez omezeni, bude 1=25FPS atd...
	uint32_t frames; // pocet snimku animace
	uint16_t comp; // typ komprese - 0=hrube snimky za sebou
		// 1=typ komprese je v hlavicce(bez kopirovani bloku) 2=jako 1, ale s kopirovanim bloku
	uint16_t index; // pritomnost indexu snimku za hlavickou, zatim 0-bez indexu
	uint16_t soundsize; // velikost jednotlivych bloku
	uint16_t soundcomp; // typ komprese zvuku
} tRAFFHead;

/* struktura polozky palety BMP souboru */
typedef struct{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t res;
} tBMPpalList;	

void err(char *str) {
	fprintf(stderr,str);
	fprintf(stderr,"!\n");
	exit(1);
}

int main(int argc, char **argv)
{
	FILE *in,*out;
	char *data[200];
	int count=0;
	unsigned short dw;
		unsigned long size;
	if (argc<3) {
		printf("Program na konverzi skupiny obrazku do formatu raf\n");
		printf("Priklad pouziti:\nbmp2raf cilovy_raf_soubor soubory_bmp\n");
		printf("BMP soubory museji mit hloubku barev 256 a byt stejne velike\n");
		exit(1);
	}

	tBMPHead BMPhead;
	tRAFFHead Rhead;
	tBMPpalList BMPpal[256];
	int i,c,w,h;
	unsigned char *dp;
	for (c=2;c<argc;c++) {
		in = fopen(argv[c],"rb");
						
		if (fread(&BMPhead,2,1,in) <= 0) err("hlavicka souboru");
		if ((BMPhead.bfType[0] != 'B') || (BMPhead.bfType[1] != 'M')) err("typ souboru");
		if (fread(&(BMPhead.bfSize),52,1,in) <= 0) err("hlavicka souboru");
		if (BMPhead.biCompression != 0) err("komprese");
		if (fread(&BMPpal,1024,1,in) <= 0) err("paleta");
		fseek(in,BMPhead.bfOffBits,SEEK_SET);
		if (count==0) {
			w=BMPhead.biWidth;
			h=BMPhead.biHeight;
			dw = (w+3) & 0xFFFC;
			size=dw*h;
		} else {
			if ((BMPhead.biHeight!=h) || (BMPhead.biWidth!=w)) err("velikost"); 
		}
		dp = (unsigned char*)malloc(size);
		data[count] = dp;
		count++;
		if (fread(dp,size,1,in) <= 0) err("data");
		for (i=0;i<size;i++) dp[i] = (BMPpal[dp[i]].r & 0xE0) |
			((BMPpal[dp[i]].g & 0xE0) >> 3) | (BMPpal[dp[i]].b >> 6);
		fclose(in);
	}
	fprintf(stderr,"o");
	out = fopen(argv[1],"wb");
	Rhead.type[0]='R';Rhead.type[1]='A';Rhead.type[2]='F';Rhead.type[3]=' ';
	Rhead.version=0x00010000;
	Rhead.headerSize=28;
	Rhead.width=w;
	Rhead.height=h;
	Rhead.speed=0;
	Rhead.frames=count;
	Rhead.comp=0;
	Rhead.index=0;
	Rhead.soundsize=0;
	Rhead.soundcomp=0;
	fwrite(&Rhead,28,1,out);
	for (c=0;c<count;c++) {
		dp=data[c];
		for (i=h-1;i>=0;i--) {
			fwrite(&dp[i*dw],w,1,out);
		}
	}
	fclose(out);
}
