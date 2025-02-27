/*******************************************************************************
   
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

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

   Popis:
     Nizko-urovnove fce pro praci z flash.
*******************************************************************************/

#include <string.h>
#include "arch_specific.h"
#include "fitkitlib.h"
#include "define.h"
#include "globfun.h"
#include "uart.h"
#include "flash_fpga.h"
#include "spi.h"
#include "flash.h"
#include "timer_b.h"

unsigned short CNT_PAGES = 1024;
unsigned char TypeBufferFlash;     ///< typ bufferu pro zápis, 0 a 1
unsigned int  AdrBufferFlash;      ///< zaplnìní bufferu flash
unsigned int  CountPage;           ///< inkrementace stránek

/**
 \brief Prevod z cisla stranky na adresu
 \return adresa zacatku stranky ve FLASH
 **/
unsigned int FLASH_PageToAddr(unsigned short page) {
  return (unsigned int)page*PAGE_SIZE;
}

unsigned char flash_init_(void) 
{
  unsigned char ii, ipos;
  unsigned char status;

  // 100x se zkusim dotazat na stav FLASHky, pokud je vzdy negativni -> CHYBA
  // pokud je 10x za sebou positivni -> OK
  for (ii=0, ipos=0; ii<100; ii++) {
    WDG_reset();

    status = FLASH_ReadStatus();
/*
    term_send_str("FLASH status:");
    term_send_hex(status);
    term_send_crlf();
*/
    ipos = ((status != 0xFF) && (status != 0x00) && (status & 0x80)) ? (ipos + 1) : 0;
    if (ipos == 10)
       return status;         // spravna inicializace
  }
  AdrBufferFlash = 0;
  CountPage = 0;
  return 0;             // spatna inicializace
}

/**
 \brief Inicializace flash pamìti - SPI rozhraní.
 \param show_info  - zda-li zobrazit informaci o nalezene pameti
 \return 0 - chyba, jinak status FLASH pameti
 **/
unsigned char flash_init(char show_info) 
{
   unsigned char status;

   if (show_info) 
   {
      term_send_str("Inicializace FLASH: ");
      status = flash_init_();
      if (status == 0) //chyba inicializace
      {
         term_send_str_crlf("chyba");
         return 0;
      }

      switch (status & 0x38) 
      {
         case (0x18): term_send_str_crlf("AT45DB041D"); CNT_PAGES = 2048;break;

         case (0x10): term_send_str_crlf("AT45DB021B"); CNT_PAGES = 1024;break;

         default: term_send_str("neznamy typ ("); term_send_hex(status); term_send_str_crlf(")");
      }

      return status;
   } else {
      return flash_init_();
   }
}

/**
 \brief  Pocet dostupnych stranek FLASH pameti pro uzivatelska data
 */

unsigned long FLASH_availablepages(void) {
  return CNT_PAGES - FLASH_USER_DATA_PAGE;
}

/**
 \brief  Poèet blokù FLASH pamìti
 */

unsigned long FLASH_blocks(void) {
  return CNT_PAGES / CNT_PAGES_IN_BLOCK;
}


/**
 \brief  Velikost FLASH pamìti
 */

unsigned long FLASH_size(void) {
  return (unsigned long) CNT_PAGES * PAGE_SIZE;
}

/**
 \brief  Ètení stavového registru z flash pamìti.
 \return Status flash
 */
unsigned char FLASH_ReadStatus(void) {
  unsigned char chr;

  SPI_set_cs_FLASH(1);
  SPI_read_write(OPCODE_STATUS);
  chr = SPI_read_write(0);
  SPI_set_cs_FLASH(0);

  return (chr);
}

/**
 \brief Ètení RDY bitu ze stavového registru z flash pamìti.
 \return 1 - byla dokonèena operace
         0 - flash pamì pracuje
 **/
unsigned char FLASH_IsReady(void) {
  if ((FLASH_ReadStatus() & 0x80) == 0)  // test 'ready' bitu
     return (0);                            // flash je busy
  return (1);                          // flash je ready
}

/**
  \brief Fce vypíše status registr pamìti flash UniDX.
 */
