/*******************************************************************************
   fat.c: FAT routines
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Stanislav Sigmund <xsigmu02 AT fit.vutbr.cz>

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$
*******************************************************************************/

#include <fitkitlib.h>
#include <string.h>
#include <stdlib.h>
#include "ide.h"
#include "fat.h"

/****** stavove promenne ******/

/****** funkce ******/

// znamena neznamou hodnotu pro FreeClusters a NxtFree
#define FAT32_NINFO 0xffffffff
// identifikace INFO sektoru na 0. pozici
#define FAT32_I_LEADSIG 0x41615252
// identifikace INFO sektoru na 484. pozici
#define FAT32_I_STRUCSIG 0x61417272

char *FindName = NULL; // jmeno pro hledani v adresari, toleruje i ?*
unsigned short NEW_FILE_DATE;
unsigned short NEW_FILE_TIME;

// obsahuje zakazane znaky ve jmene (+ vsechny znaky <=32)
char *FAT_InvalidChars = "\x22\x2A\x2B\x2C\x2E\x2F\x3A\x3B\x3C\x3D\x3E\x3F\x5B\x5C\x5D\x7C";

// vraci cislo sektoru v ramci 1 clusteru
#define FAT_PartClus(x) ((x->Pos >> 9) & (x->fat->SecPerClus - 1))

// vraci FAT_OK, pokud znak muze byt soucasti jmena
int FAT_Is_Valid_Char(char c) {
	int i = 0;
	if (c <= 32) return FAT_ERR;
	while ((FAT_InvalidChars[i] != 0) && (c != FAT_InvalidChars[i])) i++;
	if (FAT_InvalidChars[i] == 0) return FAT_OK;
	return FAT_ERR;
}

// nacte a zpracuje FAT partition, zacinajici na sektoru offset a disku disk
// polozka f musi existovat
int FAT_Init(tFAT *f,unsigned long offset,unsigned char disk) {
	tBPB B;
	unsigned short TotSec16;
	unsigned short BytsPerSec;
	unsigned short RootEntCnt;
	unsigned long test;
	if (disk > 0) disk = 1;
	if (f == NULL) return FAT_ERR;
	if (IDE_Set_Sector(offset,disk) != IDE_C_OK) return FAT_ERR;
	int i = sizeof(tBPB);
	if (IDE_Read(0,i,(unsigned char*)&B) != i) return FAT_ERR;
	f->StartSec = offset;
	f->drive = disk;
	// procesor ma problemy s nezarovnanymi slovy:
	TotSec16 = B.TotSec16L | (B.TotSec16H << 8);
	BytsPerSec = B.BytsPerSecL | (B.BytsPerSecH << 8);
	RootEntCnt = B.RootEntCntL | (B.RootEntCntH << 8);
	int SecMult = BytsPerSec >> 9;
	f->Sectors = TotSec16;
	if (TotSec16 == 0) f->Sectors = B.TotSec32;
	f->Sectors *= SecMult;
	f->FATSize = B.FATSz16;
	if (B.FATSz16 == 0) f->FATSize = B.FATSz32;
	f->NumFATs = B.NumFATs;
	f->FATSize *= SecMult;
	f->SecPerClus = B.SecPerClus * SecMult;
	f->ExtFlags = B.ExtFlags;
	f->RootDir = (RootEntCnt * 32 + BytsPerSec - 1) / BytsPerSec;
	f->ClusStart = (f->NumFATs * f->FATSize) + (B.RsvdSecCnt + f->RootDir) * SecMult;
	f->Clusters = (f->Sectors - f->ClusStart) / f->SecPerClus;
	f->ClusStart += offset;
	f->StartFAT = offset + B.RsvdSecCnt * SecMult;
	f->FreeClus = FAT32_NINFO; // nezname
	f->NxtFree = FAT32_NINFO; // nezname
	f->FSInfo = 0; // neni
	f->RootDir = f->StartFAT + f->NumFATs * f->FATSize;
	if (f->Clusters < 4085) {
		f->FATType = FAT_12;
		f->ExtFlags = 0;
	} else if (f->Clusters < 65525) {
			f->FATType = FAT_16;
			f->ExtFlags = 0;
		} else {
			f->FATType = FAT_32;
			f->RootDir = B.RootClus;
			if (IDE_Set_Sector(offset + B.FSInfo,disk) != IDE_C_OK) return FAT_OK;
			if ((IDE_Read(0,4,(unsigned char*)&test) != 4) || (test != FAT32_I_LEADSIG)) return FAT_OK;
			if ((IDE_Read(484,4,(unsigned char*)&test) != 4) || (test != FAT32_I_STRUCSIG)) return FAT_OK;
			if (IDE_Read(488,4,(unsigned char*)&(f->FreeClus)) != 4) return FAT_OK;
			if (IDE_Read(492,4,(unsigned char*)&(f->NxtFree)) != 4) return FAT_OK;
			if (B.FSInfo < 255) f->FSInfo = B.FSInfo;
		}
	return FAT_OK;
}

