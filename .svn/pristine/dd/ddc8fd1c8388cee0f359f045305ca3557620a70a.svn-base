/*******************************************************************************
   ide.h: FAT routines
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
#ifndef _IDE_
#define _IDE_
// radic blokuje adresy:
// 3 - PIO
// 4 - 7 CTRL
// 8 - F DMA

/****** Adresy SPI_dekoderu ******/
// adresa SPI_dekoderu pro rychly zapis registru
#define SPD_BASE 0x00
// adresa SPI_dekoderu pro rezim PIO
#define PIO_BASE 0x03
// adresa SPI_dekoderu pro rezim DMA
#define DMA_BASE 0x08
// adresa SPI_dekoderu pro rizeni nekterych signalu (zatim defakto jen RESET-)
#define CTRL_BASE 0x04
// adresa SPI_dekoderu pro rizeni DMA
#define CTRL_DMA_BASE 0x05
	// W: spusteni kanalu (i pozastaveni) R: kanal bezi
	#define CDMA_RUN 0x80
	// W: reset R: signal DMARQ
	#define CDMA_RST 0x40
	// W: 0 = DMA0, 1 = DMA1
	#define CDMA_SPD 0x20
	// W: 0 = READ, 1 = WRITE
	#define CDMA_DIR 0x10
	// 00 - 03 = blok
	// R: b5 - b2 = horni 4bity adresy slova (sledovani prubehu)

/****** Adresy registru IDE-rozhrani a jejich priznaku ******/
// datovy registr, jediny je 16-bitovy
#define REG_DATA 0x10
// vraci chybovy stav, pri zapisu nastaveni FEATURES
#define REG_ERROR_FEA 0x11
	// Bad Block
	#define IERR_BBK 0x80
	// Uncorrectable data error
	#define IERR_UNC 0x40
	// Media changed - medium vymeneno - smysl snad jen u CD-ROM 
	#define IERR_MC 0x20
	// ID not found - ID sektoru nenalezeno
	#define IERR_IDNF 0x10
	// Media change requested - zadost o vymenu media (stisk tlacitka uzivatelem)
	#define IERR_MCR 0x08
	// Aborted command - prikaz zrusen (napr. kdyz je disk zaneprazdnen) 
	#define IERR_ABRT 0x04
	// Track 0 not found - stopa 0 nenalezena (spatna kalibrace)
	#define IERR_TK0NF 0x02
	// Address mark not found - spatna adresa po nalezeni ID sektoru
	#define IERR_AMNF 0x01

// pocitadlo poctu sektoru pro zapis/cteni vetsiho poctu naraz 
// (vraci zbyvajici pocet), 0 = 256 (pred prikazem), jinak znaci uspesny konec 
#define REG_SCR 0x12
// 0. byte adresy
#define REG_LBA_0_7 0x13
// 1. byte adresy
#define REG_LBA_8_15 0x14
// 2. byte adresy
#define REG_LBA_16_23 0x15
// bity 24-27 adresy + vyber disku a modu
#define REG_LBA_24_27 0x16
	// LBA + disk 0
	#define LBA_DSC0 0xe0
	// LBA + disk 1
	#define LBA_DSC1 0xf0

// vraci status disku (nuluje priznaky), pri zapisu zahajuje prikazy 
#define REG_STATUS_CMD 0x17
	// Busy - zarizeni je zaneprazdneno, ostatni prikazy a registry vetsinou nereaguji
	#define STATUS_BSY 0x80
	// Drive ready - zarizeni je schopne prijimat vsechny prikazy
	#define STATUS_DRDY 0x40
	// Drive Fault - chyba zarizeni, chovani je zavysle na vyrobci  
	#define STATUS_DF 0x20
	// Device Seek complete - hlavicky jsou umisteny nad stopou
	#define STATUS_DSC 0x10
	// Data request - data jsou pripravena na cteni/zapis
	#define STATUS_DRQ 0x08
	// Cerrected data - vadna opravitelna data byla opravena, neukoncuje prenos
	#define STATUS_CORR 0x04
	// Index - mel by byt nastaven pri projiti indexem pod hlavickami - chovani je zavysle na vyrobci
	#define STATUS_IDX 0x02
	// Error - pro dalsi informace prectete Error registr 
	#define STATUS_ERR 0x01


