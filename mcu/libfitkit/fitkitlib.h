/*******************************************************************************
   fitkitlib.h:
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

*******************************************************************************/


#ifndef _FITKITLIB_H_
#define _FITKITLIB_H_

#include "arch_specific.h"
#include "cmd.h"
#include "define.h"
#include "filetransfer.h"
#include "flash.h"
#include "flash_fpga.h"
#include "fpga.h"
#include "globfun.h"
#include "isr_compat.h"
#include "spi.h"
#include "terminal.h"
#include "timer_b.h"
#include "uart.h"

/**
 \brief Maximální délka konfiguraèní informace vèetnì overheadu
 **/
extern unsigned long FPGA_config_length;

/**
 \brief Celková inicializace MCU
 \return Vždy hodnota 0
 **/
int initialize_hardware(void);

/**
 \brief Celková inicializace MCU
 \param progfpga - je-li 1, provede se naprogramovani FPGA z FLASH
 \return Vždy hodnota 0
 **/
int initialize_hardware_(char progfpga);

/**
 \brief  Inicializace HW komponent po naprogramování FPGA, tato funkce musí být definována
  v uživatelském programu.
 **/
void fpga_initialized();

/**
 \brief Nakonfiguruje FPGA pomoci dat ulozenych ve FLASH pameti
 \return 0 - chyba
        \n jinak - bez chyby
 **/
char fpga_configure_from_flash(void);

/**
 \brief Nakonfiguruje FPGA pomoci dat poslanych pres terminal
 \return 0 - chyba
        \n jinak - bez chyby
 **/
char fpga_configure_from_mcu(void);

/**
  \brief  Jednoduchá funkce realizující zpoždení.
  \param  delay - poèet cyklu, které se èeka
 **/
void delay_cycle(unsigned int delay);

/**
 \brief Odpojeni SMCLK vedouciho do FPGA
**/
void smclk_stop(void);

#endif   /* _FITKITLIB_H_ */