// ulozi udaj o volnem mistu na disk, je treba jen po zapisech
int FAT_Close(tFAT *f) {
	if (f == NULL) return FAT_ERR;
	if (f->FSInfo == 0) return FAT_OK;
	if (IDE_Set_Sector(f->StartSec + f->FSInfo,f->drive) != IDE_C_OK) return FAT_ERR;
	if (IDE_Write(488,4,(unsigned char*)&(f->FreeClus)) != 4) return FAT_ERR;
	if (IDE_Write(492,4,(unsigned char*)&(f->NxtFree)) != 4) return FAT_ERR;
	IDE_Cache_Flush(-1);
	return FAT_OK;
}

// nastavi adresarovy ukazatel dir na RootDir
int FAT_Root_Dir(tFAT *f,tFile *dir) {
	if ((f->FATType != FAT_32) && (f->FATType != FAT_16)) return FAT_ERR;
	dir->fat = f;
	dir->Pos = 0;
	if (f->FATType == FAT_32)	{
		dir->StartClus = f->RootDir;
		dir->CurClus = f->RootDir;
	} else {
		dir->StartClus = 0;
		dir->CurClus = 0;
	}
	dir->PrevClus = 0;
	dir->RootClus = 0;
	dir->attr = ATTR_DIRECTORY;
	dir->FileSize = 0;
	return FAT_OK;
}

// nastavi disk na sektor, odpovidajici aktualnimu clusteru a jeho podsektoru
// osetren pristup k RootDirectory na FAT16 a FAT12
int FAT_Set_Cluster(tFile *f) {
	if ((f == NULL) || (f->fat == NULL) || (f->CurClus > (f->fat->Clusters + 1)) || (f->CurClus == 1)) return FAT_ERR;
	if ((f->fat->FATType != FAT_32) && ((f->attr & ATTR_DIRECTORY) > 0) && (f->CurClus == 0)) {
		if (IDE_Set_Sector(f->fat->RootDir + (f->Pos >> 9),f->fat->drive) == IDE_C_ERR) return FAT_ERR;
		else return FAT_OK; 
	}
	if (f->CurClus == 0) return FAT_EOF;
	if (IDE_Set_Sector((f->CurClus - 2) * f->fat->SecPerClus + f->fat->ClusStart + 
		FAT_PartClus(f),f->fat->drive) == IDE_C_ERR) return FAT_ERR;
	return FAT_OK;
}

// Vyhleda pristi cluster souboru / adresare
int FAT_NextClus(tFile *f) {
	unsigned long fatP;
	if ((f == NULL) || (f->fat == NULL) || (f->CurClus > (f->fat->Clusters + 1)) || (f->CurClus == 1)) return FAT_ERR;
	if ((f->fat->FATType != FAT_32) && ((f->attr & ATTR_DIRECTORY) > 0) && (f->CurClus == 0)) return FAT_OK;
	if (f->CurClus == 0) return FAT_EOF;
	if (f->fat->FATType == FAT_12) return FAT_ERR; // zatim neumi a zrejme ani nebude umet
	if (f->fat->FATType == FAT_32) {
		fatP = f->fat->StartFAT + (f->CurClus >> 7);
		// mirroring je vypnuty, cteme z prave aktivni FAT, jinak z FAT0
		if ((f->fat->ExtFlags & 0x80) > 0) fatP += f->fat->FATSize * f->fat->ExtFlags & 0x0F;
		if (IDE_Set_Sector(fatP,f->fat->drive) != IDE_C_OK) return FAT_ERR;
		f->PrevClus = f->CurClus;
		if (IDE_Read((f->CurClus & 0x7F) << 2,4,(unsigned char*)&(f->CurClus)) != 4) return FAT_ERR;
		f->CurClus &= 0x0FFFFFFF; // nejvyssi 4 bity jsou rezervovany
		if (f->CurClus >= 0x0FFFFFF8) {
			f->CurClus = 0; // EOF znacka (konec retezce)
			return FAT_EOF;
		}
	} else {
		fatP = f->fat->StartFAT + (f->CurClus >> 8);
		if (IDE_Set_Sector(fatP,f->fat->drive) != IDE_C_OK) return FAT_ERR;
		f->PrevClus = f->CurClus;
		f->CurClus = 0;
		if (IDE_Read((f->CurClus & 0xFF) << 1,2,(unsigned char*)&(f->CurClus)) != 2) return FAT_ERR;
		if (f->CurClus >= 0xFFF8) {
			f->CurClus = 0; // EOF znacka (konec retezce)
			return FAT_EOF;
		}
	}
	return FAT_OK; 
}

