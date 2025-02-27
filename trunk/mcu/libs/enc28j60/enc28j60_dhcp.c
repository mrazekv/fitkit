/*******************************************************************************
   enc28j60_dhcp.c: Implementace protokolu DHCP.
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
#include "enc28j60_dhcp.h"
#include "enc28j60_udp.h"
#include "enc28j60_ip.h"
#include "enc28j60_arp.h"
#include "enc28j60_spi.h"

#include "enc28j60_string.h"

extern unsigned char local_mac[MAC_LEN];
unsigned char dhcp_state = DHCP_DISCOVER;
unsigned long magic_cookie = 0x63538263;

unsigned char dhcp_message_type;
unsigned long dhcp_server_id = 0;
unsigned long dhcp_netmask = 0;
unsigned long dhcp_router = 0;
unsigned long dhcp_dns_server = 0;
unsigned long dhcp_transaction_id = 0x12345678;
unsigned long dhcp_requested_ip = 0;
unsigned char dhcp_timeout = DHCP_TIMEOUT;
	
//Odeslání discover zprávy
char dhcp_send_discover();
//odeslání request zprávy
char dhcp_send_request();

// UDP handler - prijem DHCP zprav
UDP_APP_HANDLER(dhcp_fsm){
	//term_send_str_crlf(" DHCP zprava");
	struct bootp_h bootp_header;
	
	if(rx_left() < BOOTP_HEADER_LEN_TOTAL + MAGIC_LEN)	//paket musi byt vetsi nez bootp hlavicka + magic cookie
		return;
	
	rx_read(&bootp_header, BOOTP_HEADER_LEN);

	if(bootp_header.opcode != BOOTP_REPLY){				//pouze BOOTP odpovedi
		return;}
		
	if(bootp_header.hw_type != BOOTP_HW_ETH){			//pouze ethernet protokol
		return;}

	if(bootp_header.hw_address_length != MAC_LEN){		//6-bajtová MAC
		return;}
	
	if(bootp_header.transaction_id != dhcp_transaction_id){		//kontrola ID transkace (pouzivam cast MAC adresy)
		return;}
		
	rx_skip(BOOTP_HEADER_LEN_TOTAL - BOOTP_HEADER_LEN);						//prekoceni dalsich polozek hlavicky (server name, filename)
	
		//magic packet
	unsigned long dhcp_magic;
	rx_read(&dhcp_magic,MAGIC_LEN);
		
	if(dhcp_magic != MAGIC_COOKIE)									//kontrola pritomnosti magic_cookie
		return;
		
	dhcp_requested_ip = bootp_header.your_ip;							//nabizena IP adresa
	unsigned int option;

		//cteni DHCP nastaveni
	while((rx_left() >0) & ((option=rx_getc()) != DHCP_END)){		//precteni typu parametru
		
		if(option == DHCP_PAD)							//mezera
			continue;

		int len = rx_getc();					//delka parametru

		switch(option){
			case DHCP_MESSAGE_TYPE:				//typ zpravy
				dhcp_message_type = rx_getc();	
				break;
			case DHCP_ROUTER:					//IP routeru
				rx_read(&dhcp_router,IP_LEN);
				break;
			case DHCP_DNS_SERVER:				//IP DNS serveru
				rx_read(&dhcp_dns_server,IP_LEN);
				break;
			case DHCP_SERVER_ID:				//IP DHCP serveru
				rx_read(&dhcp_server_id,IP_LEN);	
				break;
			case DHCP_SUBNET_MASK:				//maska podsite
				rx_read(&dhcp_netmask,IP_LEN);
				break;
		
			default:
				rx_skip(len);
		}	
	}
	switch(dhcp_state){
		case DHCP_DISCOVER:
			
			switch(dhcp_message_type){		//prislusna akce na prichozi zpravu
				case DHCP_OFFER:		//nabidka IP
						dhcp_state = DHCP_REQUEST;
						dhcp_timeout = DHCP_TIMEOUT;
						dhcp_send_request();
					break;
					
				default:
						dhcp_reset();
			}
			
		case DHCP_REQUEST:
			
			switch(dhcp_message_type){			//prislusna akce na prichozi zpravu
					
					//potvrzeni, ze se muze nabidnuta IP adresa pouzivat
				case DHCP_ACK:
					
					//nastaveni IP adresy,masky,brany,DNS
					set_ip_l(HTONL(dhcp_requested_ip));
					set_netmask_l(HTONL(dhcp_netmask));
					set_gateway_l(HTONL(dhcp_router));
					set_dns_server_l(HTONL(dhcp_dns_server));
					dhcp_state = DHCP_ACK;
		
					//zavreni socketu s DHCP
					udp_unbind(DHCP_CLIENT_PORT);
					break;	
					
				case DHCP_OFFER:
					break;	
					//odmitnuti serveru
				case DHCP_DECLINE:	
				case DHCP_NACK:
					dhcp_reset();
					break;
			}		
	}
return;
}


/**
 \brief Inicializace DHCP protokolu.
 **/
char dhcp_init(){
	set_ip(0,0,0,0);				//nulovani IP adres
	set_netmask(255,255,255,255);
	set_gateway(0,0,0,0);
	dhcp_state = DHCP_DISCOVER;
	dhcp_timeout = DHCP_TIMEOUT;
	enc_memcpy(&dhcp_transaction_id, &local_mac,MAC_LEN -2); 	//ID transkace (pouziva se cast MAC adresy)
	
	//zaregistrovani portu 68 pro prijem DHCP zprav
return udp_bind(DHCP_CLIENT_PORT,dhcp_fsm);
}

/**
 \brief Restart DHCP klienta.
 **/
