/*******************************************************************************
   define.h:
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
     Definice preprocesoru
*******************************************************************************/

#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "arch_specific.h"

#define SPIMAX
#define IDKIT "FITkit "     // Identifikace FITkitu
#define IDREV "$Rev: 163 $" // Revize z SVN

#if defined MSP_16X
  #define IDFLG "1.x "
#elif defined MSP_261X
  #define IDFLG "2.x "
#else
  #error "Unknown CPU type"
#endif

/*
 * #ifdef SPIMAX
  #define IDFLG "SPImax "   // Nastavena max. frekvence SPI
#else 
  #define IDFLG ""
#endif
*/

#define ID_KIT_STRING (IDKIT IDFLG IDREV)     ///< Identifikacni string FITKitu


/******************************************************************************/
/* Seriova linka UART */
#define B_921600        3               /* baud rychlosti */
#define B_460800        2
#define B_230400        1
#define B_115200        0
#define B_57600         57600
#define B_38400         38400
#define B_19200         19200
#define B_9600          9600
#define B_4800          4800
#define B_2400          2400
#define B_1200          1200

/* Defaultni rychlost seriove linky po inicializaci MCU */
#define B_default       B_460800


/******************************************************************************/
/* Seriova Flash pamet */

#define BUFFER_SIZE       264       ///< velikost stranky buffru [byte]
#define PAGE_SIZE         264       ///< velikost stranky [byte]
#define CNT_PAGES_IN_BLOCK 8        ///< pocet stranek v bloku
#define BLOCK_SIZE   CNT_PAGES_IN_BLOCK * PAGE_SIZE ///< velikost bloku [byte]

/* adresovy prostor Flash pameti */
/*   - nulta stranka nevyuzita - informace pro FKFlash (checksum HEX, BIN) */
/*   - stranka 8 - 215 data pro FPGA (XC3S50 - max 54 908B) */
/*   - stranka 8 - 815 data pro FPGA (XC3S400 - max 212 392B), zarovnano na nasobky 8 */

#define FLASH_FPGA_BIN_PAGE 8 ///< pocatecni adresa (cislo stranky) FPGA dat
#define FLASH_FPGA_BIN_ADDRESS (FLASH_FPGA_BIN_PAGE*PAGE_SIZE)
#define FPGA_BIN_PAGES (((unsigned long)(FPGA_config_length) + PAGE_SIZE-1) / PAGE_SIZE)
#define FLASH_USER_DATA_PAGE (FLASH_FPGA_BIN_PAGE + (((FPGA_BIN_PAGES) >> 3) << 3)) //zarovnano na velikost bloku (8 stranek)
#define FLASH_USER_DATA_ADDRESS ((FLASH_USER_DATA_PAGE)*PAGE_SIZE)

/******************************************************************************/
/* Zvlastni znaky */
#define BS              0x08            ///< BackSpace
#define CR              0x0D            ///< CR
#define LF              0x0A            ///< LF
#define ESC             0x1B            ///< ESC
#define ERS_CHAR        0xFF            ///< znak, kterym se maze flash


/******************************************************************************/
/* Hodinove vystupy */

/* ACLK vystup - 32.768kHz */
#define ACLK            BIT6            ///< ACLK pin

/* SMCLK vystup - 7.3728MHz */
#define SMCLK           BIT5            ///< SMCLK pin


/******************************************************************************/
/* LED vystupy */
#define LED0            BIT0            ///< LED D5
#define LED1            BIT7            ///< LED D6

#define set_led_d5(on) {\
  P1DIR |= LED0; /* nastavit bit jako vystup */ \
  P1OUT = (on) ? (P1OUT & ~LED0) : (P1OUT | LED0); /* 0 - sviti, 1 - nesviti */ \
  }

#define set_led_d6(on) {\
  P5DIR |= LED1; /* nastavit bit jako vystup */ \
  P5OUT = (on) ? (P5OUT & ~LED1) : (P5OUT | LED1); /* 0 - sviti, 1 - nesviti */ \
  }

#define flip_led_d5() {P1OUT ^= LED0;}
#define flip_led_d6() {P5OUT ^= LED1;}


/******************************************************************************/
/* Adresovy prostor MCU */
#define MCU_RAM_START   0x0200          ///< MCU
#define MCU_RAM_END     0x09FF


/******************************************************************************/
/* Watchdog */

/* Povoleni/zakaz watchdogu */
//#define WDG_ENABLE


/* pokud je definovan WDG */
#ifdef  WDG_ENABLE                      // watchdog

  #define WDG_init() \
    { WDTCTL = WDTPW + WDTCNTCL + WDTIS0;    /* SW watchdog na SMCLK, (8.9ms interval) */ \
      WDG_reset(); \
    }

  /* musi se aktivovat casteji nez 8.9ms, jinak dojde k resetu MCU */
  #define WDG_reset() \
    { WDTCTL = WDTPW + WDTCNTCL + WDTIS0;  \
      if ((IFG1 & OFIFG) != 0) { _DINT(); WDTCTL = 0; while (1) {}; } \
    }

#else

  #define WDG_init()
  #define WDG_reset()

#endif /* WDG_ENABLE */

#define WDG_stop()      WDTCTL = WDTPW + WDTHOLD;             ///< stop WDT


/******************************************************************************/
/* Terminal */

/* Maximalni delka retezce z terminalu */
#define  MAX_COMMAND_LEN   24

