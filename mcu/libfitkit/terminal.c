/*******************************************************************************
   terminal.c:
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
     Realizace TERMINAL modu.
*******************************************************************************/

/* Provedena revize */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arch_specific.h"
#include "define.h"
#include "timer_b.h"
#include "globfun.h"
#include "uart.h"
#include "cmd.h"

//#include "xmodem.h"
#include "filetransfer.h"

#include "flash.h"
#include "flash_fpga.h"
#include "terminal.h"
#include "fitkitlib.h"
#include "fpga.h"
#include "spi.h"

unsigned char Cmd;      ///< aktuální pøíkaz
unsigned char Cmd_Old;  ///< starý pøíkaz

/**
 \brief Inicializace terminálu - UART0.
 **/
void terminal_init(void) {
  term_cmd_clear();

  serial_init();                // inicializace RS232 0

  /// vymazani obrazovky terminalu
  term_send_esc_seq("[2J");

  //RS232_SendESCString("[1;34;47m");
  term_send_esc_seq("[4m");
  term_send_str_crlf(ID_KIT_STRING);
  //RS232_SendESCString("[0;30;47m");INIT_HW_MCU
  term_send_esc_seq("[24m");
  term_send_crlf();
}

/**
 \brief Obsluha terminálu s call backem.
 **/
