/*******************************************************************************
   flash_fpga.c:
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
     Sdileny ovladac pro flash pamet and FPGA SpartanIII.
*******************************************************************************/

#include "arch_specific.h"
#include "define.h"
#include "flash_fpga.h"
#include "uart.h"
#include "spi.h"

// SPI rozhrani a SpartanII
#define SPI_CSn     BIT0        ///< obecný výstup jako chip select, aktivní v 0
#define SPI_DO      BIT1        ///< datový výstup v master módu
#define SPI_DI      BIT2        ///< datový vstup v master módu
#define SPI_CLK     BIT3        ///< hodinovy výstup v master módu
#define DONE        BIT7        ///< DONE vstup do FPGA
#define INITn       BIT4        ///< INIT vstup/výstup FPGA, aktivní v 0
#define PROGn       BIT6        ///< PROG vstup do FPGA, aktivní v 0

// port SPI a SpartanII
#define FLPORTDIR   P5DIR       ///< direct port
#define FLPORTIN    P5IN        ///< vstupní port
#define FLPORTOUT   P5OUT       ///< vystupní port
#define FLPORTSEL   P5SEL       ///< selekèní port

/**
 \brief Fce realizující zpozdìní.
 **/

void delay_cycle_(unsigned int delay) {
  volatile unsigned int ii;
  for (ii=0; ii<delay; delay++) {
    WDG_reset();
  }
}

/**
 \brief Inicializace rozhraní pro flash pamìt a FPGA SpartanIII.
 **/
void FPGAConfig_init(void) {
  // #FPROG jako otevreny kolektor, pri inicializaci v HiZ
  P4OUT |= PROGn;               // jenom vnitrni priznak v MCU
  P4DIR &= ~PROGn;              // #FPROG jako vstup, HiZ stav
  // FDONE jako vstup
  P4DIR &= ~DONE;               // FDONE vstup
  // FINITn jako vstup
  P5DIR &= ~INITn;              // FINIT vstup
}

/**
 \brief Inicializace programování FPGA SpartanIII, data z FLASH/MCU.
 \return 0 - inicializace skonèila chybou
        \n 1 - inicializace v poøádku
 **/
unsigned char FPGAConfig_initialize(void) {
  // #FPROG do 0
  P4OUT &= ~PROGn;              // jenom vnitrni priznak v MCU
  P4DIR |= PROGn;               // #FPROG jako vystup do 0
  P4OUT &= ~PROGn;              // #FPROG do 0
  delay_cycle_(100);                 // zpozdeni
  P4DIR &= ~PROGn;              // #FPROG do HiZ (do 1)

  P5DIR &= ~INITn;              // FINITn jako vstup

  if ((P5IN & INITn) != 0) {    // #INIT neni v nule
    term_send_str_crlf("#INIT neni v 0 po #PROG inicializaci FPGA");
    return 0;
  }
  if ((P4IN & DONE) != 0) {     // DONE neni v 0
    term_send_str_crlf("DONE neni v 0 po #PROG inicializaci FPGA");
    return 0;
  }
  delay_cycle_(100);                // zpozdeni minimalne 100us
  if ((P5IN & INITn) == 0) {    // #INIT neni v 1
    term_send_str_crlf("#INIT neni v 1 po vynulovani FPGA");
    return 0;
  }

  return 1;                     // inicializace v poradku, cekani na nacteni dat
}

/**
 \brief Konec programování FPGA SpartanIII, kontrola naprogramování.
 \return 0 - programování skonèilo chybou
        \n 1 - programování v poøádku
 **/
unsigned char FPGAConfig_finalize(void) {
  delay_cycle_(100);                // zpozdeni
  if ((P4IN & DONE) == 0) {    // DONE neni v 1
    term_send_str_crlf("DONE neni v  1 po programovani FPGA");
    return 0;
  }

  P5DIR |= INITn;      // FINITn jako vystup
  return 1;            // inicializace v poradku, cekani na nacteni dat
}

/**
 \brief Detekce FPGA (hack).
 \return 0 - FPGA neodpovida zvolenemu IDcode
        \n 1 - FPGA odpovida zvolenemu IDcode
 **/
unsigned char FPGAConfig_detect_fpga(unsigned long idcode)
{
  int il;
  unsigned char detected;

  if (FPGAConfig_initialize() == 0) {
    return 0;
  }

  SPI_read_wait_write(0xFF); SPI_read_wait_write(0xFF); SPI_read_wait_write(0xFF); SPI_read_wait_write(0xFF); //dummy
  SPI_read_wait_write(0xAA); SPI_read_wait_write(0x99); SPI_read_wait_write(0x55); SPI_read_wait_write(0x66); //sync
/*
  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x07); //RCRC
*/
  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x60); SPI_read_wait_write(0x01); //flr
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x0F); //len (words)
  // XC3S50  - FLR=24

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x20); SPI_read_wait_write(0x01); //cor
  SPI_read_wait_write(0x40); SPI_read_wait_write(0x00); SPI_read_wait_write(0x3F); SPI_read_wait_write(0xE5); //enable crc

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0xC0); SPI_read_wait_write(0x01); //idcode write
  SPI_read_wait_write(idcode >> 24); SPI_read_wait_write((idcode >> 16) & 0xFF); SPI_read_wait_write((idcode >> 8) & 0xFF); SPI_read_wait_write(idcode & 0xFF); 

/*
  SPI_read_wait_write(0x30); SPI_read_wait_write(0x00); SPI_read_wait_write(0xC0); SPI_read_wait_write(0x01); //mask
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); 

  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x09); //SWITCH

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x00); SPI_read_wait_write(0x20); SPI_read_wait_write(0x01); //far
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); 
  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");
*/  

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x01); //WCFG

//  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x00); SPI_read_wait_write(0x40); SPI_read_wait_write(0x10); //write fdri 44 words
for (il=0;il<0x10;il++) {  
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); 
}
  detected = FPGAConfig_INITn() > 0;
             
  //abort
  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x03); //LFRM
  
/*
  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x0A); //GRESTORE

  term_send_str("E");
  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");
  */
/*

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x03); //LFRM

  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");
*/  
/*
for (il=0;il<50;il++) {  
  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");
  SPI_read_wait_write(0x20); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); //nop
}

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x05); //START

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x00); SPI_read_wait_write(0xA0); SPI_read_wait_write(0x01); //CTL
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); 

  SPI_read_wait_write(0x30); SPI_read_wait_write(0x01); SPI_read_wait_write(0x80); SPI_read_wait_write(0x01); //cmd
  SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x0D); //DESYNCH
  
term_send_str("G");
for (il=0;il<50;il++) {  
  term_send_num(FPGAConfig_DONE()); term_send_str(","); term_send_num(FPGAConfig_INITn()); term_send_str(",");
  SPI_read_wait_write(0x20); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); SPI_read_wait_write(0x00); //nop
}
*/
  
  P5DIR |= INITn;  // FINITn jako vystup

  return detected != 0;
}