// vraci status disku (nenuluje priznaky), pri zapisu rizeni nekterych funkci
#define REG_ALTER_DEV_CTRL 0x0e
	// bit, ktery ma byt nastaven
	#define DEC_MASK 0x08
	// zakaz preruseni, pokud je =1
	#define DEC_nIEN 0x02
	// softwarovy RESET
	#define DEC_RESET 0x04

// adresy disku, smysl maji pouze priznaky pritomnosti disku, je zastaraly
#define REG_ADDRESS 0x0f

/****** Prikazy IDE ******/

/**** prikazy cteni / zapisu ****/
// Precte dany pocet sektoru
#define CMD_READ_SECTORS 0x21
#define CMD_READ_SECTORS_RETRIES 0x20
// Zapise dany pocet sektoru
#define CMD_WRITE_SECTORS 0x31
#define CMD_WRITE_SECTORS_RETRIES 0x30

// Precte dany pocet sektoru, ale neposle je - test
#define CMD_READ_VERIFY 0x41
#define CMD_READ_VERIFY_RETRIES 0x40
// Stejne jako Write sector, ale kazdy sektor je po zapisu overen
#define CMD_WRITE_VERIFY 0x3C

// Read Buffer - cteni dat z bufferu zarizeni - melo by nasledovat po Write Buffer
// vraci 512Bytu
#define CMD_READ_BUFFER 0xE4
// Zapise 512Bytu do bufferu zarizeni, pokud nasleduje READ_BUFFER, pak by mely 
// byt navraccena stejna data 
#define CMD_WRITE_BUFFER 0xE8

// Nastavi pocet sektoru, ctenych/zapisovanych pomoci READ/WRITE MULTIPLE
// povolene hodnoty jsou mocninou cisla 2, maximalni hodnota je v MultipleSectors navracenych Identify device
#define CMD_SET_MULTIPLE_MODE 0xC6
// Podobne Read Sector, ale podle nastaveni SET MULTIPLE MODE
#define CMD_READ_MULTIPLE 0xC4
// Podobne Write Sector, ale podle nastaveni SET MULTIPLE MODE
#define CMD_WRITE_MULTIPLE 0xC5

/**** pomocne prikazy ****/
// Execute device diagnostic - diagnostika disku, vraci stav v error registru:
// 01h - Dev0 passed, Dev1 passed or not pressent
// 0, 02h-7fh - Dev0 failed, Dev1 passed or not pressent
// 81h - Dev0 passed, Dev0 failed
// 80h, 82h-ffh - Dev0 failed, Dev1 failed
// ignoruje vyber disku
#define CMD_DIAGNOSTIC 0x90
// Identify Device - odesle zpet data (512 bytu), popisujici disk, 
// -> struktura tIdeIdentify
#define CMD_IDENTIFY 0xEC
// Muze nastavit hlavicky na zadanou adresu, ale chovani je zavysle na vyrobci
#define CMD_SEEK 0x70
// Nastavi nektera nastaveni, zavysle na hodnote ve FEATURES registru:
// 02h - enable write chache, 82h - disable write chache
// 03h - set transfer mode, definovany v SCR:
	// 0 - PIO default, 1 - PIO default bez IORDY,
	// 8 + n(0-7) - PIO mod n s IORDY
	// 32 + n(0-7) Multiword DMA n mod  
// 04h - povol automaticke opravovani chyb, 84h - zakaz
// 33h - zakaz retry, 99h - povol retry
// 55h - zakaz vyhledavani napred pro cteni, AAh - povol
// 66h - zakaz navrat do zakladniho nastaveni po softwarovem resetu, CCh - povol 
// 77h - zakaz ECC, 88h - povol ECC
// 9Ah - nastav maximalni prumernou spotrebu - nastavi spotrebu zarizeni na
	// nejblizsi nizsi povolenou hodnotu = hodnota v SCR * 4mA
	// nastavi registr LBA_8_15 na minimalni moznou spotrebu a LBA_16_23 na maximalni (* 4 mA) 
