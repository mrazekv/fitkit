/*******************************************************************************
   ide.c: IDE routines
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

/****** stavove promenne ******/
signed char IDE_Active_Disc = 2; // aktivni disk
int IDE_Wait_Count; // maximalni pocet cekani
tIDE_HDD IDE_HDD[2];
/****** Oblast Cache disku ******/
/*typedef struct {
	unsigned long Start,Stop,Curr; // sektory
	unsigned char disk;
	unsigned char status; // 0 = volny 1 = nacten 2 = nezapsana data 3 = nepouzit 
} tCache;*/

tCache IDE_Cache[4];
unsigned long IDE_Cache_Sector; // sektor, na ktery je nastavena Cache
unsigned char IDE_Cache_Disc = 2; // disk, na ktery je nastavena Cache
unsigned char IDE_Cache_Last = 0; // posledne pouzita Cache

// nastavi cislo ciloveho sektoru a cilovy disk, pokud jsou v cache nezapsana upravena data, tak je zapise
// a pokud existuje Cache, tak je do ni nacte
int IDE_Set_Sector(unsigned long sector,unsigned char disk) {
	if ((sector != IDE_Cache_Sector) || (disk != IDE_Cache_Disc)) {
		int tr = 4;
		int i = IDE_Cache_Last;// snazi se najit, zda je dane misto uz v cache
		while ((tr>0) && ((IDE_Cache[i].status == 0) || (IDE_Cache[i].status > 2)
			|| (IDE_Cache[i].Curr != sector) || (IDE_Cache[i].disk != disk))) 
				{i=(i+1)&3;tr--;}
		if (tr <= 0) { // snazi se najit volne, nezmenene misto
			tr = 4;i = (IDE_Cache_Last+1)&3;
			while ((tr>0) && ((IDE_Cache[i].status>1) || (IDE_Cache[i].disk!=disk)
				|| (IDE_Cache[i].Start>sector) || (IDE_Cache[i].Stop<sector)))
					{i=(i+1)&3;tr--;}
			if (tr <= 0) { // snazi se najit i zmenene misto
				tr = 4;i = (IDE_Cache_Last+1)&3;
				while ((tr>0) && ((IDE_Cache[i].status!=2) || (IDE_Cache[i].disk!=disk)
					|| (IDE_Cache[i].Start>sector) || (IDE_Cache[i].Stop<sector)))
						{i=(i+1)&3;tr--;}
				if (tr <= 0) return IDE_C_ERR;
				IDE_Cache_Flush(i);
				IDE_Cache[i].status = 0;
			}
		}
		IDE_Cache_Last = i;IDE_Cache_Sector = sector;IDE_Cache_Disc = disk;
		if (IDE_Cache[IDE_Cache_Last].Curr != IDE_Cache_Sector) {
			IDE_Cache[IDE_Cache_Last].Curr = IDE_Cache_Sector;
			IDE_Cache[IDE_Cache_Last].status = 0;
		}
	}
	return IDE_C_OK;
}

// Pripravi data do Cache, pokud jsou treba
int IDE_PrepCache(unsigned short count) {
	if (IDE_Cache_Disc>1) return IDE_C_ERR;
	if (IDE_Cache[IDE_Cache_Last].status == 0) {
		IDE_Change_Disc(IDE_Cache_Disc);
		if (count == 512) IDE_Sector(IDE_Cache_Sector);
		if (count<512) {
//			IDE_Write_Reg(REG_SCR,1);
			int st;
			int wait = IDE_Wait_Count;
			while ((wait>0) && (((st=IDE_Read_Reg(REG_STATUS_CMD)) & STATUS_BSY) || ((st & STATUS_DRDY)==0))) {
				wait--;
				if (st & STATUS_ERR) return IDE_C_ERR;
			}
			if (wait <= 0) return IDE_C_ERR;
			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RST);
			IDE_Spd_Sector(IDE_Cache_Sector,CMD_READ_DMA_RETRIES);
			while ((wait>0) && (((st=IDE_Read_Reg(REG_STATUS_CMD)) & STATUS_DRQ) == 0)) {
				wait--;
				if (st & STATUS_ERR) return IDE_C_ERR;
			}
//			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RST);
			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RUN 
				| IDE_Cache_Last | (IDE_HDD[IDE_Cache_Disc].DMA=1 ? CDMA_SPD : 0));
			wait = IDE_Wait_Count;
			while ((wait>0) && ((FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ,CTRL_DMA_BASE,0) & 0x80) > 0)) wait--;
			if (wait <= 0) {
				FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RST);
				return IDE_C_ERR;
			}
		}
	}
	return IDE_C_OK;
}