/* Makra strcmpN na porovnani retezcu delky N */
#define  strcmp1(a, b)  (*(a) == *(b))
#define  strcmp2(a, b)  (strcmp1(a, b) && strcmp1(a+1, b+1))
#define  strcmp3(a, b)  (strcmp1(a, b) && strcmp2(a+1, b+1))
#define  strcmp4(a, b)  (strcmp1(a, b) && strcmp3(a+1, b+1))
#define  strcmp5(a, b)  (strcmp1(a, b) && strcmp4(a+1, b+1))
#define  strcmp6(a, b)  (strcmp1(a, b) && strcmp5(a+1, b+1))
#define  strcmp7(a, b)  (strcmp1(a, b) && strcmp6(a+1, b+1))
#define  strcmp8(a, b)  (strcmp1(a, b) && strcmp7(a+1, b+1))
#define  strcmp9(a, b)  (strcmp1(a, b) && strcmp8(a+1, b+1))
#define  strcmp10(a, b) (strcmp1(a, b) && strcmp9(a+1, b+1))
#define  strcmp11(a, b) (strcmp1(a, b) && strcmp10(a+1, b+1))
#define  strcmp12(a, b) (strcmp1(a, b) && strcmp11(a+1, b+1))
#define  strcmp13(a, b) (strcmp1(a, b) && strcmp12(a+1, b+1))
#define  strcmp14(a, b) (strcmp1(a, b) && strcmp13(a+1, b+1))
#define  strcmp15(a, b) (strcmp1(a, b) && strcmp14(a+1, b+1))
#define  strcmp16(a, b) (strcmp1(a, b) && strcmp15(a+1, b+1))
#define  strcmp17(a, b) (strcmp1(a, b) && strcmp16(a+1, b+1))
#define  strcmp18(a, b) (strcmp1(a, b) && strcmp17(a+1, b+1))
#define  strcmp19(a, b) (strcmp1(a, b) && strcmp18(a+1, b+1))
#define  strcmp20(a, b) (strcmp1(a, b) && strcmp19(a+1, b+1))
#define  strcmp21(a, b) (strcmp1(a, b) && strcmp20(a+1, b+1))
#define  strcmp22(a, b) (strcmp1(a, b) && strcmp21(a+1, b+1))
#define  strcmp23(a, b) (strcmp1(a, b) && strcmp22(a+1, b+1))
#define  strcmp24(a, b) (strcmp1(a, b) && strcmp23(a+1, b+1))
#define  strcmp25(a, b) (strcmp1(a, b) && strcmp24(a+1, b+1))
#define  strcmp26(a, b) (strcmp1(a, b) && strcmp25(a+1, b+1))
#define  strcmp28(a, b) (strcmp1(a, b) && strcmp27(a+1, b+1))
#define  strcmp29(a, b) (strcmp1(a, b) && strcmp28(a+1, b+1))
#define  strcmp30(a, b) (strcmp1(a, b) && strcmp29(a+1, b+1))
#define  strcmp31(a, b) (strcmp1(a, b) && strcmp30(a+1, b+1))
#define  strcmp32(a, b) (strcmp1(a, b) && strcmp31(a+1, b+1))


/******************************************************************************/
/* prikazy terminalu */
#define PROG0                       10
#define CMD_MCU_PROG_FPGAI     PROG0+1  ///< programování FPGA SpartanIII z PC

#define RST0                        20
#define CMD_MCU_RESET           RST0+1  ///< reset MCU
#define CMD_FPGA_RESET          RST0+2  ///< reset FPGA
#define CMD_CLEARSCREEN         RST0+3  ///< clear screen (terminal reset)
#define CMD_SMCLK_STOP          RST0+4  ///< stop SMCLK

#define FL0                         30
#define CMD_FLASH_FST            FL0
#define CMD_MCU_FLASHW_FPGA      FL0     ///< pøik. pro uložení desig. FPGA z PC do flash
#define CMD_MCU_FLASHW_X         FL0+1   ///< zápis do flash na adresu (xmodem)
#define CMD_MCU_FLASHW_C         FL0+2   ///< zápis jednoho bytu do flash na adresu

#define CMD_MCU_FLASHP_FPGA      FL0+3   ///< programování FPGA SpartanIII z Flash

#define CMD_MCU_FLASHR_ADR       FL0+4   ///< výpis 64 Bytu flash pamìti z adresy
#define CMD_MCU_FLASHR_PAGE      FL0+5   ///< výpis stránky pamìti flash
#define CMD_MCU_FLASHR_BLOCK     FL0+6   ///< výpis bloku pamìti flash
#define CMD_MCU_FLASHR_STATUS    FL0+7   ///< výpis status registru pamìti flash

#define CMD_MCU_FLASHD_BYTE      FL0+10  ///< smazání jednoho bytu ve flash pamìti
#define CMD_MCU_FLASHD_PAGE      FL0+11  ///< smazání stránky pamìti flash
#define CMD_MCU_FLASHD_BLOCK     FL0+12  ///< smazání bloku pamìti flash
#define CMD_MCU_FLASHD_ALL       FL0+13  ///< smazání celé flash pamìti flash

#define CMD_MCU_FLASHD_FPGA      FL0+20  ///< smazání designu FPGA z flash
#define CMD_FLASH_LAST           CMD_MCU_FLASHD_FPGA

// prikazy pro FPGA
/*
#define FP0                         60
#define CMD_FPGA_READ_B          FP0+0
#define CMD_FPGA_READ_W          FP0+1
#define CMD_FPGA_WRITE_B         FP0+2
#define CMD_FPGA_WRITE_W         FP0+3
*/

#define CMD_MCU_RAM_DUMP         249     ///< dump RAM/ROM pameti MCU
#define USER_COMMAND             251     ///< uživatelský pøíkaz
#define CMD_NULL                 252     ///< prázdný pøíkaz
#define CMD_HELP                 253     ///< pøíkaz helpu
#define CMD_END                  254     ///< neznámý pøíkaz
#define CMD_UNKNOWN              255     ///< neznámý pøíkaz
/******************************************************************************/


#endif /* _DEFINE_H_ */
