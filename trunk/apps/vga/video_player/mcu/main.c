/*******************************************************************************
   main.c: main for Video player
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Stanislav Sigmund <xsigmu02 AT stud.fit.vutbr.cz>

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
#define SPIMAX

#include <fitkitlib.h>
#include <string.h>
#include <stdlib.h>

#include <ide\ide.h>
#include <ide\fat.h>

void print_user_help(void){
    term_send_str_crlf(" INIT ............ spusti diagnostiku disku a nacte FAT z prvniho oddilu");
    term_send_str_crlf(" DIR [name] ...... vypise podrobny seznam souboru ");
    term_send_str_crlf("                   v aktualnim adresari (prip. podle masky)");
    term_send_str_crlf(" DIRP [name] ..... jako DIR, ale jen strucny seznam");
    term_send_str_crlf(" CD adr .......... skoci do zadaneho adresare");
    term_send_str_crlf(" CD \\ ............ skoci do korenoveho adresare");
    term_send_str_crlf(" CAT name ........ vypise obsah souboru na obrazovku");
    term_send_str_crlf(" MON ............. zapne monitor");
    term_send_str_crlf(" CLEAR ........... vycisti obrazovku");
    term_send_str_crlf(" PLAY name ....... spusti prehravani videa");
    term_send_str_crlf("   Vetsina jmen lze zapsat hvezdickovou konvenci, vybere se pak prvni vhodna");
    term_send_str_crlf("   polozka.");
}

unsigned char IDE_str[41];
tFAT F;
tFile ADir;
tFile file;
char name[13];

void report(int status) {
	switch (status) {
    case IDE_RET_OK:term_send_str_crlf("OK");break;
    case IDE_RET_ERROR:term_send_str_crlf("Nastala chyba!");break;
    case IDE_RET_COUNT:term_send_str_crlf("Pomala reakce!");break;
    case IDE_RET_ABRT:term_send_str_crlf("Nepodporovany prikaz!");break;
    case IDE_RET_DF:term_send_str_crlf("Chyba zarizeni, muze byt nadale nestabilni!");break;
    default:term_send_str_crlf("Neco jineho!");
	}
}

int init_Part(unsigned char part,unsigned char disc) {
	term_send_str_crlf("Nacitam oddil...");
	if (FAT_init_Part(part,&F,name,disc) == FAT_ERR) return FAT_ERR;
	term_send_str("Nactena ");
	switch (F.FATType) {
		case FAT_32: term_send_str_crlf("FAT32");break;
		case FAT_16: term_send_str_crlf("FAT16");break;
		case FAT_12: term_send_str_crlf("FAT12");break;
	}
	term_send_str_crlf("Celkova velikost [kByte]: ");
	term_send_num(F.Clusters * F.SecPerClus / 2);
	term_send_crlf();
	if (F.FreeClus != 0xFFFFFFFF) {
		term_send_str(" Volne misto (posledni udaj): ");
		term_send_num(F.FreeClus * F.SecPerClus / 2);
		term_send_crlf();
	}
	if (name[0] != 0) {
		term_send_str_crlf("Jmeno oddilu: ");
		term_send_str_crlf(name);
	}
	return FAT_OK;
}

void init_FAT() {
	F.FATType = 0;
	int c0 = 0,c1 = 0;
	if ((IDE_HDD[0].status & IDE_HDD_ATA) == IDE_HDD_ATA) {
		term_send_str_crlf("Disk: 0");
		if ((c0 = FAT_init_Part(0,NULL,NULL,0)) == FAT_ERR) {
			term_send_str_crlf("Chyba pri nacitani MBR");
			return;
		}
		term_send_str("Pocet aktivnich partition s FAT32: ");
		term_send_num(c0);
		term_send_crlf();
	}
	if ((IDE_HDD[1].status & IDE_HDD_ATA) == IDE_HDD_ATA) {
		term_send_str("Disk: 1");
		if ((c1 = FAT_init_Part(0,NULL,NULL,1)) == FAT_ERR) {
			term_send_str("Chyba pri nacitani MBR");
			return;
		}
		term_send_str("Pocet aktivnich oddilu s FAT32: ");
		term_send_num(c1);
		term_send_crlf();
	}
	if (c0 > 0) {
		if (init_Part(0,0) != FAT_OK) {
			term_send_str_crlf("Chyba pri inicializaci oddilu na disku 0!");
			return;
		}
	} else if (c1 > 0) {
		if (init_Part(0,1) != FAT_OK) {
			term_send_str_crlf("Chyba pri inicializaci oddilu na disku 1!");
			return;
		}
	} else {
		term_send_str_crlf("Nenalezen zadny oddil FAT32 na discich!");
		return;
	}
	if (FAT_Root_Dir(&F,&ADir) != FAT_OK) return; 
}

void init_disk() {
	unsigned int id;
	term_send_str("Inicializace IDE");
	IDE_HDD_Setup_Init();
	while (IDE_HDD_Setup() == IDE_SETUP_WAIT) {
		WDG_reset();
		term_send_str(".");
	}
	term_send_crlf();
	for (id = 0;id < 2;id++) {
	 	term_send_str("Disk ");
	 	term_send_num(id);
	 	term_send_char(':');
	 	term_send_crlf();
		switch (IDE_HDD[id].status) {
			case IDE_HDD_UNKNOWN:term_send_str("  Neidentifikovan!");break;
			case IDE_HDD_NONE:term_send_str("  Neni");break;
			case IDE_HDD_BAD:term_send_str("  Vadny!");break;
			case IDE_HDD_ATA:term_send_str("  ATA");break;
			case IDE_HDD_ATAPI:term_send_str("  ATAPI");break;
			case IDE_HDD_ATA+IDE_HDD_SMART:term_send_str("  ATA + SMART");break;
		}
		term_send_crlf();
		if (IDE_HDD[id].status >= IDE_HDD_ATA) {
			term_send_str("  Velikost: ");
			term_send_num(IDE_HDD[id].sectors >> 11);
			term_send_str_crlf(" MB");
		}
	}
}

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

void showFile(char *fname) {
	int status;
	unsigned char str[4];
	unsigned short w,h;
	FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x41, 80); // 640
	FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x42, 60); // 480
	FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x43,0);
	FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40, 128); // run + banka 0
	FAT_Set_Find(&ADir,fname);
	term_send_str_crlf("Start");
	while (((status = FAT_Find_Next(&ADir,name,NULL,NULL,&file)) == FAT_OK) && 
		((file.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) > 0)) WDG_reset();
	if (status == FAT_ERR) term_send_str_crlf("Chyba pri hledani souboru!");
	if (status == FAT_EOF) term_send_str_crlf("Soubor nenalezen!");
	if (status == FAT_OK) {
		if (FAT_Read(&file,4,str) < 4) return;
		if ((str[0] != 'R') || (str[1] != 'I') || (str[2] != 'F') || (str[3] != ' ')) return;
		if (FAT_Read(&file,2,(char*)&w) < 2) return;
		if (FAT_Read(&file,2,(char*)&h) < 2) return;
		term_send_num(w);term_send_crlf();
		term_send_num(h);term_send_crlf();
		long y;
		if ((w>640) || (h>480)) {term_send_str_crlf("BIG");return;}
  	for (y=0;y<h;y++) {
			FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_WRITE,0x80,y << 10);
			if (FAT_Read(&file,w,NULL) <= 0) {term_send_str_crlf("C4!");return;}
		}
	}
}

void Play(char *fname) {
	int status,f;
	tRAFFHead RH;
	FAT_Set_Find(&ADir,fname);
	term_send_str_crlf("Start");
	while (((status = FAT_Find_Next(&ADir,name,NULL,NULL,&file)) == FAT_OK) && 
		((file.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) > 0)) WDG_reset();
	if (status == FAT_ERR) term_send_str_crlf("Chyba pri hledani souboru!");
	if (status == FAT_EOF) term_send_str_crlf("Soubor nenalezen!");
	if (status == FAT_OK) {
		if (FAT_Read(&file,28,(unsigned char*)&RH) < 28) return;
		if ((RH.type[0] != 'R') || (RH.type[1] != 'A') || (RH.type[2] != 'F') || (RH.type[3] != ' ')) 
			{term_send_str_crlf("Neni soubor videa!");return;}
		term_send_str("Rozmery: ");term_send_num(RH.width);term_send_char('x');term_send_num(RH.height);term_send_crlf();
		term_send_str("Snimku: ");term_send_num(RH.frames);term_send_crlf();
		long y,t;
		if ((RH.width>640) || (RH.height>480)) {term_send_str_crlf("video je priliz velke");return;}
		int bank=1;
		int size=0;
		if ((RH.width<=320) && (RH.height<=240)) size=1;
		if ((RH.width<=160) && (RH.height<=120)) size=2;
		if ((RH.width<=80) && (RH.height<=60)) size=3;
	  FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x41,(RH.width+7) >> 3);
	  FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x42,(RH.height+7) >> 3);
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x43,size);
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40,128);
  	for (f=0;f<RH.frames;f++) {
  		if (FPGA_SPI_RW_A8_D16(SPI_FPGA_ENABLE_READ,0x20,0) & 0x8000) {
				term_send_str_crlf("preruseno uzivatelem");break;
			}
	  	for (y=0;y<RH.height;y++) {
				FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_WRITE,0x80,(y << 10) | ((long)bank << 19));
				if (FAT_Read(&file,RH.width,NULL) <= 0) term_send_str_crlf("Chyba cteni!");
				t=FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_READ,0x80,0)-(y << 10) - ((long)bank << 19);
				if (t!=RH.width) {
					term_send_str("radek");term_send_num(y);
					term_send_char(':');term_send_num(t);term_send_crlf();
				}
			}
			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40,128+bank);
			bank=(bank+1) & 1;
		}
	}
}

void clearBank(int bank) {
	if (bank>15) bank=15;
	long b=(long)bank << 19;

  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA | SPI_FPGA_ENABLE_WRITE);
  SPI_read_write(DMA_BASE | 6);
  SPI_read_write(0);
  int i;
  for (i = 0; i < 512; i++)
    SPI_read_write(0);
  SPI_set_cs_FPGA(0);

  unsigned long addr;
	long y;
	for (y=0;y<480;y++) {
		addr=(y << 10) | b;
		FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_WRITE,0x80,addr);
		IDE_CH_WriteData(1536,320);
		IDE_CH_WriteData(1536,320);
	}
}

/*******************************************************************************
 * DEKODOVANI A VYKONANI UZIVATELSKYCH PRIKAZU
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  unsigned short data;
  if (strcmp3(cmd_ucase, "DIR")) {
  	char *Fname = NULL;
  	int packed = 0;
  	if (cmd_ucase[3] == 'P') {
  		packed = 1;
	  	if ((strlen(cmd_ucase) >= 6) && (cmd_ucase[4] == ' ')) Fname = &cmd_ucase[5];		
		} else  
		 	if ((strlen(cmd_ucase) >= 5) && (cmd_ucase[3] == ' ')) Fname = &cmd_ucase[4];		
		FAT_Set_Find(&ADir,Fname); // NULL = '*'
		int status;
		while ((status = FAT_Find_Next(&ADir,name,NULL,NULL,&file)) == FAT_OK) {
			if ((file.attr & ATTR_VOLUME_ID) == 0) { // jmeno svazku nevypisujeme
				term_send_str(name);term_send_char(' ');
				if (!packed) {
					int i;
					for (i = 12 - strlen(name);i > 0;i--) term_send_char(' ');
					if ((file.attr & ATTR_READ_ONLY) > 0) term_send_char('r');
					else term_send_char(' ');
					if ((file.attr & ATTR_HIDDEN) > 0) term_send_char('h');
					else term_send_char(' ');
					if ((file.attr & ATTR_ARCHIVE) > 0) term_send_char('a');
					else term_send_char(' ');
					if ((file.attr & ATTR_SYSTEM) > 0) term_send_char('s');
					else term_send_char(' ');
					if ((file.attr & ATTR_DIRECTORY) > 0) term_send_str("DIR");
					else term_send_num(file.FileSize);
					term_send_crlf();
				}
			}
      WDG_reset();
		}
		if (status == FAT_ERR) term_send_str_crlf("Chyba pri cteni adresare! ");
    return (USER_COMMAND);
  }  
  else if (strcmp3(cmd_ucase, "CD ")) {
		if ((cmd_ucase[3] == '/') || (cmd_ucase[3] == '\\')) {
			if (FAT_Root_Dir(ADir.fat,&ADir) != FAT_OK) 
				term_send_str_crlf("Nelze inicializovat RootDir!");
	    return (USER_COMMAND);
		}
		FAT_Set_Find(&ADir,&cmd_ucase[3]);
		int status;
		while (((status = FAT_Find_Next(&ADir,name,NULL,NULL,&file)) == FAT_OK) && 
			(((file.attr & ATTR_DIRECTORY) == 0) || 
			((file.attr & ATTR_VOLUME_ID) > 0))) WDG_reset();
		if (status == FAT_ERR) term_send_str_crlf("Chyba pri hledani adresare!");
		if (status == FAT_EOF) {term_send_str_crlf("Adresar nenalezen!");}
		else {
			memcpy(&ADir,&file,sizeof(tFile));
			if (ADir.StartClus == 0) FAT_Root_Dir(ADir.fat,&ADir);
		}
    return (USER_COMMAND);
  }  
  else if (strcmp4(cmd_ucase, "CAT ")) {
		FAT_Set_Find(&ADir,&cmd_ucase[4]);
		int status;
		while (((status = FAT_Find_Next(&ADir,name,NULL,NULL,&file)) == FAT_OK) && 
			((file.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) > 0)) WDG_reset();
		if (status == FAT_ERR) term_send_str_crlf("Chyba pri hledani souboru!");
		if (status == FAT_EOF) term_send_str_crlf("Soubor nenalezen!");
		if (status == FAT_OK) {
			while ((status = FAT_Read(&file,40,IDE_str)) > 0) {
				IDE_str[status] = 0;
				term_send_str(IDE_str);
			}
		}
    return (USER_COMMAND);
  }  
  else if (strcmp(cmd_ucase, "INIT") == 0) {
		IDE_Cache_Init();
  	init_disk();
  	init_FAT();
		return (USER_COMMAND);
	}
  else if (strcmp(cmd_ucase, "MON") == 0) {
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x41, 80); // 640
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x42, 60); // 480
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x43,0);
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40, 128); // run + banka 0
    return (USER_COMMAND);
	}
  else if (strcmp4(cmd_ucase, "MON ")) {
     long cdata = str2long((unsigned char *)&cmd_ucase[4]);
     data = (unsigned char) cdata;
	   FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40,data); // 640
     return (USER_COMMAND);
  }
  else if (strcmp4(cmd_ucase, "MONX ")) {
     long cdata = str2long((unsigned char *)&cmd_ucase[4]);
     data = (unsigned char) cdata;
	   FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x41,data); // 640
     return (USER_COMMAND);
  }
  else if (strcmp4(cmd_ucase, "MONY ")) {
     long cdata = str2long((unsigned char *)&cmd_ucase[4]);
     data = (unsigned char) cdata;
	  FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x42,data); // 640
     return (USER_COMMAND);
  }
  else if (strcmp4(cmd_ucase, "CLR ")) {
    //long cdata = str2long((unsigned char *)&cmd_ucase[4]) << 19;
    clearBank(str2long((unsigned char *)&cmd_ucase[4]));
    return (USER_COMMAND);
  }
  else if (strcmp(cmd_ucase, "CLEAR")==0) {
  int b;
 	for (b=0;b<16;b++) clearBank(b);
    return (USER_COMMAND);
  }
  else if (strcmp5(cmd_ucase, "SHOW ")) {
	  showFile(&cmd_ucase[5]);
	  return (USER_COMMAND);
  }
  else if (strcmp5(cmd_ucase, "PLAY ")) {
	  Play(&cmd_ucase[5]);
	  return (USER_COMMAND);
  }
  else if (strcmp5(cmd_ucase, "SIZE ")) {
     long cdata = str2long((unsigned char *)&cmd_ucase[5]);
     data = (unsigned char) cdata;
		 FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x43,data);
     return (USER_COMMAND);
  }

	return (CMD_UNKNOWN);
}



/**
  \brief  Inicializace HW komponent po naprogramovani FPGA, tato funkce musi byt defimovana
  v uzivatelskem programu.
 */
