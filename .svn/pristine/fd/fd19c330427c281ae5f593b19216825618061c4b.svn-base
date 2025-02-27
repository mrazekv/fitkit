/*******************************************************************************
   fpga_rs_232.h:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jan Markovic <xmarko04@stud.fit.vutbr.cz>
              Jakub Vavra  <xvavra07@stud.fit.vutbr.cz>
              
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

#ifndef _FPGA_RS_232_H_
#define _FPGA_RS_232_H_

#include <spi/spi_config.h>

#ifndef BASE_ADDR_RS_232_USB
   #define BASE_ADDR_RS_232_USB 0x04 ///< Bázová adresa SPI radice RS_232 (USB)
#endif

#ifndef BASE_ADDR_RS_232_CAN9
   #define BASE_ADDR_RS_232_CAN9 0x05 ///< Bázová adresa SPI radice RS_232 (CAN9_M)
#endif

/// delka vyrovnavaciho Rx buffru pro FPGA
#define FPGA_LengthBuffers_Rx 50
/// delka vyrovnavaciho Tx buffru pro FPGA
#define FPGA_LengthBuffers_Tx 50

/**
 * Poslani 8.bitovych dat na rs_232 (data + adresa)
 **/
#define send_bit_fpga_rs_232(data, rs_232_id) \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_WRITE,rs_232_id,data)

/**
 * precteni 8.bitovych dat z rs_232 (adresa)
 **/
#define read_bit_fpga_rs_232(rs_232_id) \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_READ,rs_232_id,0)

/**
 * Pocatecna inicializace rs232 pro FPGA
 */
void FPGA_RS232_Init();


/**
 * Pocet znaku ve vstupnim bafru z rs232 z FPGA
 * aneb test ci je bufer prazdny
 */
char FPGA_rs232_UseBuf();

/**
 * Pocet znaku ve vytupnim bafru do rs232 na FPGA
 * aneb test na prazdnost bufru
 */
char FPGA_rs232_TX_UseBuf();



/**
 * Cteni znaku z bufru seriove linky FPGA.
 * - vraci jeden znak (8 bitu)
 **/
unsigned char FPGA_RS232_ReceiveChar(void);

 
 /**
 * Zapis znaku do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendChar(unsigned char schar);


 /**
 * Zapis retezce do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendString(unsigned char *ptrStr);

/**
 * Zapis retezce a CRLF do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendStringCRLF(unsigned char *ptrStr);


/**
 * Obsluha preruseni pri primu dat z rs232 v FPGA
 * precteni znaku z rs232 v FPGA a ulozeni do bufferu
 */
void fpga_interrupt_rs232_rxd(void);

/**
 * Obsluha preruseni pri odesilani dat z rs232 v FPGA
 * zapsani znaku na rs232 v FPGA a smazani z bufferu
 */
void fpga_interrupt_rs232_txd(void);

/**
 * Zaslani CRLF na rs232 na fpga
 */
void FPGA_SendCRLF();

#endif  // _FPGA_RS_232_H_



