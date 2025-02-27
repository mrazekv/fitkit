/*******************************************************************************
   flash_fpga.h:
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


#ifndef _FLASH_FPGA_H_
#define _FLASH_FPGA_H_

// SPI rozhrani a SpartanIII
#define FCSn    BIT0        ///< chip select Flash pameti, aktivni v 0
#define FSI     BIT1        ///< datovy vstup Flash pameti
#define FSO     BIT2        ///< datovy vystup Flash pameti
#define FSCK    BIT3        ///< hodinovy vstup Flash pameti
#define DONE    BIT7        ///< DONE vstup do FPGA
#define INITn   BIT4        ///< INIT vstup/vystup FPGA, aktivni v 0
#define PROGn   BIT6        ///< PROG vstup do FPGA, aktivni v 0

// port SPI
#define FLPORTDIR   P5DIR       ///< direct port
#define FLPORTIN    P5IN        ///< vstupni port
#define FLPORTOUT   P5OUT       ///< vystupni port
#define FLPORTSEL   P5SEL       ///< selekcni port

/**
 \brief Inicializace konfiguraèního rozhraní FPGA.
 **/
void FPGAConfig_init(void);

/**
 \brief Inicializace programování FPGA SpartanIII, data z MCU nebo FLASH.
 \return 0 - inicializace skonèila chybou
        \n 1 - inicializace v poøádku
 **/
unsigned char FPGAConfig_initialize(void);

/**
 \brief Konec programování FPGA SpartanIII, kontrola naprogramování.
 \return 0 - programování skonèilo chybou
        \n 1 - programování v poøádku
 **/
unsigned char FPGAConfig_finalize(void);

/**
 \brief Detekce typu FPGA.
 \return 0 - FPGA neodpovida zvolenemu IDcode
        \n 1 - FPGA odpovida zvolenemu IDcode
 **/
unsigned char FPGAConfig_detect_fpga(unsigned long idcode);

#define FPGAConfig_DONE() (P4IN & DONE)

#define FPGAConfig_INITn() (P5IN & INITn)

#endif        /* _FLASH_FPGA_H_ */