// nacte data do data od zadaneho offsetu
// pokud jsou data v Cache, tak cte z ni, jinak se je snazi cist postupne z disku
// vraci pocet nactenych polozek
// data==NULL -> zapis/cteni z DMA kanalu
int IDE_Read(unsigned short offset,unsigned short count,unsigned char *data){
	if (count == 0) return 0;
	if (offset > 511) return IDE_C_ERR;
	if ((count + offset) > 512) count = 512 - offset;
	if (IDE_PrepCache(0) != IDE_C_OK) return IDE_C_ERR;
	if (data != NULL)	IDE_ReadData((IDE_Cache_Last << 9) | offset,count,data);
	else IDE_CH_WriteData((IDE_Cache_Last << 9) | offset,count);
	if (IDE_Cache[IDE_Cache_Last].status == 0) IDE_Cache[IDE_Cache_Last].status = 1;
	return count;
}
// podobne IDE_Read, ale pro zapis
// vraci pocet ulozenych polozek
// data==NULL -> zapis/cteni z DMA kanalu
int IDE_Write(unsigned short offset,unsigned short count,unsigned char *data){
	if (count == 0) return 0;
	if (offset > 511) return IDE_C_ERR;
	if ((count + offset) > 512) count = 512 - offset;
	if (IDE_PrepCache(count) != IDE_C_OK) return IDE_C_ERR;
	if (data != NULL)	IDE_WriteData((IDE_Cache_Last << 9) | offset,count,data);
	else IDE_CH_ReadData((IDE_Cache_Last << 9) | offset,count);
	IDE_Cache[IDE_Cache_Last].status = 2;
	return count;
}

// zapise pozmenena data ve vybrane cachi, cache = 4 -> vsechny, cache > 4 = aktualni 
int IDE_Cache_Flush(unsigned char cache) {
	if (IDE_Cache_Disc>1) return IDE_C_ERR;
	if (cache>4) cache = IDE_Cache_Last;
	int end = cache;
	if (cache == 4) {cache = 0;end = 3;}
	for (;cache<=end;cache++) {
		if (IDE_Cache[cache].status == 2) {
			IDE_Change_Disc(IDE_Cache[cache].disk);
//			IDE_Sector(IDE_Cache[cache].Curr);
//			IDE_Write_Reg(REG_SCR,1);
			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RST);
			IDE_Spd_Sector(IDE_Cache[cache].Curr,CMD_WRITE_DMA_RETRIES);
/*			if (IDE_CMD(CMD_WRITE_DMA_RETRIES) != IDE_RET_OK) {
				return IDE_C_ERR;
			}*/
			FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_DIR | CDMA_RUN 
				| cache | (IDE_HDD[IDE_Cache[cache].disk].DMA=1 ? CDMA_SPD : 0));
			int wait = IDE_Wait_Count;
			while ((wait>0) && ((FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ,CTRL_DMA_BASE,0) & 0x80) > 0)) wait--;
			if (wait <= 0) {
				FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_DMA_BASE, CDMA_RST);
				return IDE_C_ERR;
			}
			IDE_Cache[cache].status = 1;
		}
	}
	return IDE_C_OK;
}

// nastavi Cache0-3 na cacheovani zadane oblasti na disku
// Ssector > Esector - > vypnuta
void IDE_Cache_Set(unsigned char cache,unsigned long Ssector,unsigned long Esector,unsigned char disk) {
	if (cache>3) return;		
	if (disk>1) return;
	if (IDE_Cache[cache].status == 2) IDE_Cache_Flush(cache);
	if ((cache == IDE_Cache_Last) && ((IDE_Cache_Disc != disk) || 
		(IDE_Cache_Sector<Ssector) || (IDE_Cache_Sector>Esector))) {
		int t = IDE_Cache_Disc;
		IDE_Cache_Disc = 2;
		if (IDE_Set_Sector(IDE_Cache_Sector,t) != IDE_C_OK) { 
			IDE_Cache_Disc = 2;
			return;
		}
	}
	IDE_Cache[cache].status = Ssector > Esector ? 3 : 0;
	IDE_Cache[cache].Start = Ssector;
	IDE_Cache[cache].Stop = Esector;
	IDE_Cache[cache].disk = disk;
}
// nastavi Cache na vychozi stav (c0 na cely disk 0, c1 na cely disk 1, c2 + c3 nic)
void IDE_Cache_Init() {
	IDE_Cache[0].Start = 0;
	IDE_Cache[0].Stop = 0xFFFFFFFF;
	IDE_Cache[0].status = 0;
	IDE_Cache[0].disk = 0;
	IDE_Cache[1].Start = 0;
	IDE_Cache[1].Stop = 0xFFFFFFFF;
	IDE_Cache[1].status = 0;
	IDE_Cache[1].disk = 1;
	IDE_Cache[2].status = 3;
	IDE_Cache[3].status = 3;
	IDE_Cache_Last = 0;
	IDE_Cache_Disc = 0;
	IDE_Cache_Sector = 0;
}

