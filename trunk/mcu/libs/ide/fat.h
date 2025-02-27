/*******************************************************************************
   fat.h: FAT routines
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
#ifndef _FAT_
#define _FAT_

#define BRS_NUM 0xAA55

// zadny / neznamy
#define PAR_TYPE_NONE 0
// FAT12
#define PAR_TYPE_FAT12 1
// FAT16 < 32MB
#define PAR_TYPE_SM_FAT16 4
#define PAR_TYPE_EXTENDED 5
// FAT16 > 32MB
#define PAR_TYPE_FAT16 6
// FAT32 < 2GB
#define PAR_TYPE_FAT32 0x0b
// LBA verze
#define PAR_TYPE_FAT32_LBA 0x0c
#define PAR_TYPE_FAT16_LBA 0x0e
#define PAR_TYPE_EXTENDED_LBA 0x0f


#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define ATTR_LONG_NAME 0x0F


#define FAT_32 1
#define FAT_16 2
#define FAT_12 3

#define FAT_OK 0
#define FAT_ERR -1
#define FAT_EOF -2
#define FAT_FULL -3


typedef struct {
	unsigned char Active; // 0x80  = active, 0 = inactive
	unsigned char BHead; // hlava zacaku partition 
	unsigned short BCylSec; // cylinder/sektor zacaku partition 
	unsigned char type; // typ partition
	unsigned char EHead; // hlava konce partition 
	unsigned short ECylSec; // cylinder/sektor konce partition 
	unsigned long offset; // pocet sektoru mezi MBR a 0. sektorem partition 
	unsigned long sectors; // pocet sektoru partition 
} tPartition;

typedef struct {
	unsigned char code[446];
	tPartition Partitions[4];
	unsigned short BRS; // Boot Sector Signature = BRS_NUM (0xAA55) 
} tMBR;

typedef struct {
	unsigned char JumpCode[3]; // adresa pro skok
	char OEMName[8]; // neukoncene, obsahuje nejcasteji MSWIN4.1
//	unsigned short BytsPerSec; // 512,1024,2048,4096 - bytu na sektor
	unsigned char BytsPerSecL; // dolni cast slova, kvuli problemum se zarovnavanim
	unsigned char BytsPerSecH; // horni cast slova, kvuli problemum se zarovnavanim
	unsigned char SecPerClus; // sektoru na cluster BytsPerSec * SecPerClus <=32K
	unsigned short RsvdSecCnt; // Rezervovanych sektoru, bezne  FAT12,16 = 1, FAT32 = 32
	unsigned char NumFATs; // pocet FAT, bezne = 2
//	unsigned short RootEntCnt; // pocet polozek v Root Directory, FAT32 = 0
	unsigned char RootEntCntL; // dolni cast slova, kvuli problemum se zarovnavanim
	unsigned char RootEntCntH; // horni cast slova, kvuli problemum se zarovnavanim
//	unsigned short TotSec16; // celkovy pcet sektoru, pokud = 0, pak se bere v uvahu TotSec32
	unsigned char TotSec16L; // dolni cast slova, kvuli problemum se zarovnavanim
	unsigned char TotSec16H; // horni cast slova, kvuli problemum se zarovnavanim
	unsigned char Media; // typ disku, fixni = 0xF8
	unsigned short FATSz16; // Pocet sektoru / 1 FAT, FAT32: = 0 -> FATSz32
	unsigned short SecPerTrck; // Sektoru na stopu, nepotrebne
	unsigned short NumHeads; // Hlavicek
	unsigned long HiddSec; // skrytych sektoru, nepotrebne
	unsigned long TotSec32; // celkovy pcet sektoru, pokud = 0, pak se bere v uvahu TotSec16
/**** nasledujici cast je platna pouze ve FAT32 ****/
	unsigned long FATSz32; // Pocet sektoru / 1 FAT, FAT16: = 0 -> FATSz16
	unsigned short ExtFlags; // bity 0-3 = cislo aktivni FAT, bit 7 = 1 pouze jedna FAT aktivni, = 0 mirroring FAT 		 
	unsigned short FSVer; // Verze FAT, pouze 0
	unsigned long RootClus; // CLuster root directory
	unsigned short FSInfo; // sektor Info sektoru, obvykle 1
	unsigned short BkBootSec; // sektor zalozniho boot sektoru - nepotrebne
	unsigned char Reserved[12];
	unsigned char DrvNum; // cislo disku, od 0x80 zacinaji disky, zavysle na op. syst. - nepotrebne
	unsigned char Reserved1;
	unsigned char BootSig; // indikuje pritomnost nasledujicich 3 poli:
	unsigned long VolID; // cislo svazku, pro detekci vyjmuti, obvykle z aktualniho data a casu
	char VolLab[11]; // jmeno svazku, melo by byt aktualizovano p zmene jmena svazku v 
		// Root Dir, muze obsahovat "NO NAME    " - bez jmena
	char FilSysType[8]; // obvykle "FAT32   "
} __attribute__((__packed__)) tBPB;

typedef struct {
	// nasledujici udaje jsou prepocitany na sektor vel. 512 bytu, adresace je prepocitana od zacatku disku
	unsigned long StartSec; // prvni sektor Partition
	unsigned long Sectors; // celkovy pocet sektoru
	unsigned long Clusters; // celkovy pocet clusteru
	unsigned long FATSize; // velikost FAT v sektorech
	unsigned char drive; // cilso disku na radici = 0,1
	unsigned char SecPerClus;
	unsigned char FATType; 
	unsigned char NumFATs; // pocet FAT, bezne = 2
	unsigned long RootDir; // FAT16 - sektor Root Directory, FAT32 - cluster RD
	unsigned long ClusStart; // cislo sektoru 2. clusteru (1. cluster v systemu)
	unsigned long StartFAT; // Start FAT
	unsigned char ExtFlags; // bity 0-3 = cislo aktivni FAT, bit 7 = 1 pouze jedna FAT aktivni, = 0 mirroring FAT 		 
	unsigned char FSInfo; // delta pozice FSInfo tabulky
	unsigned long FreeClus; // pocet volnych clusteru, 0xffffffff = N/A
	unsigned long NxtFree; // doporucene misto hledani volneho clusteru, 0xffffffff = N/A
} tFAT;

