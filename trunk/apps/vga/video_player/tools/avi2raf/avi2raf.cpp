#include <vfw.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifndef max
#define max(a,b)			(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)			(((a) < (b)) ? (a) : (b))
#endif

// prototypy
void OpenAVI(const char *FileName);
void CloseAVI();

//------------------------------------------------------------------------------
PAVIFILE AVIFile = NULL;
PAVISTREAM VideoStream = NULL;
PGETFRAME GetFrame = NULL;
BITMAPINFO *Frame = NULL;

typedef struct{
	unsigned char type[4]; // "RAFF"
	unsigned long version; // zatim jen 0x00010000
	unsigned short headerSize; // velikost hlavicky
	unsigned short width;
	unsigned short height;
	unsigned short speed; // 0-bez omezeni, bude 1=25FPS atd...
	unsigned long frames; // pocet snimku animace
	unsigned short comp; // typ komprese - 0=hrube snimky za sebou
		// 1=typ komprese je v hlavicce(bez kopirovani bloku) 2=jako 1, ale s kopirovanim bloku
	unsigned short index; // pritomnost indexu snimku za hlavickou, zatim 0-bez indexu
	unsigned short soundsize; // velikost jednotlivych bloku
	unsigned short soundcomp; // typ komprese zvuku
} tRAFFHead;

//------------------------------------------------------------------------------
// jadro aplikace
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// zacatek programu
int main(int argc, char **argv)
{
	if (argc<3) {
		printf("Program na konverzi video souboru do formatu raf\n");
		printf("Priklad pouziti:\navi2raf zdrojovy_avi_soubor cilovy_raf_soubor\n");
		exit(1);
	}
	AVIFileInit();

	int komp=0;
//	if (argc>3) komp=1;
	OpenAVI(argv[1]);
	tRAFFHead Rhead;

  AVIFILEINFO aviinfo;
  AVIFileInfo(AVIFile, &aviinfo, sizeof(aviinfo));
  printf("Rozliseni videa: %ld x %ld\n",aviinfo.dwWidth,aviinfo.dwHeight);
  float framerate = (double)aviinfo.dwRate / (double)aviinfo.dwScale;
  printf("Framerate: %.2f\n",framerate);
	
	long f;
	int w=0,h=0,i,x,y;
	unsigned char *data=NULL;
	unsigned char *col;
/*	unsigned char *dataC=NULL;
	int pos,cnt;
	unsigned char lastcol;*/
	FILE *out = fopen(argv[2],"wb");
	long maxf=AVIStreamStart(VideoStream) + AVIStreamLength(VideoStream)-1;
	for (f=AVIStreamStart(VideoStream);f<maxf;f++) {
	  Frame = (BITMAPINFO*)AVIStreamGetFrame(GetFrame,f);
	  if (f==AVIStreamStart(VideoStream)) {
	  	w = Frame->bmiHeader.biWidth;
			h = Frame->bmiHeader.biHeight; 
		  printf("Snimku %ld, BitCount %i\n",AVIStreamLength(VideoStream),Frame->bmiHeader.biBitCount);
		
			Rhead.type[0]='R';Rhead.type[1]='A';Rhead.type[2]='F';Rhead.type[3]=' ';
			Rhead.version=0x00010000;
			Rhead.headerSize=28;
			Rhead.width=w;
			Rhead.height=h;
			Rhead.speed=0;
			Rhead.frames=AVIStreamLength(VideoStream)-1;
			Rhead.comp=komp;
			Rhead.index=0;
			Rhead.soundsize=0;
			Rhead.soundcomp=0;
			fwrite(&Rhead,28,1,out);
			data =(unsigned char*)malloc(w*h);
		}
		col = (unsigned char*)&(Frame->bmiColors);
		i=0;
		for (y=0;y<h;y++) {
			for (x=0;x<w;x++) {
				data[(h-y-1)*w+x]=(col[i*3] & 0xE0) | ((col[i*3+1] & 0xE0) >> 3) | ((col[i*3+2] & 0xC0) >> 6);
				i++;
			}
		}
		fwrite(data,w*h,1,out);
/*		if (komp==0) fwrite(data,w*h,1,out);
		else {
			pos=0;lastcol=0;cnt=0;
			for (i=0;i<w*h;i++) {
				
			
			
			}
			fwrite(dataC,w*h,1,out);
		}*/
	}
	fclose(out);
  
	CloseAVI();

	AVIFileExit();
	return 0;
}

void OpenAVI(const char *FileName)
{
  AVIFILEINFO aviinfo;
	int hr = AVIFileOpen(&AVIFile, FileName, OF_READ, 0L);
	if (hr == 0)
	{
    if ((hr = AVIFileInfo(AVIFile, &aviinfo, sizeof(aviinfo))) != 0) {
  		fprintf(stderr,"Nepodarilo se precist hlavicku!\nChyba %d.", hr);
  		return;
    }

    if ((hr = AVIFileGetStream(AVIFile, &VideoStream,streamtypeVIDEO,0)) != 0) {
  		fprintf(stderr,"Nepodarilo se nacist video stream!\nChyba %d.", hr);
  		return;
    }
    GetFrame = AVIStreamGetFrameOpen(VideoStream,NULL);
  }
}

//------------------------------------------------------------------------------
void CloseAVI()
{
  if (GetFrame != NULL) {
    AVIStreamGetFrameClose(GetFrame);
    GetFrame = NULL;
  }
  if (VideoStream != NULL) {
    AVIStreamRelease(VideoStream);
    VideoStream = NULL;
  }
	/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
	if (AVIFile != NULL)
	{
		AVIFileRelease(AVIFile);
	}

	GetFrame = NULL;
	VideoStream = NULL;
	AVIFile = NULL;
	Frame = NULL;
}
