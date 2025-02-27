/*******************************************************************************
   enc28j60_arp.c: ARP modul
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
#include "enc28j60_spi.h"
#include "enc28j60_eth.h"
#include "enc28j60_arp.h"
#include "enc28j60_ip.h"

#include "enc28j60_string.h"

extern unsigned long local_ip;
extern unsigned long ip_netmask;
extern unsigned long default_gateway;
extern unsigned char broadcast_mac[MAC_LEN];

struct arp_record arp_table[ARP_TABLE_SIZE];	// ARP tabulka
unsigned char arp_last;

extern unsigned char local_mac[MAC_LEN];
unsigned char arp_next_free = 0;				//ukazatel na volne misto v tabulce / na nejstarsi zaznam

/**
 \brief Funkce volaná pøi pøíchodu ARP datagramu.
 **/
void arp_recv(){
	
	struct arp_h arp_header;
	
	rx_read(&arp_header,ARP_HEADER_LEN);
	
	// podpora pouze protokolu ethernet
	if(arp_header.hw_type != HTONS(ETH_HW_TYPE))
		return;

	// podpora pouze IP protokolu
	if(arp_header.proto_type != NTOHS(ETH_PROTO_IP))
		return;

	//kontrola delky MAC adresy
	if(arp_header.hw_addr_len != MAC_LEN)
		return;

	//kontrola delky IP adresy
	if(arp_header.proto_addr_len != IP_LEN)
		return;

	//kontrola IP adresy
	if(arp_header.target_ip != HTONL(local_ip))
		return;

	//ARP REPLY
	if(arp_header.operation == HTONS(ARP_REPLY)){					//jedna se o odpoved
		//ulozeni ARP zaznamu
		arp_table_add(NTOHL(arp_header.sender_ip), arp_header.sender_mac);	//ulozeni zaznamu do arp tabulky
	}
	//ARP REQUEST
	else if(arp_header.operation == HTONS(ARP_REQUEST)){					//jedna se o arp zadost
	
		//ulozeni ARP zaznamu
		arp_table_add(NTOHL(arp_header.sender_ip), arp_header.sender_mac);

		//doplneni hlavicky pro odpoved
		arp_header.operation = HTONS(ARP_REPLY);
		enc_memcpy(&arp_header.target_mac,&arp_header.sender_mac, MAC_LEN);	//prohozeni udaju sender a target
		arp_header.target_ip = arp_header.sender_ip;
		enc_memcpy(&arp_header.sender_mac,&local_mac, MAC_LEN);
		arp_header.sender_ip = HTONL(local_ip);


		tx_init(ARP_PROTO);				//prepnuti na druhou cast odesilaciho bufferu		
		tx_write(&arp_header, sizeof(struct arp_h));

		tx_close();
		
		eth_send(arp_header.target_mac,ETH_PROTO_ARP);
	}
	
return;
}

/**
 \brief Odešle ARP dotaz.
 **/
char arp_send(unsigned long dest_ip){
	
	struct arp_h arp_header;
	enc_memset(&arp_header, 0, ARP_HEADER_LEN);

	/*sestaveni hlavicky*/
	//protokol ethernet
	arp_header.hw_type = HTONS(ETH_HW_TYPE);
	// IP protokol
	arp_header.proto_type = HTONS(ETH_PROTO_IP);
	//typ operace, chceme REQUEST
	arp_header.operation = HTONS(ARP_REQUEST);
	//delka MAC adresy
	arp_header.hw_addr_len = MAC_LEN;
	//delka IP adresy
	arp_header.proto_addr_len = IP_LEN;
	//IP adresa odesilatele
	arp_header.sender_ip = HTONL(local_ip);
	//MAC adresa odesilatele
	enc_memcpy(&arp_header.sender_mac,&local_mac,MAC_LEN);
	//IP adresa prijemce
	arp_header.target_ip = HTONL(dest_ip);
	
	tx_init(ARP_PROTO);
	tx_write(&arp_header,ARP_HEADER_LEN);
	tx_close();

return eth_send(broadcast_mac,ETH_PROTO_ARP);
}	
	
/**
 \brief Vymaže ARP tabulku.
 **/
void arp_table_clear(){
	enc_memset(&arp_table, 0, sizeof(struct arp_record) * ARP_TABLE_SIZE);
	arp_last = 0;
}

/**
 \brief Pøidá záznam do ARP tabulky.
 **/
void arp_table_add(int ip,unsigned char* mac){

	
	char* ptr = arp_table_find(ip);
		
	if(ptr != NULL){		//zaznam jiz existuje
		enc_memcpy(ptr,mac,MAC_LEN);		//aktualizace mac_adresy	
		return;
	}

	//zaznam neexistuje, prepiseme posledni zaznam
	arp_table[arp_next_free].ip = ip;
	enc_memcpy( &(arp_table[arp_next_free].mac), mac, MAC_LEN);
	
	//posun arp_next_free pointeru (kruhove, pouze hodnoty <0;ARP_TABLE_SIZE> )
	if(++arp_next_free >= ARP_TABLE_SIZE)
		arp_next_free = 0;


return;
}

/**
 \brief Hledá MAC adresu v ARP tabulce podle IP adresy cíle.
 **/
unsigned char* arp_table_find(int ip){
	
	int i;
	for (i=0; i < ARP_TABLE_SIZE; i++){
		if(arp_table[i].ip == ip)
			return arp_table[i].mac;
	}

return NULL;
}
