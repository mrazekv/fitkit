/*******************************************************************************
   enc28j60_eth.c: Linková vrstva modelu TCP/IP
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Martin Musil <xmusil34 AT fit.vutbr.cz>

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
#include "enc28j60_ip.h"
#include "enc28j60_arp.h"
#include "enc28j60_eth.h"
#include "enc28j60_spi.h"

#include "enc28j60_string.h"

extern unsigned int tx_end_ptr;
extern char enc_initialized;
//defaultni MAC adresa
unsigned char local_mac[MAC_LEN] = {0x00,0x23,0x8B,0x6B,0x38,0x64};
unsigned char broadcast_mac[MAC_LEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


/**
 \brief Obsluha øadièe ENC28J60. Musí být volána v nekoneèné smyèce.
 **/
void ENC28J60_idle(){
	
	if(read_reg8(EIR,NO_DUMMY) & EIR_PKTIF){		//paket v rx bufferu
		eth_recv();
	}	
	
return;
}


/**
 \brief Funkce volaná pøi pøíchodu nového rámce
 **/
void eth_recv(){
	
	struct eth_h eth_header;
				
	if(rx_init() < 0){
		return;
	}

	rx_read(&eth_header, ETH_HEADER_LEN);
	
	switch(eth_header.type_len){
		
		case HTONS(ETH_PROTO_IP):
			ip_recv();
			break;
			
		case HTONS(ETH_PROTO_ARP):
			arp_recv();
			break;
	}
	
	rx_free();

return;
}

/**
 \brief Odešle Ethernetový rámec
 **/
char eth_send(unsigned char* dest_addr,unsigned int type){
	
	struct eth_h eth_header;
	
	enc_memcpy(&eth_header.dest_addr, dest_addr, MAC_LEN);
	enc_memcpy(&eth_header.src_addr, &local_mac, MAC_LEN);
	eth_header.type_len = HTONS(type);
	
	tx_seek(CTRL);
	tx_putc(CTRL_BYTE);
	tx_write(&eth_header,ETH_HEADER_LEN);
	
return tx_send();	
}

/**
 \brief Nastaví lokální MAC adresu (musí být volána pøed inicializací øadièe - pøed voláním ENC28J60_init() ),
 		 jinak se použije dafaultní MAC 01:24:8C:6B:38:64
 **/
inline char set_mac(unsigned char* mac_ptr){
	
	if(enc_initialized == 0){
		enc_memcpy(&local_mac,mac_ptr, MAC_LEN);
		return 1;
	}	
return 0;
}

