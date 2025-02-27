/*******************************************************************************
   enc28j60_icmp.h: Implementace protokolu ICMP
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
#include "enc28j60_ip.h"
#include "enc28j60_icmp.h"

ICMP_APP_HANDLER(*icmp_handler_);

unsigned int icmp_seq_num = 1;
unsigned char handler_set = 0; 



void icmp_recv(struct ip_h* ip_header){
	
	struct icmp_h icmp_header;
	
	rx_read(&icmp_header,ICMP_HEADER_LEN);
	
	if(handler_set == 1){
		//volani handleru
		(*(icmp_handler_))(ip_header, &icmp_header);	
	}
	else{ 

	
		//obsluha podle typu a kodu
		switch(icmp_header.type){
			case ICMP_ECHO:				//echo pozadavek
			
				if( tx_init(ICMP_PROTO) < 0)
					return;

				unsigned int len;
				unsigned char temp[5];			//buffer, pres ktery se budou kopirovat data ECHO zpravy	
				
				while(rx_left() > 0){			//precte vsechny data a da je do odesilaciho bufferu
					len = rx_read(&temp,5);
					tx_write(&temp,len);
				}
				tx_close();
					
					//odesle ICMP odpoved
				icmp_send(NTOHL(ip_header->src_ip),ICMP_ECHO_REPLY, ICMP_ECHO_CODE, NTOHS(icmp_header.id), NTOHS(icmp_header.seq));
				break;
		}
	}

return;
}

char icmp_send(unsigned long ip, unsigned char type, unsigned char code, unsigned int id, unsigned int sequence ){
/*	
	if(!link_up()){			//kabel odpojen,nebudeme odesilat
		tx_unlock(TX2);
		return -1;
	}
*/
	
	struct icmp_h icmp_header;
	
	icmp_header.type = type;		//zapsani dat do hlavicky
	icmp_header.code = code;
	icmp_header.checksum = 0x0000;
	icmp_header.id = HTONS(id);
	icmp_header.seq = HTONS(sequence);

	tx_seek(ICMP_HEADER);
	tx_write(&icmp_header,ICMP_HEADER_LEN);

	tx_seek(ICMP_HEADER);
	icmp_header.checksum = HTONS(tx_checksum());

	tx_write(&icmp_header,ICMP_HEADER_LEN);	
	
return ip_send(ip,IP_PROTO_ICMP);
}


void icmp_bind(void* handler){
	
	icmp_handler_ = handler;
	handler_set = 1;
}

void icmp_unbind(){
	handler_set = 0;
}
