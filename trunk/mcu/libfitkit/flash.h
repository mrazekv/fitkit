/*******************************************************************************
   flash.h:
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

#ifndef _FLASH_H_
#define _FLASH_H_

/******************************************************************************/
/* operacni kody pro flash */
#define OPCODE_STATUS                 0x57    ///< stavový registr
#define OPCODE_CONTINUOS_ARRAY_READ   0x68    ///< ètení
#define OPCODE_PAGE_WRITE_BUF1        0x82    ///< zápis pøes buffer 1
#define OPCODE_PAGE_WRITE_BUF2        0x85    ///< zápis pøes buffer 2
#define OPCODE_WRITE_BUF1             0x84    ///< zápis do bufferu 1
#define OPCODE_WRITE_BUF2             0x87    ///< zápis do bufferu 2
#define OPCODE_BUF1_MEMORY_WERASE     0x83    ///< zápis bufferu 1 do flash
#define OPCODE_BUF2_MEMORY_WERASE     0x86    ///< zápis bufferu 2 do flash
#define OPCODE_MEMORY_BUF1            0x53    ///< ètení z flash do bufferu 1
#define OPCODE_MEMORY_BUF2            0x55    ///< ètení z flash do bufferu 2
#define OPCODE_READ_BUF1              0x54    ///< ètení z buffer 1
#define OPCODE_READ_BUF2              0x56    ///< ètení z buffer 2
#define OPCODE_PAGE_ERASE             0x81    ///< mazání stranky
#define OPCODE_BLOCK_ERASE            0x50    ///< mazání bloku
#define OPCODE_BUF1_MEMORY_WOERASE    0x88    ///< zápis bufferu 1 do flash bez ERASE
#define OPCODE_BUF2_MEMORY_WOERASE    0x89    ///< zápis bufferu 2 do flash bez ERASE
#define OPCODE_PAGE_READ              0x52    ///< cteni stranky

extern unsigned int  AdrBufferFlash;      ///< zaplnìní bufferu flash
extern unsigned int  CountPage;           ///< inkrementace stránek

/**
 \brief Prevod z cisla stranky na adresu
 \return adresa zacatku stranky ve FLASH
 **/
unsigned int FLASH_PageToAddr(unsigned short page);

/**
 \brief Inicializace flash pamìti - SPI rozhraní.
 \return 0 - chyba 
        \n 1 - v poøádku
 **/
unsigned char flash_init(char show_info);

/**
 \brief  Ètení stavového registru z flash pamìti.
 \return Status flash
 */
unsigned char FLASH_ReadStatus(void);

/**
 \brief Ètení RDY bitu ze stavového registru z flash pamìti.
 \return 1 - byla dokonèena operace
         0 - flash pamì pracuje
 **/
unsigned char FLASH_IsReady(void);

/**
  \brief Fce vypíše status registr pamìti flash UniDX.
 */
void FLASH_print_status(void);

/**
 \brief Zápis dat do bufferu flash pamìti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 **/
void FLASH_WriteToBuffer(unsigned char *buf, unsigned long bufsz);

/**
 \brief Zápis dat do bufferu flash pamìti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_WriteDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                           unsigned int  Bufsz, unsigned int AdrBuffer);

/**
 \brief Ètení dat z bufferu flash pamìti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu ve Flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                          unsigned int  Bufsz,      unsigned int AdrBuffer);


/**
 \brief Ètení stránky dat z flash pamìti pøímo.
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param Page - èíslo stránky ve flash, od 0 do CNT_PAGES-1
 \param AdrInPage - adresa ve stránce pamìti flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadData(unsigned char *Buf, unsigned int Bufsz,
                    unsigned int Page,  unsigned int AdrInPage);

/**
 \brief Ètení dat pøímo z adresy flash pamìti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 \param Adr - adresa ve flash pamìti, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataA(unsigned char *Buf, unsigned int Bufsz, long int Adr);

/**
 \brief Zápis dat z bufferu 1(2) do flash pamìti.
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - stránka, od 0 do NumPagesFlash
 **/
void FLASH_WriteBufferMemory(unsigned char TypeBuffer, unsigned int Page);

/**
 \brief Ètení dat z flash pamìti do bufferu 1(2).
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - stránka, od 0 do NumPagesFlash
 **/
void FLASH_ReadMemoryBuffer(unsigned char TypeBuffer, unsigned int Page);

/**
 \brief Programování FPGA SpartanIII pøímo z pamìti flash.
 \param Adr - adresa ze které se ètou data
 \param Length - délka dat
 \return 0 - chyba
        \n 1 - v poøádku
 **/
char FLASH_ProgFPGA(unsigned long Adr, unsigned long Length);

/**
 \brief Zápis jednoho bytu do flash.
 \param Adr - adresa
 \param uChar - znak
 **/
void FLASH_WriteByte(unsigned long Adr, unsigned char uChar);

/**
 \brief Mazání jednoho bytu ve flash.
 \param Adr - adresa
 **/
void FLASH_EraseByte(unsigned long Adr);

/**
 \brief Mazání stránky ve flash.
 \param Page - èíslo stránky
 **/
void FLASH_ErasePage(unsigned int Page);

/**
 \brief Mazání bloku.
 \param block - blok flash
 **/
void FLASH_EraseBlock(unsigned int block);

/**
 \brief  Mazání celé flash pamìti.
**/
void FLASH_EraseAll(void);

/**
 \brief Fce vypíše obsah celé flash pamìti na UART0.
**/
void FLASH_Dump_All(void);

/**
 \brief Fce vypíše jeden blok flash pamìti na UART0.
 \param block - èíslo bloku (0..)
 **/
void FLASH_Dump_Block(long int block);

/**
 \brief Fce vypíše jednu stránku flash pameti na UART0.
 \param page - èislo stránky (0..)
**/
void FLASH_Dump_Page(long int page);

/**
  \brief Fce vypíše blok flash pamìti o dané délce od adresy adr na UART0.
  \param adr - adresa (0..)
  \param len - délka vypisovaného bloku
 **/
void FLASH_Dump_Adr(long int adr, long int len);

/**
 \brief Test, zda-li je dana stranka prazdna (obsahuje same 0x00 nebo 0xFF)
 \param page - èíslo stránky ve flash, od 0 do CNT_PAGES-1
 \return 0 - stranka obsahuje data, 1 stranka je prazdna
 **/
char FLASH_isPageBlank(unsigned short page);

/**
 \brief Vraci pocet dostupnych stranek FLASH pameti, ktera mohou byt pouzita pro uzivatelska data
 \return pocet stranek
 **/
unsigned long FLASH_availablepages(void);

#endif
