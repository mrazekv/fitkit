/*******************************************************************************
   fitkitlib.c:
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

*******************************************************************************/

#include <math.h>
#include <stdio.h>
#include "arch_specific.h"
#include "define.h"
#include "globfun.h"
#include "timer_b.h"
#include "uart.h"
#include "cmd.h"
#include "flash_fpga.h"
#include "flash.h"
#include "terminal.h"
#include "fpga.h"
#include "spi.h"
#include "fitkitlib.h"

//#include "xmodem.h"
#include "filetransfer.h"

unsigned long FPGA_config_length = 54908;

/**
  \brief  Jednoduchá funkce realizující zpoždení.
  \param  delay - poèet tiku, které se èeka
 **/
void delay_cycle(unsigned int delay) {
  volatile unsigned int i = 0;

  while (1) {
     if (i == delay) return;
     i++;
  }
}


/**
 \brief Inicializace a nastaveni hodin pro MCU a FPGA
 \return Vždy hodnota 0
 **/
int clock_system_init(void) {
  WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT

  #if defined MSP_16X
    /// pocatecni nastaveni hodin
    BCSCTL1 = RSEL2 + RSEL1 + RSEL0;      /// nizka frekvence na LFXT1, vys. fr. na LFXT2
                                          /// DCO na 3.0MHz, ACLK na 32.768kHz
    /// nabeh krystalovych oscilatoru
    _BIC_SR(OSCOFF);                      /// nulovani OscOFF, povoleni LFXT1
    do {
      IFG1 &= ~OFIFG;                     /// nulovani OFIFG
      delay_cycle(1000);                         /// zpozdeni
    } while ((IFG1 & OFIFG) != 0);        /// test nabehnuti oscilatoru LFXT1 a LFXT2
  //  delay_cycle(0);                           /// zpozdeni
  
    BCSCTL2 = SELM_2 + SELS;              /// SMCLK = LFXT2 = 7.3728 MHz, MCLK = LFXT2 = 7.3728 MHz
  #elif defined MSP_261X
    BCSCTL1 &= ~XT2OFF;                       // Activate XT2 high freq xtal
    BCSCTL3 |= XT2S_2;                        // 3 – 16MHz crystal or resonator
  
    // Wait for xtal to stabilize
    _BIC_SR(OSCOFF);                      /// nulovani OscOFF, povoleni LFXT1
    do
    {
      IFG1 &= ~OFIFG;                           // Clear OSCFault flag
      delay_cycle(1000);
    } while ((IFG1 & OFIFG));                   // OSCFault flag still set?
    
    BCSCTL2 = SELM_2 | SELS | DIVM_0 | DIVS_1;   // MCLK = XT2CLK = 14.745 MHz, SMCLK = XT2CLK/2 = 7.3728 MHz
  #else
    #error "Can't initialize clock system"
  #endif

  /// INICIALIZACE vystupu hodin pro FPGA
  P5DIR |= SMCLK + ACLK;         // nastavi piny jako vystupni
  P5SEL |= SMCLK;                ///< pripoji SMCLK na pin

  return 0;
}

/**
 \brief Odpojeni SMCLK vedouciho do FPGA
**/
void smclk_stop(void) {
  FPGA_RESET_DIR |= FPGA_RESET_PIN;
  FPGA_RESET_OUT |= FPGA_RESET_PIN;
  P5DIR |= SMCLK;         // nastavi pin jako vystupni
  P5OUT &= ~SMCLK;        // na vystup 0
  P5SEL &= ~SMCLK;        // odpojit od hodinoveho signalu
}

/**
 \brief Nakonfiguruje FPGA pomoci dat ulozenych ve FLASH pameti
 \return 0 - chyba
        \n jinak - bez chyby
 **/
char fpga_configure_from_flash(void) {
  term_send_str("Programovani FPGA: ");

  // test, zda-li jsou ve FLASH platna data
  if (FLASH_isPageBlank(FLASH_FPGA_BIN_PAGE) != 0) {
    term_send_str_crlf("neuspesne - chybi data ve FLASH");
    return 0; // chyba
  }

  // programovani FPGA
  if (FLASH_ProgFPGA(FLASH_FPGA_BIN_ADDRESS, FPGA_config_length) == 0) {
    term_send_str_crlf("neuspesne - chybne FPGA");
    return 0;
  }

  term_send_str_crlf(" OK");

  term_send_str_crlf("Inicializace HW");
  
  FPGA_reset(); // reset FPGA
  fpga_initialized(); // uzivatelska inicializace

  return 1; // OK
}

/**
 \brief Nakonfiguruje FPGA pomoci dat poslanych pres terminal
 \return 0 - chyba
        \n jinak - bez chyby
 **/
char fpga_configure_from_mcu(void) {

  term_send_str("Programovani FPGA: ");

  // inicializace FPGA SpartanII pro programovani
  if (FPGAConfig_initialize() == 0) {
    term_send_str_crlf("neuspesne - FPGA se neporadilo inicializovat");
    return  0; // chybna inicializace
  }

  #ifdef XMODEMAPI
  XModemProgFPGA();
  #else
  SPI_WriteFile("FPGA configuration","*.bin", FPGA_config_length);
  #endif

  // ukonceni programovani FPGA SpartanIII a kontrola chyb
  if (FPGAConfig_finalize() == 0) {
    term_send_str_crlf("neuspesne - chybne FPGA");
    return 0;
  }

  term_send_str_crlf(" OK");

  term_send_str_crlf("Inicializace HW");
  FPGA_reset(); // reset FPGA
  fpga_initialized(); // uzivatelska inicializace

  return 1;
}

//////////////////////////////////////////////////////////////////////////////
// inicializace MCU

/**
 \brief Celková inicializace MCU
 \param progfpga - je-li 1, provede se naprogramovani FPGA z FLASH
 \return Vždy hodnota 0
 **/
int initialize_hardware_(char progfpga) {
   clock_system_init();   ///< Inicializace a nastaveni hodin pro MCU a FPGA
 
   timerb_init();  /// inicializace casovace, pro funkci delay_ms()

   FPGA_init();    ///<  Inicializace resetu pro FPGA

   WDG_init();             ///< init softwaroveho watchdogu
   _EINT();                 ///< povoleni preruseni

   terminal_init(); /// inicializace seriove linky

   SPI_Init();            // SPI inicializace

   term_send_str("Inicializace FPGA: ");
   if (FPGAConfig_detect_fpga(0x0140D093)) 
   {
       FPGA_config_length = 54908;
       term_send_str_crlf("XC3S50")
   } 
   else if (FPGAConfig_detect_fpga(0x0141C093)) 
   {
       FPGA_config_length = 212392;
       term_send_str_crlf("XC3S400")
   }
   else if (FPGAConfig_detect_fpga(0x01414093))
   {
       FPGA_config_length = 130952;
       term_send_str_crlf("XC3S200")
   }
   else
       term_send_str_crlf("nezname FPGA");

   if (flash_init(1)) //Inicializace FLASH, vypis informaci
   {

      FPGAConfig_init();
      ///Programovani FPGA
      if (progfpga) 
         fpga_configure_from_flash();
   }

   term_send_str(">");  
   return 0;
}
/**
 \brief Celková inicializace MCU
 \return Vždy hodnota 0
 **/
int initialize_hardware(void) {
  return initialize_hardware_(1);
}