// ABh - nastavi Maximum Prefetch v SCR
#define CMD_SET_FEATURES 0xEF

/**** prikazy rizeni spoteby ****/
// Check power mode - vraci stav v registru SCR (sector count)
// 0 = StandBy, 128 = Idle, 255 = Active
#define CMD_CHECK_POWER_MODE 0xE5

// Prejde do Idle rezimu + nastaveni casovac standby rezimu - data zapsana do SCR
// 0=vypnuto  1-240= *5s  241-251=(hodnota-240)*30 minut
// 252=21 minut  253=8-12 hodin  254=rezervovano  255=21minut 15s
#define CMD_IDLE 0xE3
// Prejde okamzite do Idle rezimu 
#define CMD_IDLE_IMMEDIATE 0xE1

// Prejde do rezimu spanku, je vhodne precist stauts register, ukoncen je pouze resetem
#define CMD_SLEEP 0xE6

// Nastavi casovac pro prechod do rezimu StandBY, hodnoty stejne, jako u IDLE 
#define CMD_STANDBY 0xE2
// Okamzite prejde do rezimu StandBy 
#define CMD_STANDBY_IMMEDIATE 0xE0

/**** Vymenitelna media ****/
// Door lock - uzamceni dvirek
#define CMD_DOOR_LOCK 0xDE
// Door unlock - odemceni dvirek
#define CMD_DOOR_UNLOCK 0xDF
// Media eject 
#define CMD_EJECT 0xED

/**** nepotrebne prikazy ****/ 
// NOP - vzdy provede chyby ABRT - zruseni prikazu - neni potreba 
#define CMD_NOP 0x00
// Provede rekalibraci, SCR by melo byt nastave na 1 a LBA adresa na 0 pred spustenim prikazu 
#define CMD_READ_RECALIBRATE 0x10

/***** SMART *****/
#define CMD_SMART 0xB0
// Definovano hodnotami v registrech FEATURE, LBA_8_15 a LBA_15_23 = (n,n,n)
// Disable - (D9h,4Fh,C2h) 
	// Ukonci veskere operace S.M.A.R.T.
// Enable - (D8h,4Fh,C2h) 
	// Spusti veskere operace S.M.A.R.T.
// Enable/Disable Attribute Autosave - (D2h,4Fh,C2h)
	// SCR =0 Vypnuto, =F1h Zapnuto 
// Read Attribute Treshold - (D1h,4Fh,C2h) 
	// Vraci 512B informaci o prahovych urovnich atributu
// Read Attribute Values - (D0h,4Fh,C2h) 
	// Vraci 512B informaci o urovnich atributu
// Return Status - (DAh,4Fh,C2h) 
	// Pokud neni prekrocen zadny limit, pak vraci (-,4Fh,C2h), jinak (-,F4h,2Ch)
// Save Attribute values - (D3h,4Fh,C2h)
	// Okamzite ulozi atributy do trvanlive pameti

/**** prikazy bezpecnosti ****/
// Vyzaduje 512Bytu s bezpecnostnim heslem -> struktura tIdePassword
#define CMD_SECURITY_DISABLE_PASSWORD 0xF6
// Musi byt zavolan pred Security Erase Unit - zabezpeceni proti nehodam 
#define CMD_SECURITY_ERASE_PREPARE 0xF3
// Nasleduje za Security Erase Prepare - maze uzivatelska data,
// vyzaduje 512Bytu s uzivatelskym heslem -> struktura tIdePassword
#define CMD_SECURITY_ERASE_UNIT 0xF4
// Uzamkne prikazy UN_LOCK, SET_PASSWORD, DISABLE_PASSWORD a ERASE_UNIT
// Odemknout lze jen hardwarovym resetem  
#define CMD_SECURITY_FREEZE_LOCK 0xF5
// Vyzaduje 512Bytu s bezpecnostnim heslem -> struktura tIdePassword
// USER + High = nastaveni hesla + uzamceni po hardwarovem resetu, lze odemknout i hlavnim heslem
// USER + Maximum = nastaveni hesla + uzamceni po hardwarovem resetu, lze odemknout pouze uzivatelskym heslem
// Hlavni = pouze nastaveni hesla, nezmeni stav
#define CMD_SECURITY_SET_PASSWORD 0xF1
// Odemceni heslem, pokud je ve stavu Maximalni bezpecnosti, pak pouze uzivatelskym
// Pouze 5 pokusu, pak je nutno provest hardwarovy reset
#define CMD_SECURITY_UNLOCK 0xF2