// nastavi polohu v souboru na zadanou, povolene hodnoty jsou <0;FileSize>
int FAT_Seek(tFile *f,unsigned long pos) {
	// osetrime hazardni stavy
	unsigned long mask;
	int i;
	if ((f == NULL) || (f->fat == NULL)) return FAT_ERR;
	if ((pos > f->FileSize) && ((f->attr & ATTR_DIRECTORY) == 0)) pos = f->FileSize;
	mask = ((f->fat->SecPerClus << 9) - 1) ^ 0xFFFFFFFF; // maska cisla clusteru
	if ((pos & mask) != (f->Pos & mask)) { // zmena clusteru
		if (pos < f->Pos) {
			f->Pos = 0;
			f->CurClus = f->StartClus;
			f->PrevClus = 0;
		}
		mask = (pos & mask) - (f->Pos & mask);
		for (i = 0;i < mask;i++) if (FAT_NextClus(f) != FAT_OK) return FAT_ERR;
	}
	f->Pos = pos;
	return FAT_OK;
}

// precte length bytu dat ze souboru (adresare) do buff, vraci pocet uspesne nactenych bytu
// buff==NULL -> zapis/cteni z DMA kanalu
int FAT_Read(tFile *f,unsigned short length,unsigned char *buff) {
	unsigned short left = length;
	unsigned short count;
	// osetrime hazardni stavy
	if ((f == NULL) || (f->fat == NULL)) return FAT_ERR;
	if (length == 0) return 0; // neni potreba nic nacitat
	int status;
	while (left > 0) {
		if ((f->CurClus > (f->fat->Clusters + 1)) || (f->CurClus == 1)) return FAT_ERR;
		if (f->CurClus == 0) return FAT_EOF;
		if (((f->attr & ATTR_DIRECTORY) == 0) && (f->Pos >= f->FileSize)) break;
		count = 512 - (f->Pos & 511);
		if (left < count) count = left;
		if (((f->attr & ATTR_DIRECTORY) == 0) && ((f->Pos + count) > f->FileSize))
			count = f->FileSize - f->Pos; // orezani na konec souboru
		if (count == 0) break;
		// nastavime spravny cluster a sektor
		if (FAT_Set_Cluster(f) == FAT_ERR) return FAT_ERR;
		if (IDE_Read(f->Pos & 511,count,buff) != count) return FAT_ERR;

		left -= count;
		f->Pos += count;
		if (buff != NULL) buff += count;
		// doslo ke skoku na dalsi cluster
		if (((f->Pos & 511) == 0) && (FAT_PartClus(f) == 0)) 
			if ((status = FAT_NextClus(f)) == FAT_ERR) return FAT_ERR;
			if (status == FAT_EOF) break; // necekany konec souboru?  
	}
	if (length == left) return FAT_EOF; // konec souboru
			else return length - left; // kolik bylo nacteno dat
}

