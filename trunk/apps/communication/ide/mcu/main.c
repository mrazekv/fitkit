/*******************************************************************************
   main.c: main for Disc browser
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
    term_send_str_crlf(" IRESET .......... zresetuje zarizeni");
    term_send_str_crlf(" INIT ............ spusti diagnostiku disku a nacte FAT z prvniho oddilu");
    term_send_str_crlf(" CLOSE ........... uzavre nactenou FAT, je treba jen kdyz se na ni zapisovalo");
    term_send_str_crlf(" DIR [name] ...... vypise podrobny seznam souboru ");
    term_send_str_crlf("                   v aktualnim adresari (prip. podle masky)");
    term_send_str_crlf(" DIRP [name] ..... jako DIR, ale jen strucny seznam");
    term_send_str_crlf(" CD adr .......... skoci do zadaneho adresare");
    term_send_str_crlf(" CD \\ ............ skoci do korenoveho adresare");
    term_send_str_crlf(" CAT name ........ vypise obsah souboru na obrazovku");
    term_send_str_crlf(" MKDIR name ...... vytvori prazdny adresar"); 
    term_send_str_crlf(" WRITE name ...... zacne zapisovat textovy soubor, znak * ukonci zapis"); 
    term_send_str_crlf(" APPEND name ..... podobne WRITE, ale zacne psat za koncem existujiciho souboru"); 
    term_send_str_crlf(" DEL name ........ vymaze vsechny soubory/prazdne adresare zadaneho jmena"); 
    term_send_str_crlf("   Vetsina jmen lze zapsat hvezdickovou konvenci, vybere se pak prvni vhodna");
    term_send_str_crlf("   polozka. U prikazu MKDIR a WRITE musi byt zapsano cele.");
}

unsigned char IDE_str[41];
tFAT F;
tFile ADir;
tFile file;
char name[13];

void Init() {
term_send_str("Inicializace IDE");
IDE_HDD_Setup_Init();
while (IDE_HDD_Setup() == IDE_SETUP_WAIT) {
  WDG_reset();
  term_send_str(".");
}
term_send_crlf();
term_send_str("Disk: ");
switch (IDE_HDD[0].status) {
  case IDE_HDD_UNKNOWN:term_send_str("  Neidentifikovan!");break;
  case IDE_HDD_NONE:term_send_str("  Neni");break;
  case IDE_HDD_BAD:term_send_str("  Vadny!");break;
  case IDE_HDD_ATA:term_send_str("  ATA");break;
  case IDE_HDD_ATAPI:term_send_str("  ATAPI");break;
  case IDE_HDD_ATA+IDE_HDD_SMART:term_send_str("  ATA + SMART");break;
}
term_send_crlf();
if (IDE_HDD[0].status >= IDE_HDD_ATA) {
  term_send_str("  Velikost: ");
  term_send_num(IDE_HDD[0].sectors >> 11);
  term_send_str_crlf(" MB");
}
F.FATType = 0;
int cnt = 0;
if ((IDE_HDD[0].status & IDE_HDD_ATA) == IDE_HDD_ATA) {
  term_send_str_crlf("Disk: 0");
  if ((cnt = FAT_init_Part(0,NULL,NULL,0)) == FAT_ERR) {
    term_send_str_crlf("Chyba pri nacitani MBR");
    return;
  }
  term_send_str("Pocet aktivnich partition s FAT32: ");
  term_send_num(cnt);
  term_send_crlf();
}
if (cnt > 0) {
  if (FAT_init_Part(0,&F,name,0) == FAT_ERR) return;
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
} else {
  term_send_str_crlf("Nenalezen zadny oddil FAT32 na discich!");
  return;
}
if (FAT_Root_Dir(&F,&ADir) != FAT_OK) return;
}

void CreateDir(char *name) {
	if (FAT_Create_File(&ADir,name,ATTR_DIRECTORY,NULL) == FAT_OK) 
		term_send_str("Adresar vytvoren");
	else term_send_str("Chyba pri vytvareni adresare!");
	term_send_crlf();
}  

void WriteText(char *name) {
	unsigned char c;
	tFile f;
	if (FAT_Create_File(&ADir,name,ATTR_ARCHIVE,&f) == FAT_OK) { 
		term_send_str_crlf("Soubor vytvoren, piste (* = konec souboru):");
		while ((c = term_rcv_char()) != '*') {
			if (c >= 32) {
				term_send_char(c);
				if (FAT_Write(&f,1,&c) != 1) break;
			}
			if (c == 13) {
				term_send_crlf();
				if (FAT_Write(&f,1,&c) != 1) break;
				c = 10;
				if (FAT_Write(&f,1,&c) != 1) break;
			}
		}
		FAT_Close_File(&f);
		term_send_str_crlf("Konec souboru");
	}
	else term_send_str_crlf("Chyba pri vytvareni souboru (soubor jiz existuje?)!");
}

void AppendText(char *name) {
	unsigned char c;
	tFile f;
	FAT_Set_Find(&ADir,name);
	int status;
	while (((status = FAT_Find_Next(&ADir,NULL,NULL,NULL,&f)) == FAT_OK) && 
		((f.attr & (ATTR_DIRECTORY | ATTR_VOLUME_ID)) > 0)) WDG_reset();
	if (status == FAT_ERR) term_send_str_crlf("Chyba pri hledani souboru!");
	if (status == FAT_EOF) {
		if (FAT_Create_File(&ADir,name,ATTR_ARCHIVE,&f) == FAT_OK) { 
			term_send_str("Soubor vytvoren, ");
		} else {
			term_send_str_crlf("Soubor nelze vytvorit!");
			return;
		}
	} else term_send_str("Soubor nacten, ");
	if (status == FAT_OK) {
		term_send_str("piste (* = konec souboru):");
/*		while ((status = FAT_Read(&f,40,IDE_str)) > 0) {
			IDE_str[status] = 0;
			RS232_SendString(IDE_str);
		}*/
		FAT_Seek(&f,f.FileSize);
		while ((c = term_rcv_char()) != '*') {
			if (c >= 32) {
				term_send_char(c);
				if (FAT_Write(&f,1,&c) != 1) break;
			}
			if (c == 13) {
				term_send_crlf();
				if (FAT_Write(&f,1,&c) != 1) break;
				c = 10;
				if (FAT_Write(&f,1,&c) != 1) break;
			}
		}
		FAT_Close_File(&f);
		term_send_str_crlf("Konec souboru");
	}
}