/**** DMA - zatim nepodporovano ****/
// Write DMA - zapis sektoru pomoci DMA kanalu
#define CMD_WRITE_DMA 0xCB
#define CMD_WRITE_DMA_RETRIES 0xCA
// Read DMA - cteni sektoru/sektoru a jejich prenos pomoci DMA kanalu
#define CMD_READ_DMA 0xC9
#define CMD_READ_DMA_RETRIES 0xC8

// Identify Device DMA - odesle zpet data (512 bytu), popisujici disk, v DMA rezimu 
// -> struktura tIdeIdentify
#define CMD_IDENTIFY_DMA 0xEE

/****** struktury ******/
// struktura pro prenos hesla
typedef struct {
	unsigned short type; // 0 =0 uzivatelske heslo  =1 hlavni heslo
		// 8 0=High 1=Maximum  - jen u SET_PASSWORD 
	unsigned short password[16]; // heslo, 32 Bytu
	unsigned short reserved[239];
} tIdePassword;

/****** pomocne struktury ******/

typedef struct {
	unsigned char status; // 0=neznamy 1=nenalezen 2=chybny
		// + 32 = LBA48
		// + 64 = SMART
		// 128=ATA 129=ATAPI
	unsigned char DMA; // aktualni DMA rezim
	unsigned char FeatureSet; // kopie z tIdentify
	unsigned char res; // reservovano
	unsigned long sectors; // celkovy pocet sektoru
} tIDE_HDD;

#define IDE_HDD_UNKNOWN 0
#define IDE_HDD_NONE 1
#define IDE_HDD_BAD 2
#define IDE_HDD_LBA48 32
#define IDE_HDD_SMART 64
#define IDE_HDD_ATA 128
#define IDE_HDD_ATAPI 129


