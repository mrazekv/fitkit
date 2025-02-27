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
#define OPCODE_STATUS                 0x57    ///< stavov� registr
#define OPCODE_CONTINUOS_ARRAY_READ   0x68    ///< �ten�
#define OPCODE_PAGE_WRITE_BUF1        0x82    ///< z�pis p�es buffer 1
#define OPCODE_PAGE_WRITE_BUF2        0x85    ///< z�pis p�es buffer 2
#define OPCODE_WRITE_BUF1             0x84    ///< z�pis do bufferu 1
#define OPCODE_WRITE_BUF2             0x87    ///< z�pis do bufferu 2
#define OPCODE_BUF1_MEMORY_WERASE     0x83    ///< z�pis bufferu 1 do flash
#define OPCODE_BUF2_MEMORY_WERASE     0x86    ///< z�pis bufferu 2 do flash
#define OPCODE_MEMORY_BUF1            0x53    ///< �ten� z flash do bufferu 1
#define OPCODE_MEMORY_BUF2            0x55    ///< �ten� z flash do bufferu 2
#define OPCODE_READ_BUF1              0x54    ///< �ten� z buffer 1
#define OPCODE_READ_BUF2              0x56    ///< �ten� z buffer 2
#define OPCODE_PAGE_ERASE             0x81    ///< maz�n� stranky
#define OPCODE_BLOCK_ERASE            0x50    ///< maz�n� bloku
#define OPCODE_BUF1_MEMORY_WOERASE    0x88    ///< z�pis bufferu 1 do flash bez ERASE
#define OPCODE_BUF2_MEMORY_WOERASE    0x89    ///< z�pis bufferu 2 do flash bez ERASE
#define OPCODE_PAGE_READ              0x52    ///< cteni stranky

extern unsigned int  AdrBufferFlash;      ///< zapln�n� bufferu flash
extern unsigned int  CountPage;           ///< inkrementace str�nek

/**
 \brief Prevod z cisla stranky na adresu
 \return adresa zacatku stranky ve FLASH
 **/
unsigned int FLASH_PageToAddr(unsigned short page);

/**
 \brief Inicializace flash pam�ti - SPI rozhran�.
 \return 0 - chyba 
        \n 1 - v po��dku
 **/
unsigned char flash_init(char show_info);

/**
 \brief  �ten� stavov�ho registru z flash pam�ti.
 \return Status flash
 */
unsigned char FLASH_ReadStatus(void);

/**
 \brief �ten� RDY bitu ze stavov�ho registru z flash pam�ti.
 \return 1 - byla dokon�ena operace
         0 - flash pam� pracuje
 **/
unsigned char FLASH_IsReady(void);

/**
  \brief Fce vyp�e status registr pam�ti flash UniDX.
 */
void FLASH_print_status(void);

/**
 \brief Z�pis dat do bufferu flash pam�ti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 **/
void FLASH_WriteToBuffer(unsigned char *buf, unsigned long bufsz);

/**
 \brief Z�pis dat do bufferu flash pam�ti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_WriteDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                           unsigned int  Bufsz, unsigned int AdrBuffer);

/**
 \brief �ten� dat z bufferu flash pam�ti.
 \param TypeBuffer - 0 nebo 1, TypeBuffer = 0, tak buffer = 1, jinak buffer = 2
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param AdrBuffer - adresa v bufferu ve Flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataBuffer(unsigned char TypeBuffer, unsigned char *Buf,
                          unsigned int  Bufsz,      unsigned int AdrBuffer);


/**
 \brief �ten� str�nky dat z flash pam�ti p��mo.
 \param Buf - ukazatel na data
 \param Bufsz - velikost dat
 \param Page - ��slo str�nky ve flash, od 0 do CNT_PAGES-1
 \param AdrInPage - adresa ve str�nce pam�ti flash, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadData(unsigned char *Buf, unsigned int Bufsz,
                    unsigned int Page,  unsigned int AdrInPage);

/**
 \brief �ten� dat p��mo z adresy flash pam�ti.
 \param buf - ukazatel na data
 \param bufsz - velikost dat
 \param Adr - adresa ve flash pam�ti, od 0 do PAGE_SIZE-1
 **/
void FLASH_ReadDataA(unsigned char *Buf, unsigned int Bufsz, long int Adr);

/**
 \brief Z�pis dat z bufferu 1(2) do flash pam�ti.
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - str�nka, od 0 do NumPagesFlash
 **/
void FLASH_WriteBufferMemory(unsigned char TypeBuffer, unsigned int Page);

/**
 \brief �ten� dat z flash pam�ti do bufferu 1(2).
 \param TypeBuffer: 0 nebo 1, TypeBuffer = 1, tak buffer = 1, jinak buffer = 2
 \param Page - str�nka, od 0 do NumPagesFlash
 **/
void FLASH_ReadMemoryBuffer(unsigned char TypeBuffer, unsigned int Page);

/**
 \brief Programov�n� FPGA SpartanIII p��mo z pam�ti flash.
 \param Adr - adresa ze kter� se �tou data
 \param Length - d�lka dat
 \return 0 - chyba
        \n 1 - v po��dku
 **/
char FLASH_ProgFPGA(unsigned long Adr, unsigned long Length);

/**
 \brief Z�pis jednoho bytu do flash.
 \param Adr - adresa
 \param uChar - znak
 **/
void FLASH_WriteByte(unsigned long Adr, unsigned char uChar);

/**
 \brief Maz�n� jednoho bytu ve flash.
 \param Adr - adresa
 **/
void FLASH_EraseByte(unsigned long Adr);

/**
 \brief Maz�n� str�nky ve flash.
 \param Page - ��slo str�nky
 **/
void FLASH_ErasePage(unsigned int Page);

/**
 \brief Maz�n� bloku.
 \param block - blok flash
 **/
void FLASH_EraseBlock(unsigned int block);

/**
 \brief  Maz�n� cel� flash pam�ti.
**/
void FLASH_EraseAll(void);

/**
 \brief Fce vyp�e obsah cel� flash pam�ti na UART0.
**/
void FLASH_Dump_All(void);

/**
 \brief Fce vyp�e jeden blok flash pam�ti na UART0.
 \param block - ��slo bloku (0..)
 **/
void FLASH_Dump_Block(long int block);

/**
 \brief Fce vyp�e jednu str�nku flash pameti na UART0.
 \param page - �islo str�nky (0..)
**/
void FLASH_Dump_Page(long int page);

/**
  \brief Fce vyp�e blok flash pam�ti o dan� d�lce od adresy adr na UART0.
  \param adr - adresa (0..)
  \param len - d�lka vypisovan�ho bloku
 **/
void FLASH_Dump_Adr(long int adr, long int len);

/**
 \brief Test, zda-li je dana stranka prazdna (obsahuje same 0x00 nebo 0xFF)
 \param page - ��slo str�nky ve flash, od 0 do CNT_PAGES-1
 \return 0 - stranka obsahuje data, 1 stranka je prazdna
 **/
char FLASH_isPageBlank(unsigned short page);

/**
 \brief Vraci pocet dostupnych stranek FLASH pameti, ktera mohou byt pouzita pro uzivatelska data
 \return pocet stranek
 **/
unsigned long FLASH_availablepages(void);

#endif