char dhcp_reset(){
	set_ip(0,0,0,0);				//nulovani IP adres
	set_netmask(255,255,255,255);
	set_gateway(0,0,0,0);
	dhcp_state = DHCP_DISCOVER;
	dhcp_timeout = DHCP_TIMEOUT;
	dhcp_transaction_id++;
	
	//zaregistrovani portu 68 pro prijem DHCP zprav
return udp_bind(DHCP_CLIENT_PORT,dhcp_fsm);	
}

/**
 \brief DHCP èasovaè, funkci je nutné volat v èasových intervalech pro správnou funkci DHCP.
 		\n Doba mezi voláními èasovaèe by mìla být mezi 500 a 1000ms.
 **/
void dhcp_timer(){
	
	if(dhcp_timeout == 0)
		dhcp_reset();
	
	switch(dhcp_state){
		
		case DHCP_DISCOVER:
			dhcp_send_discover();
			dhcp_timeout--;
			break;
		
		case DHCP_REQUEST:
			dhcp_send_request();
			dhcp_timeout--;	
			break;
		
		case DHCP_ACK:
			
			if(link_change()){		//stav linky se zmenil
			
				if(!link_up()){		//kontrola, zda je linka zapnuta
							
					//kabel odpojen, reset DHCP nastaveni
					dhcp_reset();
					arp_table_clear();
					return;	
				}
			}
			break;
	}
	
return;
}

/**
 \brief Vrátí stav DHCP.
 **/
char dhcp_ready(){
	
	if(dhcp_state == DHCP_ACK){
		return 1;
	}
	
return 0;	
}


//Odeslání discover zprávy
char dhcp_send_discover(){

	struct bootp_h bootp_header;
	enc_memset(&bootp_header,0,BOOTP_HEADER_LEN);
	
		//vyplneni hlavicky
	bootp_header.opcode = BOOTP_REQUEST;
	bootp_header.hw_type = BOOTP_HW_ETH;
	bootp_header.hw_address_length = MAC_LEN;
	bootp_header.transaction_id = dhcp_transaction_id;
	enc_memcpy(&bootp_header.client_hw_address, &local_mac, MAC_LEN);
	
		//inicializace bufferu
	if(tx_init(UDP_PROTO) < 0)
		return -1;
	
		//zapsani hlavicky
	tx_write(&bootp_header,BOOTP_HEADER_LEN);
		//vyplneni mista pro host name a boot filename
	tx_fill(0,BOOTP_HEADER_LEN_TOTAL - BOOTP_HEADER_LEN);
		//magic cookie
	tx_write(&magic_cookie,MAGIC_LEN);
	
		//DHCP request (zadost)
	tx_putc(DHCP_MESSAGE_TYPE);
	tx_putc(1);
	tx_putc(DHCP_DISCOVER);

	tx_putc(12);			//typ:hostname
	tx_putc(6);				//delka
	tx_write("FITKit",6);	//hodnota


	tx_putc(55);			//typ:parameter request list
	tx_putc(3);				//delka
	tx_putc(1);				//maska
	tx_putc(3);				//router
	tx_putc(6);				//DNS
	
	tx_putc(DHCP_END);		//ukonceni DHCP zadosti
	
	tx_close();
	
		//odeslani DHCP zadosti
	udp_send(DHCP_SERVER_PORT,DHCP_CLIENT_PORT,IP_BROADCAST);

	dhcp_state = DHCP_DISCOVER;

return 0; 
}

char dhcp_send_request(){
	
		//inicializace tx bufferu
	if(tx_init(UDP_PROTO) < 0)
		return -1;
	
		//vytvoreni BOOTP hlavicky
	struct bootp_h bootp_header;
	enc_memset(&bootp_header,0,BOOTP_HEADER_LEN);
	
	bootp_header.opcode = BOOTP_REQUEST;
	bootp_header.hw_type = BOOTP_HW_ETH;
	bootp_header.hw_address_length = MAC_LEN;
	bootp_header.transaction_id = dhcp_transaction_id;
	enc_memcpy(&bootp_header.client_hw_address, &local_mac, MAC_LEN);
	
		//zapis do tx bufferu
	tx_write(&bootp_header, BOOTP_HEADER_LEN);
	tx_fill(0,BOOTP_HEADER_LEN_TOTAL - BOOTP_HEADER_LEN);
		
		//DHCP magic_cookie
	tx_write(&magic_cookie,MAGIC_LEN);
	
		//zapsani DHCP parametru
	tx_putc(DHCP_MESSAGE_TYPE);		//typ zpravy
	tx_putc(1);
	tx_putc(DHCP_REQUEST);			//zadost
	
	tx_putc(DHCP_REQUESTED_IP);		//zadost o nabizenou IP
	tx_putc(4);
	tx_write(&dhcp_requested_ip, IP_LEN);
	
	tx_putc(DHCP_SERVER_ID);		// ID DHCP serveru, ktery ucinil nabidku
	tx_putc(4);
	tx_write(&dhcp_server_id, IP_LEN);
	
	
	tx_putc(12);			//typ:hostname
	tx_putc(6);				//delka
	tx_write("FITKit",6);	//hodnota


	tx_putc(55);			//typ:parameter request list
	tx_putc(3);				//delka
	tx_putc(1);				//maska
	tx_putc(3);				//router
	tx_putc(6);				//DNS
			
	
	tx_putc(DHCP_END);		//ukonceni DHCP zadosti
	tx_close();
	
		//odeslani
	udp_send(DHCP_SERVER_PORT,DHCP_CLIENT_PORT,IP_BROADCAST);
		
	dhcp_state = DHCP_REQUEST;

return 0;
}