// struktura, popisujici data z prikazu Identify device
typedef struct {
	unsigned short general; // 15 - 0=ATA, 1=ATAPI; 7=removable,6=nonremovable
	unsigned short numberOfLogicalCylinders; // neni potreba
	unsigned short Reserved1;
	unsigned short numberOfLogicalHeads; // neni potraba
	unsigned short Obsolete1; 
	unsigned short Obsolete2;
	unsigned short numberOfLogicalSectors; // per logical track - neni potreba
	unsigned short Vendor1;
	unsigned short Vendor2;
	unsigned short Vendor3;
	unsigned char SerialNumber[20]; // je treba zamenit sude/liche, vyplneno mezerami
	unsigned short Obsolete3; 
	unsigned short Obsolete4;
	unsigned short LongBytes; // pocet bytu pri LONG READ/WRITE - neni treba 
	unsigned char Firmware[8]; // retezec s verzi firmwaru, nemusi byt podporovan
	unsigned char ModelNumber[40]; // retezec se jmenem zarizeni
	unsigned short MultipleSectors; // maximalni pocet sektoru pri READ/WRITE MULTIPLE
	unsigned short Reserved2;
	unsigned short Capabilities; // 13 1=STANDBY TIMER je standartni
		// 11 1=IORDY podporovan, 0=muze byt podporovan    10 1=IORDY muze byt vypnut 
	unsigned short Reserved3;
	unsigned char Vendor4;
	unsigned char PIOTimingMode; // aktivni PIO mod
	unsigned short Vendor5;
	unsigned short FiledsReport; // 0 1=skupina 1 podporovana  1 1=skupina 2 podporovana
/**** skupina 1 ****/
	unsigned short CurrentLogicalCylinders; // neni treba
	unsigned short CurrentLogicalHeads; // neni treba
	unsigned short CurrentLogicalSectors; // per track - neni treba
	unsigned long CurrentCapacity; // sektoru
/**** konec skupiny 1 ****/
	unsigned short MultipleSectorsSetting; // neni treba
	unsigned long LBASectors; // pocet adresovatelnych sektoru pro LBA
	unsigned short Obsolete5;
	unsigned char MultiwordDMAModes; // podporovane rezimy
	unsigned char MultiwordDMAActive; // aktivni transportni mod Multiword DMA 
/**** skupina 2 ****/
	unsigned char PIOModes; // podporovane PIO Mody 
	unsigned char Reserved4;  
	unsigned short MinMultiwordDMATime; // minimalni cas cyklu [ns]
	unsigned short RecMultiwordDMATime; // doporuceny cas cyklu [ns]
	unsigned short MinPIOTime; // minimalni cas cyklu [ns]
	unsigned short MinPIOTimeIORDY; // minimalni cas cyklu pri rizeni IORDY [ns]
/**** konec skupiny 2 ****/ // ke skupine 2 patri jeste nasledujici 2 slova
	unsigned short Reserved5[11];
	unsigned short MajorVersion; // verze, =0 nebo FFFFh -> nepodporuje
		// 1 1=ATA-1  2 1=ATA-2  3 1=ATA-3
	unsigned short MinorVersion; // verze, =0 nebo FFFFh -> nepodporuje
	unsigned short FeatureSet; // =0 nebo FFFFh -> nepodporuje
		// 0 1=podporuje SMART  1 1=podporuje security  
		// 2 1=podporuje vymenitelnost media  3 1=podporuje rizeni spotreby
	unsigned short CommandSet; // neni treba
	unsigned short Reserved6[44];
	unsigned short Security; // 8 1=high 0=Maximum
		// 4 1=Count expired  3 1=Frozen  2 1=Locked  1 1=enabled  0 1=Supported
	unsigned short Vendor6[31];
	unsigned short Reserved7[96];
} tIdeIdentify;

/****** navratove hodnoty ******/
// vse probehlo v poradku
#define IDE_RET_OK 0 
// vyskytla se chyba v chybovem registru
#define IDE_RET_ERROR 1 
// prekrocen limit pokusu
#define IDE_RET_COUNT 2
// prikaz neni podporovan
#define IDE_RET_ABRT 3
// chyba zarizeni - chovani nedefinovano
#define IDE_RET_DF 4

/****** globalni promenne ******/
typedef struct {
	unsigned long Start,Stop,Curr; // sektory
	unsigned char disk;
	unsigned char status; // 0 = volny 1 = nacten 2 = nezapsana data 3 = nepouzit 
} tCache;

extern tCache IDE_Cache[4];
extern unsigned long IDE_Cache_Sector; // sektor, na ktery je nastavena Cache
extern unsigned char IDE_Cache_Disc; // disk, na ktery je nastavena Cache
extern unsigned char IDE_Cache_Last; // posledne pouzita Cache




extern signed char IDE_Active_Disc; // aktivni disk

extern int IDE_Wait_Count; // maximalni pocet cekani
extern tIDE_HDD IDE_HDD[2];

/****** Funkce cache disku ******/

// vse v poradku
#define IDE_C_OK 0
// chyba
#define IDE_C_ERR -1