typedef struct {
	unsigned long StartClus; // prvni cluster souboru
	unsigned long RootClus; // cluster rodicovskeho adresare, 0 = rodicovsky adresar
	unsigned long CurClus; // aktualni cluster
	unsigned long PrevClus; // predchozi cluster 
	unsigned short RootIndex; // pozice v adresari
	unsigned char attr; // atributy
	unsigned char ClusInRow; // Pocet clusteru v "rade"
	unsigned long Pos; // aktualni pozice v souboru - cislo bytu, v pripade adresare - cislo polozky
	unsigned long FileSize; // delka souboru
	tFAT *fat;
} tFile; // Pro soubor, ale i pro adresar

typedef struct {
	unsigned char Name[8]; // jmeno souboru
	unsigned char Ext[3]; // koncovka souboru
	unsigned char Attr; // atributy polozky
	unsigned char NTRes; // pro Win NT
	unsigned char CrtTimeTenth; // setiny casu pro 2 vteriny
	unsigned short CrtTime; // aktualni cas
	unsigned short CrtDate; // aktualni datum
	unsigned short LstAccDate; // datum posledniho pristupu - cteni a zapisu
	unsigned short FstClusH; // horni slovo prvniho clusteru (pouze FAT32)
	unsigned short WrtTime; // cas posledniho zapisu
	unsigned short WrtDate; // datum posledniho zapisu
	unsigned short FstClusL; // dolni slovo prvniho clusteru
	unsigned long FileSize; // delka souboru
} __attribute__((__packed__)) tDirEntry; // Pro soubor, ale i pro adresar

//****** funkce ******
// Pokud je povoleno, tak se snazi v adresarich zaplnit prvne uvolnene polozky,
// teprve pak vytvaret nove 
// #define FILL_FREE_ITEMS
extern unsigned short NEW_FILE_DATE;
extern unsigned short NEW_FILE_TIME;


// pokud je fat == NULL pak pouze vraci pocet partition na disku disc
// jinak se pokusi zinicializovat fat tabulku o indexu part
// pokud volume != NULL, pak zde ulozi jmeno svazku (min.13 B)
// zatim nezpracovava rozsirene oblasti
int FAT_init_Part(unsigned char part,tFAT *fat,char *volume,unsigned char disc);

// nacte a zpracuje FAT partition, zacinajici na sektoru offset a disku disk
// polozka f musi existovat
int FAT_Init(tFAT *f,unsigned long offset,unsigned char disk);

// ulozi udaj o volnem mistu na disk, je treba jen po zapisech
int FAT_Close(tFAT *f);

// nastavi adresarovy ukazatel dir na RootDir
int FAT_Root_Dir(tFAT *f,tFile *dir);

// nastavi polohu v souboru na zadanou, povolene hodnoty jsou <0;FileSize>
int FAT_Seek(tFile *f,unsigned long pos);

// precte length bytu dat ze souboru (adresare) do buff, vraci pocet uspesne nactenych bytu
// buff==NULL -> zapis/cteni z DMA kanalu
int FAT_Read(tFile *f,unsigned short length,unsigned char *buff);

// zapise length bytu do souboru, zvetsi soubor, pokud je priliz maly
// je nutne zapnout pamet cache, jinak nebude zapis funkcni 
// buff==NULL -> zapis/cteni z DMA kanalu
int FAT_Write(tFile *f,unsigned short length,unsigned char *buff);

// Nastavi delku souboru na 0, uvolni misto
int FAT_Clear(tFile *f);

// Vymaze soubor (i adresar, pokud je prazdny)
// vraci FAT_EOF, pokud je mazany adresar neprazdny
// Pouziva hledani souboru, takze po jeho provedeni nelze pokracovat v predchozim hledani!
int FAT_Delete_File(tFile *f);

// vytvori soubor v zadanem adresari, polozka f, pokud je nenulova,
// obsahuje inicializovany soubor nulove delky
// pokud soubor existuje, vraci FAT_ERR! (Je nutne pouzit fci FAT_Clear)
int FAT_Create_File(tFile *dir,char *name,unsigned char attr,tFile *f);

// uzavre soubor, ulozi neulozena data, pokud bylo pouze cteno, neni treba ji volat 
inline int FAT_Close_File(tFile *f);

// porovna 2 retezce, 2. retezec muze obsahovat znaky ? a *, je ovlivnen znakem  
// FAT_CASE_SENSITIVE, vraci FAT_OK pri splneni podminky
int FAT_Cmp(char *name,char *Fname);

// zapocne vyhledavani na 
void FAT_Set_Find(tFile *dir,char *name);

// nastavi vyhledavani podle retezce, umoznuje pokracovani ve vyhledavani
inline void FAT_Set_Find_Continue(tFile *dir,char *name);

// nalezne nasledujici polozku, ulozi jeji jmeno a atributy
// pole Fname musi byt min. 13B velke
// do ext zapise index koncovky, pokud neni NULL
// vraci FAT_OK, nebo FAT_EOF, pokud uz zadna dalsi neni
int FAT_Find_Next(tFile *dir,char *Fname,unsigned char *ext,unsigned char *attr,tFile *file);

#endif   /* _FAT_ */
