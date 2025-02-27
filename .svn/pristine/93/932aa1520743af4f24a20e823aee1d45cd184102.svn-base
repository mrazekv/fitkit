/*******************************************************************************
   enc28j60_ip.c: Sitova vrstva modelu TCP/IP
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
#include "enc28j60_icmp.h"
#include "enc28j60_tcp.h"
#include "enc28j60_udp.h"

// pro prevod 4xchar na 1 long, pro IP adresy, v "net order"
#define CHAR_TO_LONG(x0,x1,x2,x3) ( ((unsigned long)x0<<24)|((unsigned long)x1<<16)|((unsigned int)x2<<8)|x3)


extern unsigned char broadcast_mac[MAC_LEN];

unsigned long local_ip = 0;
unsigned long ip_netmask = 0;
unsigned long default_gateway = 0;
unsigned long dns_server = 0;
unsigned int ip_seq_num = 1;


/**
 \brief Funkce volaná funkcí eth_recv() pøi pøíchodu IP paketu.
 **/
void ip_recv(void){
	
	struct ip_h ip_header;
	
	if(rx_read(&ip_header,IP_HEADER_LEN) != IP_HEADER_LEN)
		return;
	
	//kontrola verze IP a minimalni delky hlavicky
	if((ip_header.ipv != IPv4) | (ip_header.header_len < 4))
		return;
				
	//kontrola fragmentacnich flagu
	if(ip_header.fragment & NTOHS(IP_FRAG_OFFSET|IP_FRAG_MORE))		//fragmentovany paket,zahodit
		return;	
		
	//kontrola IP adresy
	if(NTOHL(ip_header.dest_ip) != local_ip && (NTOHL(ip_header.dest_ip)|ip_netmask) != IP_BROADCAST )
		return;	
		

	//pokud je hlavicka delsi nez obvyklych 20B (volitelne polozky), zahodime je
	if( ( ((unsigned char)ip_header.header_len) <<2) > IP_HEADER_LEN){		//header_len *4
		rx_skip ( ( ((unsigned char)ip_header.header_len) <<2) - IP_HEADER_LEN);
	}
	
	//odstraneni paddingu
	if(NTOHS(ip_header.length) < MIN_IP_LEN){
		rx_cut_padding(MIN_IP_LEN - NTOHS(ip_header.length));
	
	}


	switch(ip_header.protocol){
		case IP_PROTO_ICMP:
			icmp_recv(&ip_header);
			break;
		case IP_PROTO_TCP:
			tcp_recv(&ip_header);
			break;
		case IP_PROTO_UDP:
			udp_recv(&ip_header);
			break;
	}
	
return;
}	

/**
 \brief Odešle IP paket.
 **/