int FAT_SetFATEntry(tFAT *fat,unsigned long clus,unsigned long value) {
	unsigned long fatP;
	unsigned long prev;
	int i = 1;
	if ((fat == NULL) || (clus < 2) || (clus > (fat->Clusters + 1))) return FAT_ERR;
	if (fat->FATType != FAT_32) return FAT_ERR;
	fatP = fat->StartFAT + (clus >> 7);
	// mirroring je vypnuty, zapisujeme do prave aktivni FAT, jinak do vsech
	if ((fat->ExtFlags & 0x80) > 0) fatP += fat->FATSize * fat->ExtFlags & 0x0F;
	else i = fat->NumFATs;
	if (IDE_Set_Sector(fatP,fat->drive) != IDE_C_OK) return FAT_ERR;
	if (IDE_Read((clus & 0x7F) << 2,4,(unsigned char*)&prev) != 4) return FAT_ERR;
	value = (value & 0x0FFFFFFF) | (prev & 0xF0000000); // nejvyssi 4 bity jsou tabu
	for (;i > 0;i--) {
		if (IDE_Set_Sector(fatP,fat->drive) != IDE_C_OK) return FAT_ERR;
		if (IDE_Write((clus & 0x7F) << 2,4,(unsigned char*)&value) != 4) return FAT_ERR;
		fatP += fat->FATSize; // dalsi FAT
	}
	return FAT_OK;
}

// pokusi se najit volny cluster a nastavit jej na znacku EOS (konec retezce)
// vraci cislo clusteru, nebo 0, pokud nastala chyba, nebo 1, pokud neni volne misto
unsigned long FAT_Alloc_New_Clus(tFAT *fat) {
	unsigned long fatP;
	unsigned long clus;
	unsigned long fstclus;
	unsigned long value;
	if (fat == NULL) return FAT_ERR;
	if (fat->FATType != FAT_32) return FAT_ERR;
	clus = fat->NxtFree; // doporuceny zacatek hledani
	if (clus > (fat->Clusters + 1)) clus = 2;
	fstclus = clus; // prvni hledany cluster, je urcen pro ukonceni hledani
	do { 
		fatP = fat->StartFAT + (clus >> 7);
		// mirroring je vypnuty, cteme z prave aktivni FAT, jinak z FAT0
		if ((fat->ExtFlags & 0x80) > 0) fatP += fat->FATSize * fat->ExtFlags & 0x0F;
		if (IDE_Set_Sector(fatP,fat->drive) != IDE_C_OK) return 0;
		
		if (IDE_Read((clus & 0x7F) << 2,4,(unsigned char*)&value) != 4) return 0;
		if ((value & 0x0FFFFFFF) == 0) { // volny cluster
			fat->NxtFree = clus;
			if (FAT_SetFATEntry(fat,clus,0x0FFFFFFF) != FAT_OK) return 0; // EOS
			if (fat->FreeClus != 0xFFFFFFFF) fat->FreeClus--;
			return clus;
		}
		
		clus++;
		if (clus > (fat->Clusters + 1)) clus = 2;
	} while (clus != fstclus); // prosli jsme vsechny mozne clustery
	return 1; // neni volne misto
} 