/****** funkce ******/

// provede hardwarovy reset
inline void IDE_Hard_Reset() {
	unsigned char reset_counter;
	IDE_Active_Disc = 2;
	for (reset_counter = 0;reset_counter < 5;reset_counter++)
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_BASE, 0);
	// RESET by mel byt nastaven min. 25 ms
	FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE,CTRL_BASE, 1);
}

// vraci slovo z datoveho registru
unsigned short IDE_Read_Word(){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_READ);
  SPI_read_write(PIO_BASE);
  SPI_read_write(REG_DATA * 2);
  unsigned short temp = (SPI_read_write(0) << 8) | SPI_read_write(0);
  SPI_set_cs_FPGA(0);
	return temp;
}
// zapise slovo do datoveho registru
void IDE_Write_Word(unsigned short data){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_WRITE);
  SPI_read_write(PIO_BASE);
  SPI_read_write(REG_DATA * 2);
  SPI_read_write(data >> 8);
	SPI_read_write(data & 0xFF);
  SPI_set_cs_FPGA(0);
}

// Precte data z DMA bufferu, addr je max. 2047 
void IDE_ReadData(unsigned short addr,unsigned short count,unsigned char *buff) {
	if ((buff != NULL) && (addr < 2048)) {
	  int i;
	  addr |= DMA_BASE << 8;
	  SPI_set_cs_FPGA(1);
	  SPI_read_write(OC_FPGA | SPI_FPGA_ENABLE_READ);
	  SPI_read_write(addr >> 8);
	  SPI_read_write(addr & 0xFF);
	  for (i = 0; i < count; i++)
	    buff[i] = SPI_read_write(0);
	  SPI_set_cs_FPGA(0);
	}
}

// Zapise data do DMA bufferu, addr je max. 2047 
void IDE_WriteData(unsigned short addr,unsigned short count,unsigned char *buff) {
	if ((buff != NULL) && (addr < 2048)) {
	  int i;
	  addr |= DMA_BASE << 8;
	  SPI_set_cs_FPGA(1);
	  SPI_read_write(OC_FPGA | SPI_FPGA_ENABLE_WRITE);
	  SPI_read_write(addr >> 8);
	  SPI_read_write(addr & 0xFF);
	  for (i = 0; i < count; i++)
	    SPI_read_write(buff[i]);
	  SPI_set_cs_FPGA(0);
	}
}

// ceka na aktivaci stavu astatus za neaktivnich stavu nstatus, ceka count pokusu
// pokud je nastaveno errdetect, pak se ukonci pri nastaveni bitu ERR 
int IDE_Wait_Status(unsigned char astatus,unsigned char nstatus,unsigned char errdetect,unsigned short count){
	unsigned char status;
	if (errdetect > 0) errdetect = STATUS_ERR | STATUS_DF;
	else errdetect = STATUS_DF;
	do {
		status = IDE_Read_Reg(REG_STATUS_CMD);
		count--; // pockame si, dokud nevyprsi limit, nebo nejsou splneny podminky, nebo dokud nenastane chyba
	}	while ((count > 0) && (((status & astatus) != astatus) || 
		((status & nstatus) != 0)) && ((status & errdetect) == 0));
	if (count == 0) return IDE_RET_COUNT;
	if ((status & STATUS_DF) != 0) return IDE_RET_DF;
	if ((status & errdetect) != 0) return IDE_RET_ERROR;
	return IDE_RET_OK;
}
// precte hodnotu z registru
 unsigned char IDE_Read_Reg(unsigned char addr){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_READ);
  SPI_read_write(PIO_BASE);
  SPI_read_write((addr & 0x1F)*2);
  SPI_read_write(0);
  unsigned char temp = SPI_read_write(0);
  SPI_set_cs_FPGA(0);
	return temp;
}
// zapise hodnotu do registru
void IDE_Write_Reg(unsigned char addr,unsigned char value){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_WRITE);
  SPI_read_write(PIO_BASE);
  SPI_read_write((addr & 0x1F)*2);
  SPI_read_write(0);
  SPI_read_write(value);
  SPI_set_cs_FPGA(0);
}