// nastavi cislo ciloveho sektoru a cilovy disk, pokud jsou v cache nezapsana upravena data, tak je zapise
// a pokud existuje Cache, tak je do ni nacte
int IDE_Set_Sector(unsigned long sector,unsigned char disk);
// nacte data do data od zadaneho offsetu
// pokud jsou data v Cache, tak cte z ni, jinak se je snazi cist postupne z disku
// vraci pocet nactenych polozek
// data==NULL -> zapis/cteni z DMA kanalu
int IDE_Read(unsigned short offset,unsigned short count,unsigned char *data);
// podobne IDE_Read, ale pro zapis
// vraci pocet ulozenych polozek
// data==NULL -> zapis/cteni z DMA kanalu
int IDE_Write(unsigned short offset,unsigned short count,unsigned char *data);
// zapise pozmenena data ve vybrane cachi, cache = 4 -> vsechny, cache > 4 = aktualni 
int IDE_Cache_Flush(unsigned char cache);
// nastavi Cache0-3 na cacheovani zadane oblasti na disku
// Ssector > Esector - > vypnuta
void IDE_Cache_Set(unsigned char cache,unsigned long Ssector,unsigned long Esector,unsigned char disk);
// nastavi Cache na vychozi stav (c0 na cely disk 0, c1 na cely disk 1, c2 + c3 nic)
void IDE_Cache_Init();

/****** Funkce ******/

// provede hardwarovy reset
inline void IDE_Hard_Reset();

// ceka na aktivaci stavu astatus za neaktivnich stavu nstatus, ceka count pokusu
// pokud je nastaveno errdetect, pak se ukonci pri nastaveni bitu ERR 
int IDE_Wait_Status(unsigned char astatus,unsigned char nstatus,unsigned char errdetect,unsigned short count);
// precte hodnotu z registru
unsigned char IDE_Read_Reg(unsigned char addr);
// zapise hodnotu do registru
void IDE_Write_Reg(unsigned char addr,unsigned char value);
// nastavi adresu sektoru a spusti prikaz
void IDE_Spd_Sector(unsigned long addr,unsigned char cmd);
// nastavi adresu sektoru, zatim podporuje pouze prvnich 28 bitu
void IDE_Sector(unsigned long addr);
// Zmeni aktivni disk, 0= Master, ostatni = Slave
// Pozor, prepise nejvyssi 4 bity adresy!
void IDE_Change_Disc(int disc);

// vraci slovo z datoveho registru
unsigned short IDE_Read_Word();
// zapise slovo do datoveho registru
void IDE_Write_Word(unsigned short data);

// provede prikaz identify v PIO modu - vyzaduje 512B buffer 
// chybovy stav
int IDE_Identify(unsigned char *buff);

// Precte data z DMA bufferu, addr je max. 2047 
void IDE_ReadData(unsigned short addr,unsigned short count,unsigned char *buff);
// Zapise data do DMA bufferu, addr je max. 2047 
void IDE_WriteData(unsigned short addr,unsigned short count,unsigned char *buff);

// Prevede retezec z Identify na ANSI normu, pole str musi byt min. len+1 bytu dlouhe
// len = delka pole
void IDE_Convert_String(unsigned char *str,signed char len);

// provede prikaz a vyckava na jeho dokonceni (zruseni stavu BSY), pripadne na 
// chybovy stav
int IDE_CMD(unsigned char cmd);

// nastavi limit pro pocet cekani na stav
inline void Set_Wait_Count(int count);

// vraci, pokud je vse hotove
#define IDE_SETUP_DONE 0
// vraci IDE_HDD_Setup, pokud jeste neni dokoncena diagnostika disku
#define IDE_SETUP_WAIT 1

// Pokusi se nalezt a identifykovat disky
// prednastaveni a restart disku
void IDE_HDD_Setup_Init();
// cekani na dokonceni diagnostiky disku, zjisteni nastaveni
// pokud jeste neni dokoncena diagnostika, vraci IDE_SETUP_WAIT
int IDE_HDD_Setup();

// Upravena verze kvuli spravnemu poradi dat do kontroleru kanalu 
unsigned long FPGA_SPI_RW_A8_D3(unsigned char mode,unsigned char addr,unsigned long data);

// Zapise data do DMA kanalu, addr je max. 2047 
void IDE_CH_WriteData(unsigned short addr,unsigned short count);
// Precte data z DMA kanalu, addr je max. 2047 
void IDE_CH_ReadData(unsigned short addr,unsigned short count);

#endif   /* _IDE_ */

