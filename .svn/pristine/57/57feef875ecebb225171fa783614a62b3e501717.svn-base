/*******************************************************************************
   infra_comm.c: InfraRed communication library
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

#include <msp430x16x.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <fitkitlib.h>

#include "infra_comm.h"

//typova struktura bufferu
typedef struct
{
  unsigned char proto;
  unsigned char data[3];
} TBuffer;


//pole pro preklad typu protokolu na jeho offset
static const unsigned char proto_offsets[] = {IR_RC5_OFFSET, IR_NEC_N_OFFSET, IR_NEC_R_OFFSET,
                                              IR_SIRC_12_OFFSET, IR_SIRC_15_OFFSET, IR_SIRC_20_OFFSET};

//buffer pro vysilani
volatile TBuffer infra_tx_buf[IR_TX_BUF_LEN];
//aktualni index dat ctenych z bufferu
volatile unsigned char infra_txRindex;
//aktualni index dat zapisovanych do bufferu
volatile unsigned char infra_txWindex;
// pokud 1 -> zarizeni volne a je mozne hned vysilat
volatile char infra_tx_free;


//buffer pro prijem
volatile TBuffer infra_rx_buf[IR_RX_BUF_LEN];
//aktualni index dat ctenych z bufferu
volatile unsigned char infra_rxRindex;
//aktualni index dat zapisovanych do bufferu
volatile unsigned char infra_rxWindex;


/**
 * Inicializace indexu
 * indexy se rovnaji -> buffer prazdny
 */ 
void infra_init()
{
   infra_txRindex = 0;
   infra_txWindex = 0;
   infra_rxRindex = 0;
   infra_rxWindex = 0;  
   infra_tx_free = 1; 
}


/**
 * Test na prazdnost vystupniho TX bufferu
 */
char infra_tx_empty()
{
    return (infra_txRindex == infra_txWindex);
}


/**
 * Test na to, ze se zrovna nevysila
 */
char infra_no_tx_transmit()
{
  return infra_tx_free;
}


/**
 * Test na prazdnost vstupniho RX bufferu
 */
char infra_rx_empty()
{
    return (infra_rxRindex == infra_rxWindex);
}


/**
 * Obsluha preruseni pri primu infracervenych dat
 * - ulozeni dat do bufferu vcetne typu protokolu
 */
void infra_rx_interrupt_handler(unsigned char proto_type)
{
   WDG_reset(); //reset SW watchdogu
   //pokud buffer neni plny, vloz data
   if ((infra_rxWindex+1) % IR_RX_BUF_LEN != infra_rxRindex)
   {
      infra_rx_buf[infra_rxWindex].proto = proto_type;
      FPGA_SPI_RW_A8_DN(SPI_FPGA_ENABLE_READ, INFRA_BASE_ADDR+proto_offsets[proto_type], (unsigned char *)infra_rx_buf[infra_rxWindex].data, 3);       
      infra_rxWindex = (infra_rxWindex+1) % IR_RX_BUF_LEN;   
   }
}



/**
 * Obsluha preruseni pri odesilani infracervenych dat
 * - pokud buffer neni prazdny, "vyjmi" data a odesli 
 */
void infra_tx_interrupt_handler()
{
   WDG_reset(); //reset SW watchdogu
   infra_tx_free = 1;
   if (!infra_tx_empty())
   {
      infra_tx_free = 0; 
      FPGA_SPI_RW_A8_DN(SPI_FPGA_ENABLE_WRITE, INFRA_BASE_ADDR+proto_offsets[infra_tx_buf[infra_txRindex].proto], (unsigned char *)infra_tx_buf[infra_txRindex].data, 3);       
      infra_txRindex = (infra_txRindex+1) % IR_TX_BUF_LEN;   
   }
}



/**
 * Vyslani infracervenych dat
 * Pokud je zarizeni volne (a buffer prazdny),
 * jsou vyslany hned, jinak ulozeny do bufferu. 
 */
void infra_send(unsigned char proto, unsigned char *data)
{
  //zamaskovani nadbytecnych bitu v adrese:
   if (proto==IR_SIRC_15B)
      data[0]=0;       //pole extended musi byt zde nulove
   else if (proto==IR_SIRC_20B)
      data[1] &= 0x1F; //potreba pouze dolnich 5 bitu adresy
   //------      
   if (infra_tx_free && infra_tx_empty())
   {
      infra_tx_free=0;
      FPGA_SPI_RW_A8_DN(SPI_FPGA_ENABLE_WRITE, INFRA_BASE_ADDR+proto_offsets[proto], data, 3); 
      return;
   }
   //pokud buffer neni plny, vloz data
   if ((infra_txWindex+1) % IR_TX_BUF_LEN != infra_txRindex)
   {
      infra_tx_buf[infra_txWindex].proto = proto;
      memcpy((unsigned char *)infra_tx_buf[infra_txWindex].data, data, 3);  
      infra_txWindex = (infra_txWindex+1) % IR_TX_BUF_LEN;   
   }  
}



/**
 * Prijem infracervenych dat
 * precte 3B dat z bufferu, pokud neni prazdny, a vrati 1
 * jinak vraci 0 
 */
char infra_read(unsigned char * proto, unsigned char *data)
{
   if (!infra_rx_empty())
   {
      WDG_reset(); //reset SW watchdogu
      (*proto) = infra_rx_buf[infra_rxRindex].proto;
      memcpy(data, (unsigned char *)infra_rx_buf[infra_rxRindex].data, 3);       
      infra_rxRindex = (infra_rxRindex+1) % IR_RX_BUF_LEN;  
      return 1;
   }
   return 0;
}