// nastavi polozku v adresari na aktualni hodnoty
int FAT_UpdateDirEntry(tFile *f) {
	tFile dir;
	tDirEntry DE;
	if ((f == NULL) || (f->fat == NULL) || (f->RootClus < 2) || 
		(f->RootClus > (f->fat->Clusters + 1))) return FAT_ERR;
	memset((unsigned char*)&dir,0,sizeof(dir));
	dir.StartClus = f->RootClus;
	dir.CurClus = f->RootClus;
	dir.attr = ATTR_DIRECTORY;
	dir.fat = f->fat;

	if (FAT_Seek(&dir,(f->RootIndex - 1) << 5) != FAT_OK) return FAT_ERR;
	if (FAT_Read(&dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
	if ((FAT_Is_Valid_Char(DE.Name[0]) != FAT_OK) && (DE.Name[0] != 5)) return FAT_ERR;
	if ((f->FileSize == -1) && (f->Pos == -1)) DE.Name[0] = 0xE5; // vymazani souboru
	else {
		DE.WrtTime = NEW_FILE_TIME;
		DE.WrtDate = NEW_FILE_DATE;
		DE.LstAccDate = NEW_FILE_DATE;
		DE.FstClusL = f->StartClus & 0x0000FFFF;
		DE.FstClusH = f->StartClus >> 16;
		DE.FileSize = f->FileSize;
		DE.Attr = f->attr;
	}
	if ((f->attr & ATTR_DIRECTORY) > 0) DE.FileSize = 0;
	if (FAT_Seek(&dir,(f->RootIndex - 1) << 5) != FAT_OK) return FAT_ERR;
	if (FAT_Write(&dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
	return FAT_OK;
}

// Alokuje dalsi cluster pro soubor/adresar, musi se nachazet na konci retezce
int FAT_AppendClus(tFile *f) {
	unsigned long newClus;
	if ((f == NULL) || (f->fat == NULL) || (f->CurClus != 0) || (f->StartClus == 1)) return FAT_ERR;
	if (f->StartClus == 0) { // prvni cluster souboru
		newClus = FAT_Alloc_New_Clus(f->fat); // najde volny cluster a nastavi jej na EOS
		if (newClus == 0) return FAT_ERR; 
		if (newClus == 1) return FAT_EOF;
		f->StartClus = newClus;
		f->CurClus = newClus;
		f->PrevClus = 0;
		return FAT_UpdateDirEntry(f); // nastavime prvni cislo prvniho Clusteru v adresari
	} else {
		newClus = FAT_Alloc_New_Clus(f->fat);
		if (newClus == 0) return FAT_ERR; 
		if (newClus == 1) return FAT_EOF;
		if (f->PrevClus < 2) f->PrevClus = f->StartClus;
		f->CurClus = newClus;
		return FAT_SetFATEntry(f->fat,f->PrevClus,newClus); // nastavime odkaz v retezi 
	}
}

// zapise length bytu do souboru, zvetsi soubor, pokud je priliz maly
// je nutne zapnout pamet cache, jinak nebude zapis funkcni 
// buff==NULL -> zapis/cteni z DMA kanalu
int FAT_Write(tFile *f,unsigned short length,unsigned char *buff) {
	unsigned short left = length;
	unsigned short count;
	int status;
	// osetrime hazardni stavy
	if ((f == NULL) || (f->fat == NULL)) return FAT_ERR;
	if (length == 0) return 0; // neni potreba nic nacitat
	if (((f->attr & ATTR_DIRECTORY) == 0) && (f->Pos > f->FileSize)) return FAT_ERR;
	while (left > 0) {
		if ((f->CurClus > (f->fat->Clusters + 1)) || (f->CurClus == 1)) return FAT_ERR;
		if (f->CurClus == 0) {
			if ((status = FAT_AppendClus(f)) != FAT_OK) return status;
			continue;
		}
		if (FAT_Set_Cluster(f) == FAT_ERR) return FAT_ERR;
		count = 512 - (f->Pos & 511);
		if (left < count) count = left;
		if (count == 0) break;
		if (IDE_Write(f->Pos & 511,count,buff) != count) return FAT_ERR;

		left -= count;
		f->Pos += count;
		if (((f->attr & ATTR_DIRECTORY) == 0) && (f->Pos > f->FileSize)) f->FileSize = f->Pos;
		if (buff != NULL) buff += count;
		// doslo ke skoku na dalsi cluster
		if (((f->Pos & 511) == 0) && (FAT_PartClus(f) == 0)) 
			if ((status = FAT_NextClus(f)) == FAT_ERR) return FAT_ERR;
	}
	return length - left; // kolik bylo zapsano dat
}

// vytvori soubor v zadanem adresari, polozka f, pokud je nenulova,
// obsahuje inicializovany soubor nulove delky
// pouziva hledani souboru, takze se nelze vratit zpet k hledani! 
// pokud soubor existuje, vraci FAT_ERR! (Je nutne pouzit fci FAT_Clear)
// jmeno musi obsahovat pouze povolene znaky!
int FAT_Create_File(tFile *dir,char *name,unsigned char attr,tFile *f) {
	tDirEntry DE;
	tFile ndir;
	int i,i1,last = 0;
	if ((dir == NULL) || (dir->fat == NULL) || ((dir->attr & ATTR_DIRECTORY) == 0) 
		|| (name == NULL) || ((attr & ATTR_VOLUME_ID) == ATTR_VOLUME_ID)) return FAT_ERR;
	FAT_Set_Find(dir,name);
	if (FAT_Find_Next(dir,NULL,NULL,NULL,NULL) != FAT_EOF) return FAT_ERR;
	dir->CurClus = dir->StartClus;
	dir->PrevClus = 0;
	dir->Pos = 0;
	dir->FileSize = 0;
	
	do { 
		if (FAT_Read(dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
		if (DE.Name[0] == 0xE5) { // volna polozka
#ifdef FILL_FREE_ITEMS
			break;
#else
			continue;
#endif
		}
		if (DE.Name[0] == 0) {
			last = 1;
			break; // posledni polozka - lze ji prepsat
		}
	} while (1);
	if (FAT_Seek(dir,dir->Pos - 32) != FAT_OK) return FAT_ERR;
	memset((unsigned char*)&DE,0,sizeof(DE));
	DE.Attr = attr;
	DE.CrtTime = NEW_FILE_TIME;
	DE.WrtTime = NEW_FILE_TIME;
	DE.CrtDate = NEW_FILE_DATE;
	DE.WrtDate = NEW_FILE_DATE;
	DE.LstAccDate = NEW_FILE_DATE;
	memset((unsigned char*)&DE.Name,32,8); //prazdne polozky ve jmene jsou mezery
	memset((unsigned char*)&DE.Ext,32,3);
	i = 0;
	while ((i < 8) && (name[i] != '.') && (name[i] != 0)) {
		if (FAT_Is_Valid_Char(name[i]) != FAT_OK) return FAT_ERR;
		DE.Name[i] = name[i];
		i++;
	}
	if (name[i] == '.') {
		i++;
		i1 = 0;
		while ((i1 < 3) && (name[i] != 0)) {
			if (FAT_Is_Valid_Char(name[i]) != FAT_OK) return FAT_ERR;
			DE.Ext[i1] = name[i];
			i++;
			i1++;
		}
	}
	if (DE.Name[0] == 0xE5) DE.Name[0] = 5;
	if (FAT_Write(dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
	i = dir->Pos >> 5;
	if (last) {
		memset((unsigned char*)&DE,0,sizeof(DE));
		if (FAT_Write(dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
		dir->FileSize = 1; // EOS
	}
	ndir.StartClus = 0; 
	ndir.CurClus = 0;
	ndir.PrevClus = 0;
	ndir.RootClus = dir->StartClus;
	ndir.attr = attr;
	ndir.Pos = 0;
	ndir.FileSize = 0;
	ndir.RootIndex = i;
	ndir.fat = dir->fat;
	if (f != NULL) memcpy((char*)f,(char*)&ndir,sizeof(tFile));
	if ((attr & ATTR_DIRECTORY) > 0) { // je nutne jej inicializovat
		memset((unsigned char*)&DE,0,sizeof(DE));
		DE.Attr = ATTR_DIRECTORY;
		DE.CrtTime = NEW_FILE_TIME;
		DE.WrtTime = NEW_FILE_TIME;
		DE.CrtDate = NEW_FILE_DATE;
		DE.WrtDate = NEW_FILE_DATE;
		DE.LstAccDate = NEW_FILE_DATE;
		memset((unsigned char*)&DE.Name,32,8); //prazdne polozky ve jmene jsou mezery
		memset((unsigned char*)&DE.Ext,32,3);
		DE.Name[0] = '.';
		if (FAT_AppendClus(&ndir) != FAT_OK) return FAT_ERR; // je nutne pripojit prvni cluster
		 // - abychom mohli ulozit odkaz "sami na sebe" (dot entry)
		DE.FstClusL = ndir.StartClus & 0x0000FFFF;
		DE.FstClusH = ndir.StartClus >> 16;
		if (f != NULL) {
			f->StartClus = ndir.StartClus;
			f->CurClus = ndir.CurClus;
		}
		if (FAT_Write(&ndir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
		DE.Name[1] = '.'; // dotdot entry - odkaz na rod. adresar
		DE.FstClusL = ndir.RootClus & 0x0000FFFF;
		DE.FstClusH = ndir.RootClus >> 16;
		if (FAT_Write(&ndir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
		memset((unsigned char*)&DE,0,sizeof(DE)); // vynulujeme zbytek adresare
		for (i = dir->fat->SecPerClus * 16 - 2;i > 0;i--) 
			if (FAT_Write(&ndir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
	}
	return FAT_OK;
}

// uzavre soubor, ulozi neulozena data, pokud bylo pouze cteno, neni treba ji volat 
inline int FAT_Close_File(tFile *f) {
	if (FAT_UpdateDirEntry(f) != FAT_OK) return FAT_ERR;
	if (IDE_Cache_Flush(4) != IDE_C_OK) return FAT_ERR;
	return FAT_OK;
}

// Nastavi delku souboru na 0, uvolni misto
int FAT_Clear(tFile *f) {
	unsigned long prev;
	if ((f == NULL) || (f->fat == NULL) || (f->StartClus == 1) || 
		(f->StartClus > (f->fat->Clusters + 1))) return FAT_ERR;
	if (f->StartClus == 0) return FAT_OK;
	f->CurClus = f->StartClus;
	f->StartClus = 0;
	f->FileSize = 0;
	f->Pos = 0;
	while (f->CurClus >= 2) {
		prev = f->CurClus;
		if (FAT_NextClus(f) == FAT_ERR) return FAT_ERR;
		if (FAT_SetFATEntry(f->fat,prev,0) == FAT_ERR) return FAT_ERR;
		if (f->fat->FreeClus != 0xFFFFFFFF) f->fat->FreeClus++;
	}
	f->CurClus = 0;
	f->PrevClus = 0;
	return FAT_Close_File(f);
}

// Vymaze soubor (i adresar, pokud je prazdny)
// vraci FAT_EOF, pokud je mazany adresar neprazdny
// Pouziva hledani souboru, takze po jeho provedeni nelze pokracovat v predchozim hledani!
int FAT_Delete_File(tFile *f) {
	char name[13];
	if ((f == NULL) || (f->fat == NULL)) return FAT_ERR;
	if ((f->attr & ATTR_DIRECTORY) > 0) { // adresar
		FAT_Set_Find(f,NULL);
		int status;
		while (((status = FAT_Find_Next(f,name,NULL,NULL,NULL)) == FAT_OK) && (name[0] == '.'));
		if (status == FAT_OK) return FAT_EOF;
		if (status != FAT_EOF) return FAT_ERR; // neprazdny adresar
	}
	if (FAT_Clear(f) != FAT_OK) return FAT_ERR;
	f->FileSize = -1;
	f->Pos = -1;
	return FAT_Close_File(f);
}

inline char toupper(char c) {
	return ((c>='a') && (c<='z')) ? c : c-32;
}

// porovna 2 retezce, 2. retezec muze obsahovat znaky ? a *, je ovlivnen znakem  
// FAT_CASE_SENSITIVE, vraci FAT_OK pri splneni podminky
int FAT_Cmp(char *name,char *Fname) {
	int i = 0, iF = 0;
	if ((name == NULL) || (Fname == NULL)) return FAT_OK;
	while ((name[i] != 0) && (Fname[iF] != 0) && ((Fname[iF] == '?') || 
		(Fname[iF] == '*') || (toupper(name[i]) == toupper(Fname[iF])))) {
			i++;
			if ((Fname[iF] != '*') || (name[i] == '.')) iF++;
		}
	if (((name[i] == 0) && (Fname[iF] == 0)) || ((Fname[iF] == '*') && 
		((Fname[iF + 1] != '.') || (Fname[iF + 2] == 0) || (Fname[iF + 2] == '*')))) return FAT_OK;
	return FAT_ERR;
}

// nastavi vyhledavani podle retezce, umoznuje pokracovani ve vyhledavani
inline void FAT_Set_Find_Continue(tFile *dir,char *name) {
	if ((dir->attr & ATTR_DIRECTORY) == 0) return;
	FindName = name;
}

// zapocne vyhledavani na adresari dir podle retezce name (NULL = *.*)
void FAT_Set_Find(tFile *dir,char *name) {
	if ((dir->attr & ATTR_DIRECTORY) == 0) return;
	dir->CurClus = dir->StartClus;
	dir->PrevClus = 0;
	dir->Pos = 0;
	dir->FileSize = 0;
	FindName = name;
}

// nalezne nasledujici polozku, ulozi jeji jmeno a atributy
// pole Fname musi byt min. 13B velke
// do ext zapise index koncovky, pokud neni NULL
// vraci FAT_OK, nebo FAT_EOF, pokud uz zadna dalsi neni
int FAT_Find_Next(tFile *dir,char *Fname,unsigned char *ext,unsigned char *attr,tFile *file) {
	char name[13];
	tDirEntry DE;
	int i,ii;
	if ((dir->attr & ATTR_DIRECTORY) == 0) return FAT_ERR;
	if (dir->FileSize > 0) return FAT_EOF; // nelze dale hledat
	do { 
		if (FAT_Read(dir,32,(unsigned char*)&DE) != 32) return FAT_ERR;
		if (DE.Name[0] == 0) { // posledni polozka v souboru
			dir->FileSize = 1; // u adresare znamena konec hledani
			return FAT_EOF;
		} // uvolnene polozky (0xE5) a dlouha jmena preskakujeme
		if ((DE.Name[0] == 0xE5) || ((DE.Attr & ATTR_LONG_NAME) == ATTR_LONG_NAME)) continue;
		// uprava jmena souboru
		i = 7;
		memcpy(name,&DE.Name,8); 
		while ((name[i] == 32) && (i >= 0)) i--;
		i++;
		name[i] = '.';
		i++;
		if (name[0] == 5) name[0] = 0xE5; // ASCII znak 0xE5 je nahrazen symbolem 5 
		if (ext != NULL) *ext = i;
		memcpy(&name[i],&DE.Ext,3);
		ii = i + 2;
		while ((name[ii] == 32) && (ii >= i)) ii--;
		name[ii + 1] = 0;
		if (ii < i) name[ii] = 0; // neni koncovka
		if (Fname != NULL) strcpy(Fname,name);
		if (FAT_Cmp(name,FindName) == FAT_OK) {
			if (attr != NULL) *attr = DE.Attr;
			if (file != NULL) {
				file->StartClus = DE.FstClusH; 
				file->StartClus = file->StartClus << 16 | DE.FstClusL;
				file->CurClus = file->StartClus;
				file->PrevClus = 0;
				file->RootClus = dir->StartClus;
				file->attr = DE.Attr;
				file->Pos = 0;
				file->FileSize = DE.FileSize;
				file->RootIndex = dir->Pos >> 5;
				file->fat = dir->fat;
			}
			return FAT_OK; // Pozice polozky
		}
	} while (1);
}

// pokud je fat == NULL pak pouze vraci pocet partition na disku disc
// jinak se pokusi zinicializovat fat tabulku o indexu part
// pokud volume != NULL, pak zde ulozi jmeno svazku (min.13 B)
// zatim nezpracovava rozsirene oblasti
int FAT_init_Part(unsigned char part,tFAT *fat,char *volume,unsigned char disc) {
	tPartition ID;
	tFile RDir;
	tFAT f;
	int ii = 10,i = -1;
	int count = 0;
	unsigned char attr;
	int status;
	IDE_Change_Disc(disc);
 	while ((ii >= 0) && (IDE_Wait_Status(STATUS_DRDY + STATUS_DSC,STATUS_BSY,1,1000) == IDE_RET_COUNT)) {
		ii--;
		WDG_reset();
	}
	if (ii < 0) return FAT_ERR;
	if (IDE_Set_Sector(0,disc) != IDE_C_OK) return FAT_ERR;
	unsigned short BRS = 0;
	if ((IDE_Read(510,2,(unsigned char*)&BRS) != 2) || (BRS != BRS_NUM)) return FAT_ERR;
	while (i < 3) {
		i++;
		if (IDE_Read(446 + i * 16,16,(unsigned char*)&ID) != 16) return FAT_ERR;
		if ((ID.Active > 0) && ((ID.type == PAR_TYPE_FAT32) || 
			(ID.type == PAR_TYPE_FAT32_LBA))) {
			if (FAT_Init(&f,ID.offset,disc) != FAT_OK) continue;
			count++;
			if ((fat != NULL) && ((part + 1) == count)) {
				memcpy(fat,&f,sizeof(tFAT));
				if (volume != NULL) {
					if (FAT_Root_Dir(fat,&RDir) != FAT_OK) return FAT_ERR; 
					FAT_Set_Find(&RDir,NULL); // NULL = '*'
					while (((status = FAT_Find_Next(&RDir,volume,NULL,&attr,NULL)) == FAT_OK) && 
						((attr & ATTR_VOLUME_ID) == 0)) WDG_reset();
					if (status != FAT_OK) volume[0] = 0; 
				}
				return FAT_OK;
			}
		}
	}
	if (fat != NULL) return FAT_ERR;
	return count;
}
