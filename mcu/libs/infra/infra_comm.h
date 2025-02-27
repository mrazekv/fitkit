/*******************************************************************************
   infra_comm.h: InfraRed communication library
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Michal Ruzek <xruzek01@stud.fit.vutbr.cz>
              
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

#ifndef _INFRA_COMM_H_
#define _INFRA_COMM_H_

#include <spi/spi_config.h>


//jmenna konstanta udavajici typ pouziteho protokolu - nemenit!
#define IR_RC5 0
#define IR_NEC_NORM 1    //normalni ramec
#define IR_NEC_REPEAT 2  //opakovaci ramec
#define IR_SIRC_12B 3
#define IR_SIRC_15B 4
#define IR_SIRC_20B 5

/**
 * bazova adresa SPI radice IR dekoderu
 * POZOR! Jedna pro vsechny typy dekoderu!!!
 * Vyber konkretniho dekoderu se realizuje OFFSETEM 
 */
#ifndef INFRA_BASE_ADDR
   #define INFRA_BASE_ADDR 0x90
#endif


/**
 * OFFSETY jednotlivych dekoderu
 * lze menit z main.c dle poctu dekoderu v FPGA
 */
#ifndef IR_RC5_OFFSET
   #define IR_RC5_OFFSET 0x00
#endif

#ifndef IR_NEC_N_OFFSET
   #define IR_NEC_N_OFFSET 0x01
#endif

#ifndef IR_NEC_R_OFFSET
   #define IR_NEC_R_OFFSET 0x02
#endif

#ifndef IR_SIRC_20_OFFSET
   #define IR_SIRC_20_OFFSET 0x03
#endif

#ifndef IR_SIRC_15_OFFSET
   #define IR_SIRC_15_OFFSET 0x04
#endif

#ifndef IR_SIRC_12_OFFSET
   #define IR_SIRC_12_OFFSET 0x05
#endif


// delka vyrovnavaciho RX bufferu 
#define IR_RX_BUF_LEN 5
// delka vyrovnavaciho TX bufferu
#define IR_TX_BUF_LEN 5


/**
 * Inicializace...
 */ 
void infra_init();


/**
 * Test na prazdnost vystupniho TX bufferu
 */
char infra_tx_empty();


/**
 * Test na to, ze se zrovna nevysila
 */
char infra_no_tx_transmit();


/**
 * Test na prazdnost vstupniho RX bufferu
 */
char infra_rx_empty();


/**
 * Obsluha preruseni pri primu infracervenych dat
 * - ulozeni dat do bufferu vcetne typu protokolu
 */
void infra_rx_interrupt_handler(unsigned char proto_type);


/**
 * Obsluha preruseni po odesilani infracervenych dat
 * - pokud buffer neni prazdny, "vyjmi" data a odesli 
 */
void infra_tx_interrupt_handler();


/**
 * Vyslani 4B infracervenych dat
 * Pokud je zarizeni volne (a buffer prazdny),
 * jsou vyslany hned, jinak ulozeny do bufferu. 
 */
void infra_send(unsigned char proto, unsigned char *data);


/**
 * Prijem infracervenych dat
 * precte 4B dat z bufferu, pokud neni prazdny, a vrati 1
 * jinak vraci 0 
 */
char infra_read(unsigned char * proto, unsigned char *data);

#endif  // _INFRA_COMM_H_