// nastavi adresu sektoru a spusti prikaz
void IDE_Spd_Sector(unsigned long addr,unsigned char cmd){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_WRITE);
  SPI_read_write(SPD_BASE);
  SPI_read_write(REG_SCR);
  SPI_read_write(1);
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
	unsigned char val = (addr & 0x0f) | LBA_DSC0;
	if (IDE_Active_Disc) val |= 0x10;
  SPI_read_write(val);
  SPI_read_write(cmd);
  SPI_set_cs_FPGA(0);
}

// nastavi adresu sektoru, zatim podporuje pouze prvnich 28 bitu
void IDE_Sector(unsigned long addr){
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA|SPI_FPGA_ENABLE_WRITE);
  SPI_read_write(SPD_BASE);
  SPI_read_write(REG_SCR);
  SPI_read_write(1);
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
  SPI_read_write(addr & 0xff);
 	addr = addr >> 8;
	unsigned char val = (addr & 0x0f) | LBA_DSC0;
	if (IDE_Active_Disc) val |= 0x10;
  SPI_read_write(val);
  SPI_set_cs_FPGA(0);
/*	IDE_Write_Reg(REG_LBA_0_7,addr & 0xff);
	addr = addr >> 8;
	IDE_Write_Reg(REG_LBA_8_15,addr & 0xff);
	addr = addr >> 8;
	IDE_Write_Reg(REG_LBA_16_23,addr & 0xff);
	addr = addr >> 8;
	unsigned char val = (addr & 0x0f) | LBA_DSC0;
	if (IDE_Active_Disc) val |= 0x10;
	IDE_Write_Reg(REG_LBA_24_27,val);*/
}
// Zmeni aktivni disk, 0= Master, ostatni = Slave
void IDE_Change_Disc(int disc){
	if (IDE_Active_Disc != disc) {
		IDE_Active_Disc = disc;
		if (disc) IDE_Write_Reg(REG_LBA_24_27,LBA_DSC1);
		else IDE_Write_Reg(REG_LBA_24_27,LBA_DSC0);
	}
}

// provede prikaz identify v PIO modu - vyzaduje 512B buffer 
// chybovy stav
int IDE_Identify(unsigned char *buff) {
  if (IDE_CMD(CMD_IDENTIFY) != IDE_RET_OK) return IDE_RET_ERROR;
  int cnt;
	unsigned short cache;
	for (cnt = 0;cnt<256;cnt++) {
		cache = IDE_Read_Word();
		*buff = cache & 0xFF;
		buff++;		
		*buff = cache >> 8;
		buff++;		
	}
	return IDE_RET_OK;
}

// Prevede retezec z Identify na ANSI normu, pole str musi byt min. len+1 bytu dlouhe
// len = delka pole
void IDE_Convert_String(unsigned char *str,signed char len){
	len--;
	while ((len >= 0) && (str[len] == 0x20)) len--;
	str[len + 1] = 0;
	unsigned char index = 0,swp;
	for (;index < len;index++) {
		swp = str[index];str[index] = str[index + 1];
		index++;str[index] = swp;
	}
}
// provede prikaz a vyckava na jeho dokonceni (zruseni stavu BSY), pripadne na 
// chybovy stav
int IDE_CMD(unsigned char cmd){
	int status;
	if ((status = IDE_Wait_Status(STATUS_DRDY,STATUS_BSY,0,IDE_Wait_Count)) != IDE_RET_OK) return status; 	
	IDE_Write_Reg(REG_STATUS_CMD,cmd);
	if ((status = IDE_Wait_Status(0,STATUS_BSY,1,IDE_Wait_Count)) != IDE_RET_OK) return status;
	return IDE_RET_OK; 	
}

// nastavi limit pro pocet cekani na stav
inline void Set_Wait_Count(int count){
	IDE_Wait_Count = count;	
}

