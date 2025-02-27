/*******************************************************************************
   enc28j60_udp.c: Transportní vrstva modelu TCP/IP, protokol UDP
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

#include "enc28j60_udp.h"
#include "enc28j60_udp.h"
#include "enc28j60_spi.h"

struct udp_socket udp_socket_table[SOCKET_COUNT];
extern unsigned long local_ip;

/**
 \brief Funkce volaná funkcí ip_recv() po pøíchodu UDP datagramu.
 **/
void udp_recv(struct ip_h* ip_header){
	
	struct udp_h udp_header;
	
	if(rx_read(&udp_header, UDP_HEADER_LEN) != UDP_HEADER_LEN)
		return;
	
	unsigned char i;
	/*kontrola portu, volani obsluzne aplikace*/
	//vyhledani existujiciho zaznamu
	for(i=0; i<SOCKET_COUNT; i++){

		if(udp_socket_table[i].port == NTOHS(udp_header.dest_port)){		//prepsani stavajiciho handleru
			(*(udp_socket_table[i].app_handler))(ip_header, &udp_header);
			return;
		}
	}
return;
}

/**
 \brief Odešle UDP datagram.
 **/
char udp_send(unsigned int dest_port,unsigned int src_port, unsigned long ip){
/*	
	if(!link_up()){			//kabel odpojen,nebudeme odesilat
		tx_unlock(TX1);
		return -1;
	}
*/	
	struct udp_h udp_header;
	
	tx_seek(UDP_HEADER);
	
	udp_header.src_port = HTONS(src_port);			//sestaveni hlavicky
	udp_header.dest_port = HTONS(dest_port);
	udp_header.length = HTONS(tx_data_len());
	udp_header.checksum = 0x0000;
	
	tx_write(&udp_header, UDP_HEADER_LEN);			//zapsani hlavicky s prazdnym checksumem
	
	tx_seek(UDP_HEADER);
	udp_header.checksum = HTONS( udp_checksum(ip,tx_data_len() ));	//vypocitani checksumu
	tx_write(&udp_header, UDP_HEADER_LEN);			//prepsani hlavicky s platnym checksumem
	

return ip_send(ip,IP_PROTO_UDP);
}

/**
 \brief Vypoèítá 16-bitový checksum z UDP datagramu - interní funkce.
 **/
unsigned int udp_checksum(unsigned long dest_ip,int length){
	
	unsigned long sum = 0;
	unsigned int* ptr;
	
	ptr = (unsigned int*)&local_ip;
	sum += *ptr;
	ptr++;
	sum += *ptr;
	
	ptr = (unsigned int*)&dest_ip;
	sum += *ptr;
	ptr++;
	sum += *ptr;
	
	sum += IP_PROTO_UDP;
	sum += length;	
	
	sum += (unsigned int) ~tx_checksum();
	
	while(sum >>16)
		sum = (sum & 0xFFFF) + (sum >> 16);

return (unsigned int) ~sum;
}


//*********************************************
//fce pro praci s handlery k jednotlivym portum

/**
 \brief Zaregistruje handler k èíslu portu.
 **/
unsigned int udp_bind(unsigned int port,void* handler){
	
	unsigned char i;
	
	//vyhledani existujiciho zaznamu
	for(i=0; i<SOCKET_COUNT; i++){
		if(udp_socket_table[i].port == port){		//prepsani existujiciho handleru
			udp_socket_table[i].app_handler = handler;
			return port;
		}
	}
	
	//hledani volneho mista v tabulce
	for(i=0; i<SOCKET_COUNT; i++){
		if(udp_socket_table[i].port == 0){			//zaregistrovani noveho portu
			udp_socket_table[i].port = port;
			udp_socket_table[i].app_handler = handler;
			return port;
		}
	}
	
return 0;
}

/**
 \brief Odregistruje handler od èísla portu.
 **/
void udp_unbind(unsigned int port){

	unsigned char i;
	//vyhledani existujiciho zaznamu
	for(i=0; i<SOCKET_COUNT; i++){
		if(udp_socket_table[i].port == port){		//prepsani stavajiciho handleru
			udp_socket_table[i].port = 0;
			return;
		}
	}
	
return;
}

