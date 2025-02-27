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
	FILE *in = stdin,*out = stdout;
	if (argc>0) in = fopen(argv[1],"rb");
	if (argc>1) out = fopen(argv[2],"wb");
					
	tBMPHead BMPhead;
	tBMPpalList BMPpal[256];
	
	if (fread(&BMPhead,2,1,in) <= 0) err("hlavicka souboru");
	if ((BMPhead.bfType[0] != 'B') || (BMPhead.bfType[1] != 'M')) err("typ souboru");
	if (fread(&(BMPhead.bfSize),52,1,in) <= 0) err("hlavicka souboru");
	if (BMPhead.biCompression != 0) err("komprese");
	if (fread(&BMPpal,1024,1,in) <= 0) err("paleta");
	fseek(in,BMPhead.bfOffBits,SEEK_SET);
	fprintf(stderr,"%i\n",BMPhead.bfOffBits);
	unsigned short dw = (BMPhead.biWidth+3) & 0xFFFC; 
	unsigned long size=dw*BMPhead.biHeight;
	unsigned char *data = (unsigned char*)malloc(size);
	if (fread(data,size,1,in) <= 0) err("data");
	if (argc>0) fclose(in);
	int i;
	for (i=0;i<size;i++) data[i] = (BMPpal[data[i]].r & 0xE0) |
		((BMPpal[data[i]].g & 0xE0) >> 3) | (BMPpal[data[i]].b >> 6);
	char *idstr="RIF ";
	fwrite(idstr,4,1,out);
	unsigned short temp = BMPhead.biWidth;
	fwrite(&temp,2,1,out);
	temp = BMPhead.biHeight;
	fwrite(&temp,2,1,out);
	for (i=BMPhead.biHeight-1;i>=0;i--) {
		fwrite(&data[i*dw],BMPhead.biWidth,1,out);
	}
	if (argc>1) fclose(out);
	exit(0);
}