void terminal_idle_cb(void (*before_prog_cb)(void)) {
  char int_swp;
  char cmdustr[CMDStrLen+1];
  unsigned ij;

  // osetreni terminalu
  WDG_reset();
  if (term_cmd_read() != 0) {  // cteni prikazu

    strupper(cmdustr, CMDStr);
    Cmd = term_cmd_decode(cmdustr);  // dekodovani prikazu

    if (Cmd == CMD_NULL)
      Cmd = Cmd_Old;                // vyvolani predchoziho prikazu

    Cmd_Old = Cmd;                  // uchovani prikazu

    switch (Cmd) {
      case CMD_NULL:                // NEBYLO NIC ZADANO
      break;
      /*
      case CMD_UNKNOWN:            // NEZNAMY PRIKAZ
        RS232_SendString(" Neznamy prikaz");
        Cmd_Old = CMD_NULL;
      break;
      */
      case CMD_HELP:               // HELP
        term_send_str_crlf("Napoveda:");
        term_send_crlf();
        term_send_str_crlf(" PROG FPGA........programovani z pc (XModem)");
        term_send_str_crlf(" PROG FPGA FLASH..programovani FPGA z flash");
        term_send_crlf();
        term_send_str_crlf(" RESET MCU....software reset MCU");
        term_send_str_crlf(" RESET FPGA...software reset FPGA");
        term_send_str_crlf(" CLS..........reset terminalu");
        term_send_crlf();
/*        
        term_send_str_crlf(" FLASH W C chr adr...zapis znaku 'chr' dat do flash na adresu 'adr'");
        term_send_str_crlf(" FLASH W X adr...zapis dat do flash na adresu 'adr' (XModem)");
*/        
        term_send_str_crlf(" FLASH W FPGA.....zapis FPGA dat z PC do flash (XModem)");
        term_send_str_crlf(" FLASH W X........zapis dat do flash na adresu 'adr' (XModem)");
        term_send_str_crlf(" FLASH R A adr....vypis 64Bytu flash z adresy 'adr'");
        term_send_str_crlf(" FLASH R P page...vypis stranky c. 'page' flash");
        term_send_str_crlf(" FLASH R B block..vypis bloku c. 'block' flash");
        term_send_str_crlf(" FLASH R S........vypis status registru flash");
        term_send_str_crlf(" FLASH E FPGA.....odstraneni FPGA dat z flash");
/*
        term_send_str_crlf(" FLASH E A adr....smazani bytu ve flash na adrese 'adr'");
        term_send_str_crlf(" FLASH E P page...smazani stranky 'page' flash");
        term_send_str_crlf(" FLASH E B block..smazani bloku 'block' flash");
        term_send_str_crlf(" FLASH E ALL......smazani cele flash"); */
        term_send_crlf();
        term_send_str_crlf(" RAM D....dump RAM Memory");
        term_send_crlf();
        /*
        term_send_str_crlf(" FPGA R B adr....vypis bytu z fpga z adresy 'adr'");
        term_send_str_crlf(" FPGA R W adr....vypis wordu z fpga z adresy 'adr'");
        term_send_str_crlf(" FPGA W B adr byte....zapis bytu do fpga na adresu 'adr'");
        term_send_str_crlf(" FPGA W W adr world....zapis wordu do fpga na adresu 'adr'");
        */
        term_send_str_crlf("Uzivatelska napoveda:");
        print_user_help();
      break;
      case CMD_MCU_RAM_DUMP: {
        unsigned char *ptr = (unsigned char *)MCU_RAM_START - 8;

        term_send_hex((unsigned int)ptr-8);
        term_send_str(" : ");
        for (; ((unsigned int)ptr % 16) == 0; ptr++) {

          WDG_reset();

          term_send_char_hex(0);
        }
        for (; ptr <= (unsigned char *)MCU_RAM_END; ptr++) {

          WDG_reset();

          term_send_char_hex(*ptr);
          term_send_char(' ');
          if (((unsigned int)ptr % 16) == 0) {
            term_send_crlf();
            term_send_hex((unsigned int)ptr);
            term_send_str(" : ");
          }
        }
      }
      break;
      case CMD_MCU_PROG_FPGAI:      // NAPROGRAMOVANI FPGA  z PC
        if (before_prog_cb != NULL)
          before_prog_cb(); //callback
        int_swp = P1IE; //  = MCU_INTERRUP_PINS;  
        P1IE = 0;    // disable interrupt
        fpga_configure_from_mcu();
        P1IE = int_swp;  // enable interrupt
      break;
      case CMD_MCU_RESET:           // RESET MCU
        _DINT();                               // zakaz preruseni
        /*
        WDTCTL = WDTPW + WDTCNTCL + WDTIS0;    // povoleni software watchdog
        for (;;) {
          tii++;                    // nekonecny cyklus, dokud watchdog nezresetuje MCU
        }
        */
        /* vyvolany reset, pokud se zapisuje do WDTCTL bez WDTPW,
           i kdyz neni povoleny watchdog */
        WDTCTL = WDTCNTCL + WDTIS0;
      break;                        // RESET FPGA
      case CMD_FPGA_RESET:
        FPGA_reset();
      break;
      case CMD_CLEARSCREEN:         // CLS (terminal reset)
        term_send_esc_seq("[2J");
      break;
      case CMD_MCU_FLASHW_FPGA:     // ZAPSANI FPGA DAT SpartanIII do Flash
        #ifdef XMODEMAPI
        FLASH_WriteFile(FLASH_FPGA_BIN_PAGE);
        #else
        FLASH_WriteFile(FLASH_FPGA_BIN_PAGE,"FPGA configuration","*.bin", FPGA_BIN_PAGES);
        #endif
      break;
/*      case CMD_MCU_FLASHW_C:        // ZAPSANI BYTE DO FLASH
        FLASH_WriteByte(CMD_Long, CMD_Char);
      break;
*/      
      case CMD_MCU_FLASHW_X:        // ZAPSANI DAT Z PC DO FLASH
        #ifdef XMODEMAPI
        FLASH_WriteFile(FLASH_USER_DATA_PAGE);
        #else
        FLASH_WriteFile(FLASH_USER_DATA_PAGE,0,0,0);
        #endif
      break;
      case CMD_MCU_FLASHP_FPGA:     // PROGRAMOVANI FPGA SpartanIII z FLASH
        //char int_swp;
        if (before_prog_cb != NULL)
          before_prog_cb(); //callback
        int_swp = P1IE;//  = MCU_INTERRUP_PINS;      //
        P1IE = 0;   // disable interrupt
        fpga_configure_from_flash();
        P1IE = int_swp;
       break;
      case CMD_MCU_FLASHR_ADR:      // VYPIS casti flash pameti
      case CMD_MCU_FLASHR_PAGE:
      case CMD_MCU_FLASHR_BLOCK:
        switch (Cmd) {
          case CMD_MCU_FLASHR_ADR:
            FLASH_Dump_Adr(CMD_Long, 64);
          break;
          case CMD_MCU_FLASHR_PAGE:
            FLASH_Dump_Page(CMD_Long);
          break;
          case CMD_MCU_FLASHR_BLOCK:
            FLASH_Dump_Block(CMD_Long);
          break;
          default:
          break;
        }

        // zvetsim automaticky CMD_Long pro opakovani prikazu
        if ((Cmd_Old == CMD_MCU_FLASHR_PAGE) || (Cmd_Old == CMD_MCU_FLASHR_BLOCK))
          CMD_Long++;
        else if (Cmd_Old == CMD_MCU_FLASHR_ADR)
          CMD_Long += 64;
      break;

      case CMD_MCU_FLASHD_FPGA:     // ODSTRANENI DAT FPGA z FLASH
        term_send_str("Mazani dat FPGA ve Flash");
        FLASH_WriteByte(128,0);
        for (ij = 0; ij < FPGA_BIN_PAGES; ij++) {
          FLASH_ErasePage(FLASH_FPGA_BIN_PAGE + ij);
          WDG_reset();
        }
        term_send_crlf();
      break;
/*
      case CMD_MCU_FLASHD_BYTE:     // SMAZANI jednoho bytu flash pameti
        term_send_str("Mazani bytu ve flash na adrese: 0x");
        term_send_hex(CMD_Long);
        FLASH_EraseByte(CMD_Long);
        term_send_crlf();
      break;

      case CMD_MCU_FLASHD_PAGE:     // SMAZANI jedne stranky flash pameti
      case CMD_MCU_FLASHD_BLOCK:
      case CMD_MCU_FLASHD_ALL:
        // smazani casti flash
        switch (Cmd) {
          case CMD_MCU_FLASHD_PAGE:
            term_send_str("Mazani flash na strance: ");
            term_send_num(CMD_Long);
            FLASH_ErasePage(CMD_Long);
            term_send_crlf();
          break;
          case CMD_MCU_FLASHD_BLOCK:
            term_send_str("Mazani bloku flash: ");
            term_send_num(CMD_Long);
            FLASH_EraseBlock(CMD_Long);
            term_send_crlf();
          break;
          case CMD_MCU_FLASHD_ALL:
            term_send_str_crlf("Mazani cele flash.");
            FLASH_EraseAll();
            term_send_crlf();
          break;
          default :
          break;
        }
      break;
*/      
      case CMD_MCU_FLASHR_STATUS:   // VYPIS status registru flash pameti
        FLASH_print_status();
      break;

      case CMD_SMCLK_STOP:
        smclk_stop();
      break;
      
      default:
        if (decode_user_cmd(cmdustr, CMDStr) != USER_COMMAND) {
          term_send_str("Neznamy prikaz: ");
          term_send_char('\'');
          term_send_str(CMDStr);
          term_send_char('\'');
        }
      break;
    } // switch
    term_send_crlf();
    term_send_str(">");
    term_cmd_clear();
  }  // if
}

/**
 \brief Obsluha terminálu.
 **/
void terminal_idle(void) {
  terminal_idle_cb(NULL);
}