void FLASH_print_status(void) {
  unsigned char chr;

  term_send_str_crlf("FLASH INFO");
  term_send_str("  kapacita flash [kB]:   "); term_send_num((unsigned int)(FLASH_size() >> 10)); term_send_crlf();
  term_send_str("  velikost stranky [B]:  "); term_send_num(PAGE_SIZE); term_send_crlf();
  term_send_str("  velikost bufferu [B]:  "); term_send_num(BUFFER_SIZE); term_send_crlf();
  term_send_str("  pocet stranek:         "); term_send_num(CNT_PAGES); term_send_crlf();
  term_send_str("  pocet bloku:           "); term_send_num(FLASH_blocks()); term_send_crlf();
  term_send_str("  konfigurace FPGA [pg]: "); term_send_num(FPGA_BIN_PAGES); 
  term_send_str(" ("); term_send_num(FLASH_FPGA_BIN_PAGE); term_send_str(" - "); term_send_num(FLASH_USER_DATA_PAGE-1); term_send_str_crlf(")"); 
  term_send_str("  uzivatelska data [pg]: "); term_send_num(CNT_PAGES - FLASH_USER_DATA_PAGE + 1); 
  term_send_str(" ("); term_send_num(FLASH_USER_DATA_PAGE); term_send_str(" - "); term_send_num(CNT_PAGES-1); term_send_str_crlf(")"); 
  term_send_crlf();

  chr = FLASH_ReadStatus();
  term_send_str_crlf("STATUS REGISTR");
  term_send_str_crlf("  rdy cmp id id id x x x");term_send_str("   ");
  term_send_bool(chr & 0x80);term_send_str("   ");
  term_send_bool(chr & 0x40);term_send_str("   ");
  term_send_bool(chr & 0x20);term_send_str("  ");
  term_send_bool(chr & 0x10);term_send_str("  ");
  term_send_bool(chr & 0x08);term_send_str(" ");
  term_send_bool(chr & 0x04);term_send_str(" ");
  term_send_bool(chr & 0x02);term_send_str(" ");
  term_send_bool(chr & 0x01);term_send_str(" ");
  term_send_crlf();
  
}

/**
 \brief Zápis dat do bufferu flash pamìti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 **/
void FLASH_WriteToBuffer(unsigned char *buf, unsigned long bufsz) {
  unsigned int SizeFreeBuf;

  // dokud mam co zapisovat
  while (bufsz > 0) {

    WDG_reset();

    FLASH_ReadMemoryBuffer(0, CountPage);

    SizeFreeBuf = BUFFER_SIZE - AdrBufferFlash;  // velikost voneho mista v bufferu flash

    if (bufsz >= SizeFreeBuf) {                  // vsechna data nelze zapsat najednou
                                                 // zapis dat do bufferu flash
      FLASH_WriteDataBuffer(0, buf, SizeFreeBuf, AdrBufferFlash);
      bufsz -= SizeFreeBuf;                      // uz zapisi o neco mene dat
      buf   += SizeFreeBuf;                      // posun na dalsi data
      FLASH_WriteBufferMemory(0, CountPage);     // zapis stranky ve flash bufferu do flash
      CountPage++;                               // budu zapisovat do dalsi stranky
      AdrBufferFlash = 0;                        // adresa v bufferu je nula
    }
    else {                                       // vsechna data lze zapsat najednou
                                                 // zapis dat do bufferu flash
      FLASH_WriteDataBuffer(0, buf, bufsz, AdrBufferFlash);
      AdrBufferFlash += bufsz;                   // posun offsetu v bufferu
      bufsz = 0;                                 // dale uz nezapisuji
      FLASH_WriteBufferMemory(0, CountPage);     // zapis stranky ve flash bufferu do flash
    }
  } /* while */
}