char ip_send(unsigned long dest_addr,unsigned char protocol){
	
	struct ip_h ip_header;
	
	tx_seek(IP_HEADER);								//jiz zde kvuli volani tx_data_len() nize
	
	ip_header.ipv = IPv4;
	ip_header.header_len = IP_HEADER_LEN >> 2;		// deleno ctyrmi, udava se totiz v DWORD
	ip_header.tos = 0;
	ip_header.length = HTONS(tx_data_len());
	ip_header.identification = HTONS(ip_seq_num);
	ip_seq_num++;
	
	ip_header.fragment = 0;
	ip_header.ttl = DEFAULT_TTL;
	ip_header.protocol = protocol;
	ip_header.checksum = 0;
	ip_header.src_ip = HTONL(local_ip);
	ip_header.dest_ip = HTONL(dest_addr);
	
		//pri pocitani checksumu z hlavicky, ktera je big endian, jiz nemusime checksum prevadet pomoci HTONS
	ip_header.checksum = count_checksum(&ip_header,IP_HEADER_LEN);

	
	tx_write(&ip_header,IP_HEADER_LEN);
	
	
	//*****************************
	//cil nelezi ve stejne podsiti
	//*****************************
	if(default_gateway != 0){
		if((local_ip & ip_netmask) != (dest_addr & ip_netmask)){
			dest_addr = default_gateway;
		}
	}
	
	//********
	//ARP
	//*********
	char * dest_mac;
	
	if(dest_addr == IP_BROADCAST){			//IP broadcast se posila na broadcastovou mac
		dest_mac = broadcast_mac;
	}
	else{
		dest_mac = arp_table_find(dest_addr);
	}
	
	if(dest_mac == NULL){						//neexistuje arp zaznam
		
		//ulozeni ukazatelu do bufferu behem vyrizovani ARP dotazu
		unsigned char last_act = tx_act();
		
		tx_save(last_act);
		
		//odeslani ARP dotazu
		arp_send(dest_addr);
		
		unsigned int timer = ARP_TIMEOUT;		//timeout
		
		//cekani na ARP odpoved
		while((arp_table_find(dest_addr) == NULL) & (timer-- > 0) ){ //az dojde odpoved, zapise se do arp tabulky
			ENC28J60_idle();						//obsluha prichozich paketu
		};
		
		dest_mac = arp_table_find(dest_addr);
		
		if( dest_mac == NULL){						//odpoved na arp nedosla do timeoutu
			if(last_act == TX1)
				tx_unlock(TX1);							//vypnuti ochrany proti prepsani tx bufferu
			else if(last_act == TX2)
				tx_unlock(TX2);	

				
			return -2;
		}
		else{
			tx_load(last_act);
			return eth_send(dest_mac,ETH_PROTO_IP);		//odeslani paketu na ziskanou mac adresu
		}
		
	}
	
return eth_send(dest_mac,ETH_PROTO_IP);
}


/**
 \brief Vypoèítá 16-bitový checksum z bloku dat
 **/
unsigned int count_checksum(void* pointer,int count){	//vypocet CRC souctu,podle RFC 1071

  unsigned long sum = 0;
  unsigned int* ptr = (unsigned int*)pointer;

        while( count > 1 ){
			sum += *(ptr++);
			count -=2;
		}
		
		if(count == 1)
			sum += *((unsigned char*) ptr);

		while(sum >> 16){
	        sum = (sum & 0xFFFF) + (sum >> 16);
		}
     
return (unsigned int) ~sum;
}



/**
 \brief Nastavení lokální IP
 **/
inline void set_ip(unsigned char a0,unsigned char a1,unsigned char a2, unsigned char a3){
	local_ip = CHAR_TO_LONG(a0,a1,a2,a3);
}

/**
 \brief Nastavení lokální IP
 **/
inline void set_ip_l(unsigned long ip){
	local_ip = ip;
}
/**
 \brief Vrátí lokální IP adresu
 **/
inline unsigned long get_ip(){
	return local_ip;
}

/**
 \brief Nastavení masky
 **/
inline void set_netmask(unsigned char m0,unsigned char m1,unsigned char m2, unsigned char m3){
	ip_netmask = CHAR_TO_LONG(m0,m1,m2,m3);
}
/**
 \brief Nastavení masky
 **/
inline void set_netmask_l(unsigned long netmask){
	ip_netmask = netmask;
}
/**
 \brief Vrátí masku podsítì
 **/
inline unsigned long get_netmask(){
	return ip_netmask;
}

/**
 \brief Nastavení IP brány
 **/
inline void set_gateway(unsigned char g0,unsigned char g1,unsigned char g2, unsigned char g3){
	default_gateway = CHAR_TO_LONG(g0,g1,g2,g3);
}
/**
 \brief Nastavení IP brány
 **/
inline void set_gateway_l(unsigned long gateway){
	default_gateway = gateway;
}
/**
 \brief Vrátí IP adresu brány
 **/
inline unsigned long get_gateway(){
	return default_gateway;
}

/**
 \brief Nastavení IP DNS serveru
 **/
inline void set_dns_server(unsigned char d0,unsigned char d1,unsigned char d2, unsigned char d3){
	dns_server = CHAR_TO_LONG(d0,d1,d2,d3);
}
/**
 \brief Nastavení IP DNS serveru
 **/
inline void set_dns_server_l(unsigned long dns){
	dns_server = dns;
}
/**
 \brief Vrátí IP adresu DNS
 **/
inline unsigned long get_dns_server(){
	return dns_server;
}