void fpga_initialized(){
// 	init_disk();
}

/*******************************************************************************
 * HLAVNI FUNKCE MAIN
*******************************************************************************/
int main(void)
{
  initialize_hardware();
  WDG_init();
  // INICIALIZACE LED, LED0 VYSTUP
  unsigned int id = 0;
  unsigned short keys=0;
  //HLAVNI SMYCKA
  for (;;)
  {
    if ((id & 64) == 0)
    {
      WDG_reset(); // SOFTWARE WATCHDOG
      // IDLE TERMINALU
      terminal_idle(); // OBSLUHA TERMINALU
      
      if (keys>0)
				keys=FPGA_SPI_RW_A8_D16(SPI_FPGA_ENABLE_READ,0x20,0);
			else {
				keys=FPGA_SPI_RW_A8_D16(SPI_FPGA_ENABLE_READ,0x20,0);
				if (keys & (1 << 12)) {
					IDE_Cache_Init();
			  	init_disk();
			  	init_FAT();
				}
				else if (keys & (1 << 13)) {
					FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x41, 80); // 640
					FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x42, 60); // 480
					FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,0x40, 128); // run + banka 0
				}
				else if (keys & (1 << 14)) showFile("IMG2.RIF");
				else if (keys & 1) Play("T.RAF");
				else if (keys & (1 << 4)) Play("T2.RAF");
			}
    }

    id += 1;
    if (id == 500000) id = 0;
  }
} 