/**
 \brief Zápis dat do bufferu flash pamìti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_WriteDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                           unsigned int  Bufsz, unsigned int AdrBuffer) {
  unsigned int  ii;

  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }

  SPI_set_cs_FLASH(1);

  if (TypeBuffer == 0)
    SPI_read_write(OPCODE_WRITE_BUF1);
  else
    SPI_read_write(OPCODE_WRITE_BUF2);

  SPI_read_write(0);
  SPI_read_write(AdrBuffer / 256);
  SPI_read_write(AdrBuffer % 256);

  WDG_reset();

  for (ii = 0; ii < Bufsz; ii++) {
    SPI_read_write(Buf[ii]);
  }

  WDG_reset();

  SPI_set_cs_FLASH(0);
}

/**
 \brief Ètení dat z bufferu flash pamìti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu ve Flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                          unsigned int  Bufsz,      unsigned int AdrBuffer) {
  unsigned int ii;

  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }

  SPI_set_cs_FLASH(1);

  if (TypeBuffer == 0)
    SPI_read_write(OPCODE_READ_BUF1);
  else
    SPI_read_write(OPCODE_READ_BUF2);

  SPI_read_write(0);
  SPI_read_write(AdrBuffer / 256);
  SPI_read_write(AdrBuffer % 256);
  SPI_read_write(0);

  for (ii=0; ii<Bufsz; ii++) {

    WDG_reset();

    Buf[ii] = SPI_read_write(0);
  }

  SPI_set_cs_FLASH(0);
}

void inline FLASH_ReadData_Initialize(unsigned int Page,  unsigned int AdrInPage) {

  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }

  SPI_set_cs_FLASH(1);
  SPI_read_write(OPCODE_CONTINUOS_ARRAY_READ);

  SPI_read_write(Page >> 7);
  SPI_read_write(((Page << 1) & 0xFF) + (AdrInPage >> 8));
  SPI_read_write(AdrInPage & 0xFF);

  SPI_read_write(0);
  SPI_read_write(0);
  SPI_read_write(0);
  SPI_read_write(0);

}

void inline FLASH_ReadData_Finalize() {
  SPI_set_cs_FLASH(0);
}

/**
 \brief Ètení stránky dat z flash pamìti pøímo.
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param Page - èíslo stránky ve flash, od 0 do CNT_PAGES-1
 \param AdrInPage - adresa ve stránce pamìti flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadData(unsigned char *Buf, unsigned int Bufsz,
                    unsigned int Page,  unsigned int AdrInPage) {
  unsigned int ii;

  FLASH_ReadData_Initialize(Page, AdrInPage);

  for (ii = 0; ii < Bufsz; ii++) {

    WDG_reset();

    Buf[ii] = SPI_read_write(0);
  }

  FLASH_ReadData_Finalize();
}


/**
 \brief Test, zda-li je dana stranka prazdna (obsahuje same 0x00 nebo 0xFF)
 \param Page - èíslo stránky ve flash, od 0 do CNT_PAGES-1
 \return 0 - stranka obsahuje data, 1 stranka je prazdna
 **/
char FLASH_isPageBlank(unsigned short page) {
  unsigned short ii;
  unsigned char ch, charFF = 1, char00 = 1;

  FLASH_ReadData_Initialize(page, 0);
  for (ii = 0; ii < PAGE_SIZE; ii++) {
    ch = SPI_read_write(0);
    if (ch != 0x00)
      char00 = 0;
    if (ch != 0xFF)
      charFF = 0;

    WDG_reset();
  }
  FLASH_ReadData_Finalize();

  return (charFF | char00);
}

/**
 \brief Ètení dat pøímo z adresy flash pamìti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 \param Adr - adresa ve flash pamìti, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataA(unsigned char *Buf, unsigned int Bufsz, long int Adr) {
  unsigned int Page, Offset;
  Page   = (Adr / PAGE_SIZE);
  Offset = (Adr % PAGE_SIZE);

  FLASH_ReadData(Buf, Bufsz, Page, Offset);
}

/**
 \brief Zápis dat z bufferu 1(2) do flash pamìti.
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - stránka, od 0 do NumPagesFlash
 **/
void FLASH_WriteBufferMemory(unsigned char TypeBuffer, unsigned int Page) {

  while (FLASH_IsReady() != 1) {
    WDG_reset();      // dokud je flash busy
  }

  SPI_set_cs_FLASH(1);                    // povoleni flash

  if (TypeBuffer == 0)          // zapis operacniho kodu
    SPI_read_write(OPCODE_BUF1_MEMORY_WERASE);
  else
    SPI_read_write(OPCODE_BUF2_MEMORY_WERASE);

  SPI_read_write(Page >> 7);
  SPI_read_write((Page << 1) & 0xFF);
  SPI_read_write(0);

  SPI_set_cs_FLASH(0);                    // dokonceni operace

  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }
}