// Pokusi se nalezt a identifykovat disky
// prednastaveni a restart disku
void IDE_HDD_Setup_Init() {
	IDE_Hard_Reset();
	IDE_Active_Disc = 0;
	IDE_Cache_Init();
	IDE_HDD[0].status = IDE_HDD_UNKNOWN;
	IDE_HDD[1].status = IDE_HDD_UNKNOWN;
	IDE_Wait_Count = 16000;
}
// cekani na dokonceni diagnostiky disku, zjisteni nastaveni
// pokud jeste neni dokoncena diagnostika, vraci IDE_SETUP_WAIT
int IDE_HDD_Setup() {
	unsigned int i;
	unsigned char Cache[512];
/*	for (i=0;i<20000;i++) {
		FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ,CTRL_DMA_BASE,0);
		WDG_reset();
	} */
	if (IDE_Wait_Status(STATUS_DRDY,STATUS_BSY,1,IDE_Wait_Count) == IDE_RET_COUNT) return IDE_SETUP_WAIT;
//	IDE_Change_Disc(0);
/*	unsigned char status = IDE_Read_Reg(REG_ERROR_FEA);
	if ((status & 0x7F) == 1) IDE_HDD[0].status = IDE_HDD_ATA;
	else IDE_HDD[0].status = IDE_HDD_BAD;*/
/*	if ((status & 0x80) > 0) IDE_HDD[1].status = IDE_HDD_BAD;
	else {
 		IDE_HDD[1].status = IDE_HDD_NONE;
		IDE_Change_Disc(1);
		if (IDE_Read_Reg(REG_ERROR_FEA) == 1) IDE_HDD[1].status = IDE_HDD_ATA;
	}*/
	IDE_HDD[0].status = IDE_HDD_ATA;
	IDE_HDD[1].status = IDE_HDD_NONE;
	tIdeIdentify *ident = (tIdeIdentify*)Cache;
	for (i = 0;i < 2;i++) if (IDE_HDD[i].status == IDE_HDD_ATA) {
		IDE_HDD[i].status = IDE_HDD_NONE;
		IDE_HDD[i].sectors = 0;
//		IDE_Change_Disc(i);
  	if (IDE_Identify(Cache) != IDE_RET_OK) continue;
		IDE_HDD[i].status = IDE_HDD_ATA;
		if ((ident->general & 0x8000) > 0) IDE_HDD[i].status = IDE_HDD_ATAPI;
		IDE_HDD[i].DMA = ((ident->MultiwordDMAModes) & 2) > 0 ? 1 : 0;
		IDE_Write_Reg(REG_ERROR_FEA,0x03);
		IDE_Write_Reg(REG_SCR,0x20 | IDE_HDD[i].DMA);
  	if (IDE_CMD(CMD_SET_FEATURES) != IDE_RET_OK) continue;
		IDE_HDD[i].sectors = ident->LBASectors;
		IDE_HDD[i].FeatureSet = ident->FeatureSet;
/*		if ((ident->FeatureSet & 0x0001) > 0) {
			IDE_Write_Reg(REG_ERROR_FEA,0xD8);
			IDE_Write_Reg(REG_LBA_16_23,0x4F);
			IDE_Write_Reg(REG_LBA_24_27,0xC2);
	  	if (IDE_CMD(CMD_SMART) != IDE_RET_OK) continue;
  		IDE_HDD[i].status |= IDE_HDD_SMART;
  	}*/
	}
	if (((IDE_HDD[1].status & 128) > 0) && ((IDE_HDD[0].status & 128) == 0))
		IDE_Change_Disc(1);
	else IDE_Change_Disc(0);
	return IDE_SETUP_DONE;
}

unsigned long FPGA_SPI_RW_A8_D3(unsigned char mode,unsigned char addr,unsigned long data) {
  SPI_set_cs_FPGA(1);
  SPI_read_write(OC_FPGA | mode);
  SPI_read_write(addr);
  unsigned char d0=data & 0xFF;
  unsigned char d1=(data >> 8) & 0xFF;
  unsigned char d2=(data >> 16) & 0xFF;
  data = ((long)SPI_read_write(d2) << 16) | ((long)SPI_read_write(d1) << 8) | SPI_read_write(d0);
  SPI_set_cs_FPGA(0);
  return data;
}

// Zapise data do DMA kanalu, addr je max. 2047 
void IDE_CH_WriteData(unsigned short addr,unsigned short count) {
	unsigned long data = ((count + addr-1) & 0x07FF);
	data=0x800000 | (data << 11) | addr;
	FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_WRITE,0x10,data);
}
// Precte data z DMA kanalu, addr je max. 2047 
void IDE_CH_ReadData(unsigned short addr,unsigned short count) {
	unsigned long data = ((count + addr-1) & 0x07FF);
	data=0xC00000 | (data << 11) | addr;
	FPGA_SPI_RW_A8_D3(SPI_FPGA_ENABLE_WRITE,0x10,data);
}