void Delete(char *name) {
	char n[13];
	int status;
	tFile f;
	FAT_Set_Find(&ADir,name);
	while (1) {
		FAT_Set_Find_Continue(&ADir,name);
		while (((status = FAT_Find_Next(&ADir,n,NULL,NULL,&f)) == FAT_OK) && 
			(((f.attr & ATTR_VOLUME_ID) > 0) || (n[0] == '.'))) WDG_reset();
		if (status == FAT_ERR) {
			term_send_str_crlf("Chyba pri hledani souboru!");
			return;
		}
		if (status == FAT_EOF) break;
		if (FAT_Delete_File(&f) == FAT_EOF) {
			term_send_str("Adresar ");
			term_send_str(n);
			term_send_str_crlf(" neni prazdny!");
		}
	}
}

/*******************************************************************************
 * DEKODOVANI A VYKONANI UZIVATELSKYCH PRIKAZU
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
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
  else if (strcmp6(cmd_ucase, "WRITE ")) {
  	WriteText(&cmd_ucase[6]);
    return (USER_COMMAND);
  }  
  else if (strcmp7(cmd_ucase, "APPEND ")) {
  	AppendText(&cmd_ucase[7]);
    return (USER_COMMAND);
  }  
  else if (strcmp6(cmd_ucase, "MKDIR ")) {
  	CreateDir(&cmd_ucase[6]);
    return (USER_COMMAND);
  }  
  else if (strcmp4(cmd_ucase, "DEL ")) {
  	Delete(&cmd_ucase[4]);
    return (USER_COMMAND);
  }  
  else if (strcmp(cmd_ucase, "INIT") == 0) {
		Init();
		return (USER_COMMAND);
	}
  else if (strcmp(cmd_ucase, "CLOSE") == 0) {
		if (FAT_Close(&F) != FAT_OK) term_send_str_crlf("Chyba pri zavirani FAT!");
		return (USER_COMMAND);
	}
  else if (strcmp(cmd_ucase, "IRESET") == 0) {
  	 IDE_Hard_Reset();
     term_send_str_crlf("Resetovano");
     return (USER_COMMAND);
  }
	return (CMD_UNKNOWN);
}



/**
  \brief  Inicializace HW komponent po naprogramovani FPGA, tato funkce musi byt defimovana
  v uzivatelskem programu.
 */
void fpga_initialized(){
// 	Init();
}

/*******************************************************************************
 * HLAVNI FUNKCE MAIN
*******************************************************************************/
int main(void)
{
  initialize_hardware();
  WDG_init();
  //HLAVNI SMYCKA
  for (;;)
  {
    WDG_reset(); // SOFTWARE WATCHDOG
    // IDLE TERMINALU
    terminal_idle(); // OBSLUHA TERMINALU
	} 
}