/**
 \brief Ètení dat z flash pamìti do bufferu 1(2).
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - stránka, od 0 do NumPagesFlash
 **/
void FLASH_ReadMemoryBuffer(unsigned char TypeBuffer, unsigned int Page) {

  while (FLASH_IsReady() != 1) {
    WDG_reset();       // dokud je flash busy
  }

  SPI_set_cs_FLASH(1);

  if (TypeBuffer == 0)
    SPI_read_write(OPCODE_MEMORY_BUF1);
  else
    SPI_read_write(OPCODE_MEMORY_BUF2);

  SPI_read_write(Page >> 7);
  SPI_read_write((Page << 1) & 0xFF);
  SPI_read_write(0);

  SPI_set_cs_FLASH(0);
}


/**
 \brief Programování FPGA SpartanIII pøímo z pamìti flash.
 \param Adr - adresa ze které se ètou data
 \param Length - délka dat
 \return 0 - chyba
        \n 1 - v poøádku
 **/
char FLASH_ProgFPGA(unsigned long Adr, unsigned long Length) {
  unsigned long il;
  unsigned int Page, AdrInPage;
  unsigned char Dataf;

  Page      = (Adr / PAGE_SIZE);
  AdrInPage = (Adr % PAGE_SIZE);

  // cekam az je flash Ready
  while (FLASH_IsReady() != 1) {
        WDG_reset();
  }

  // nastaveni adresy ve FLASH a nastaveni FLASH pro cteni
  SPI_set_cs_FLASH(1);
  SPI_read_write(OPCODE_CONTINUOS_ARRAY_READ);

  SPI_read_write(Page >> 7);
  SPI_read_write(((Page << 1) & 0xFF) + (AdrInPage >> 8));
  SPI_read_write(AdrInPage & 0xFF);
  SPI_read_write(0);
  SPI_read_write(0);
  SPI_read_write(0);
  SPI_read_write(0);

  // nacteni jednoho bytu
  Dataf = SPI_read_write(0);

  // nastaveni FPGA pro prijem dat, PROG do 1, kontrola INIT a DONE
  if (FPGAConfig_initialize() == 0) {
    return (0);
  }

  // nacitani dat a tim padem i programovani FPGA
  for (il = 0; il < Length; il++) {
    // vyslani datoveho byte pres SPI do FPGA + nacteni dat
    Dataf = SPI_read_wait_write(Dataf);
/*
if (il < 32) {
term_send_hex(Dataf);
}
*/

    if ((il % 1024) == 0) {
      term_send_char('.');

      WDG_reset();

      if (FPGAConfig_INITn() == 0) {  // #INIT neni v 1
        term_send_str("[chybna konfigurace]"); //V prubehu programovani #INIT je v 0 - chybna programovaci data"
        return(0);
      }

    }

    if (FPGAConfig_DONE()) //nastaven signal DONE, FPGA je naprogramovano
       break;

  }

  SPI_read_write(0);
  SPI_set_cs_FLASH(0);

  // ukonceni programovani, kontrola INIT a DONE
  if (FPGAConfig_finalize() == 0) {
    return (0);
  }

  return (1);   // naprogramovano dobre
}

/**
 \brief Zápis jednoho bytu do flash.
 \param Adr - adresa
 \param uChar - znak
 **/
void FLASH_WriteByte(unsigned long Adr, unsigned char uChar) {
  unsigned int Page, AdrInPage;
  Page      = (Adr / PAGE_SIZE);
  AdrInPage = (Adr % PAGE_SIZE);

  FLASH_ReadMemoryBuffer(0, Page);
  delay_ms(50);
  FLASH_WriteDataBuffer(0, &uChar, 1, AdrInPage);
  FLASH_WriteBufferMemory(0, Page);
  delay_ms(50);
}

/**
 \brief Mazání jednoho bytu ve flash.
 \param Adr - adresa
 **/
void FLASH_EraseByte(unsigned long Adr) {
  FLASH_WriteByte(Adr, ERS_CHAR);
}

/**
 \brief Mazání stránky ve flash.
 \param Page - èíslo stránky
 **/
void FLASH_ErasePage(unsigned int Page) {

  // cekam az je flash Ready
  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }

  SPI_set_cs_FLASH(1);
  SPI_read_write(OPCODE_PAGE_ERASE);

  SPI_read_write(Page >> 7);
  SPI_read_write((Page << 1) & 0xFF);
  SPI_read_write(0);
  SPI_set_cs_FLASH(0);

}

/**
 \brief Mazání bloku.
 \param block - blok flash
 **/
void FLASH_EraseBlock(unsigned int block) {

  // cekam az je flash Ready
  while (FLASH_IsReady() != 1) {
    WDG_reset();
  }

  SPI_set_cs_FLASH(1);

  SPI_read_write(OPCODE_BLOCK_ERASE);

  SPI_read_write(block >> 4);
  SPI_read_write((block << 4) & 0xFF);
  SPI_read_write(0);
  SPI_set_cs_FLASH(0);

}

/**
 \brief  Mazání celé flash pamìti.
**/
void FLASH_EraseAll(void) {
  unsigned long il;

  // mazu jednotlive bloky
  for (il = 0; il < FLASH_blocks(); il++) {

    WDG_reset();

    FLASH_EraseBlock(il);
    if ((il % 128) == 0)
      term_send_char('.');
  }
}

/**
 \brief Fce vypíše obsah celé flash pamìti na UART0.
**/
void FLASH_Dump_All(void) {
  long int il;

  for (il = 0; il < FLASH_blocks(); il++) {

    WDG_reset();

    FLASH_Dump_Block(il);
  }
}

/**
 \brief Fce vypíše jeden blok flash pamìti na UART0.
 \param block - èíslo bloku (0..)
 **/
void FLASH_Dump_Block(long int block) {
  long int il;

  for (il = 0; il < CNT_PAGES_IN_BLOCK; il++) {

    WDG_reset();

    FLASH_Dump_Page(il);
  }
}

/**
 \brief Fce vypíše jednu stránku flash pameti na UART0.
 \param page - èislo stránky (0..)
**/
void FLASH_Dump_Page(long int page) {
  FLASH_Dump_Adr(page*PAGE_SIZE, PAGE_SIZE);
}

/**
  \brief Fce vypíše blok flash pamìti o dané délce od adresy adr na UART0.
  \param adr - adresa (0..)
  \param len - délka vypisovaného bloku
 **/
void FLASH_Dump_Adr(long adr, long len) {
  long int      ik, jj;
  unsigned char row_len;               // delka radku vypisu je 8Bytu
  unsigned char buf[17];

  row_len = 8;                         // delka radku vypisu je 8Bytu
  if (adr > (long)FLASH_size()) {
    term_send_str("Zadana adesa je mimo rozsah.");
    return;
  }

  if (flash_init(0) == 0)
    return;

  term_send_str("Obsah flash pameti na adrese");
  term_send_num(adr);
  term_send_str(" (");
  term_send_hex(adr);
  term_send_str_crlf("):");
  term_send_str_crlf("          0  1  2  3  4  5  6  7");

  for (ik = 0; ik < len; ik+=row_len) {
    // Vypisi offset
    long2str(ik, (unsigned char *)&buf, 16);
    for (jj = 0; jj < (8 - strlen(buf)); jj++)
      term_send_char('0');
    term_send_str(buf);

    term_send_str(": ");

    // prectu blok dat
    FLASH_ReadDataA((unsigned char *)&buf, row_len, adr + ik);

    // vypisi hexa
    for (jj = 0; jj < row_len; jj++) {

      WDG_reset();

      if ((ik+jj) < len) {
        term_send_char_hex(buf[jj]);
        term_send_char(' ');
      }
      else
        term_send_char(' ');
    }
    term_send_str(" | ");

    // vypisi znaky
    for (jj=0; jj<row_len; jj++) {

      WDG_reset();

      if ((ik+jj) < len) {
        if ((buf[jj] >= 0x20) && (buf[jj] <= 127))
          term_send_char(buf[jj]);
        else
          term_send_char('.');
      }
    }
    term_send_crlf();
  }
}

