/*******************************************************************************
   enc28j60_tcp_fsm.c: Stavovy automat TCP
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
#include "enc28j60_tcp.h"

extern unsigned long dest_seq_num;
extern unsigned long local_seq_num;
extern unsigned long expected_ack_num;
unsigned int dest_window_size;
unsigned int tcp_retransmit_timer;
unsigned int tcp_fsm_timer;

struct ip_h* ip_header_ptr;

extern struct tcp_socket socket;

//************************************
//TCP stavovy automat
//************************************
/**
 \brief Funkce realizující TCP koneèný automat, volána buï pøi pøíchodu TCP datagramu, nebo èasovaèem(zajišuje pøeposílání nepotvrzených dat). 
 **/
char tcp_fsm(struct tcp_h * tcp_header){

	switch(socket.state){
		
		case FSM_IDLE:
				tcp_fsm_timer = SYN_TIMEOUT;
			break;
		
		//***************************************
		case FSM_NOT_CONNECTED:

			if(tcp_header != NULL){
				
				if( tcp_header->flags == (TCP_SYN | TCP_ACK)){
				
					dest_seq_num = NTOHL(tcp_header->seq_num);
					dest_window_size = NTOHS(tcp_header->window_size);	

					//prechod do stavu SYN
					tcp_retransmit_timer = MAX_RETRANSMIT;
					socket.state = FSM_ESTABILISHED;
					dest_seq_num++;
					local_seq_num++;
					expected_ack_num = local_seq_num;
					
					tcp_retransmit_timer = MAX_RETRANSMIT;			
					//poslani ACK
					return tcp_send_header(TCP_ACK);
				}
				else if( ((tcp_header->flags) & TCP_RST) > 0 ){
					tcp_fsm_reset();
					return -1;
				}
				else{
					return tcp_send_header(TCP_RST);
				}
				
			}
			//volání FSM èasovaèem
			else{
				tcp_fsm_timer--;
				if(tcp_fsm_timer == 0 ){
					tcp_fsm_reset();
				}
				
				local_seq_num = 0;
				dest_seq_num = 0;
				
				//poslani SYN

				return tcp_send_header(TCP_SYN);

			}
			
			break;
			
		//***************************************
		case FSM_LISTEN:
			
			if(tcp_header != NULL){
				//prijeti SYN paketu
				if( tcp_header->flags == TCP_SYN ){
					
					socket.dest_port = NTOHS(tcp_header->src_port);
					socket.dest_ip = NTOHL(ip_header_ptr->src_ip);
					dest_seq_num = NTOHL(tcp_header->seq_num);
					local_seq_num = 0;
					dest_window_size = NTOHS(tcp_header->window_size);
					
					//prechod do stavu SYN
					socket.state = FSM_SYN;
					dest_seq_num++;
					
					tcp_fsm_timer = SYN_TIMEOUT;
					//poslani SYN_ACK
					return tcp_send_header(TCP_SYN | TCP_ACK);
				}
				else{
					return tcp_send_header(TCP_RST);
				}
			}
			//volání FSM èasovaèem
			else return 0;
			
			break;
		
		//***************************************
		case FSM_SYN:
			if(tcp_header != NULL){
				
				tcp_fsm_timer = SYN_TIMEOUT;				
				if( tcp_header->flags == TCP_ACK ){
					local_seq_num++; 
					
					tcp_retransmit_timer = MAX_RETRANSMIT;
					socket.state = FSM_ESTABILISHED;
					expected_ack_num = local_seq_num;
					tx_unlock(TX3);
					return 0;
				}
				//prichod RST
				if( ((tcp_header->flags) & TCP_RST) > 0 ){
					tcp_fsm_reset();
					return -1;
				}
				
			}
			//volání FSM èasovaèem
			else{
				tcp_fsm_timer--;
				if(tcp_fsm_timer == 0 ){
					tcp_fsm_reset();
				}
				
				//poslani SYN_ACK
				return tcp_send_header(TCP_SYN | TCP_ACK);

			}
			break;
		
		//***************************************
		//spojeni navazano
		case FSM_ESTABILISHED:
				
		//***************************************
		//polouzavrene spojeni - protejsek jeste smi posilat data
		case FSM_HALF_OPEN:
				
			//prichozi paket
			if(tcp_header != NULL){

				//preposilani paketu, druha strana nedostala ACK, poslu ACK znovu
				if(HTONL(tcp_header->seq_num) < dest_seq_num){
					
					return tcp_send_header(TCP_ACK);
				}
				
				//trideni podle flagu
				switch(tcp_header->flags){
					
					//prichozi SYN ACK  - ACK paket z 3-way handshake nebyl dorucen
					case (TCP_SYN | TCP_ACK):
						
						if(socket.state == FSM_ESTABILISHED){
							
							return tcp_send_header(TCP_ACK);
						
						}
						else if(socket.state == FSM_HALF_OPEN){
							
							return tcp_send_header(TCP_RST);
						}
						break;
					
					case TCP_ACK:
						
						//datova cast paketu je vetsi jak 0 -> prichozi data
						if(rx_left() > 0){
							if(dest_seq_num != NTOHL(tcp_header->seq_num)){
								//sekvencni cisla nesedi, data nedosla v poradi, odeslu potvrzeni poslednich prijatych dat
								return tcp_send_header(TCP_ACK);
							}
							dest_seq_num += rx_left();
							//odeslani ACK
							tcp_send_header(TCP_ACK);
							//zavolani handleru
							(*(socket.app_handler))((struct ip_h *)&ip_header_ptr,(struct tcp_h *) &tcp_header);
						}
						
						if(expected_ack_num == NTOHL(tcp_header->ack_num)){		//ACK cislo sedi, posledni paket potvrzen
							tcp_retransmit_timer = MAX_RETRANSMIT;
							local_seq_num = expected_ack_num;
							tx_unlock(TX3);
						}
						//patri ke stavu FSM_HALF_OPEN - potvrzeni posledniho datoveho paketu v pripade, ze jiz byl poslan i paket FIN ACK(ten se totiz pocita za 1 bajt)
						else if((socket.state == FSM_HALF_OPEN) & ((expected_ack_num - 1) == NTOHL(tcp_header->ack_num))){
							tcp_fsm_timer = FIN_WAIT1_TIMEOUT;
							local_seq_num = expected_ack_num - 1;
							tx_unlock(TX3);
						}
						else{
							return -1;	
						}
						
						
						break;	
					
					//prichod RST - okamzite ukonceni spojeni
					case TCP_RST:
					case (TCP_RST | TCP_ACK):
								
						tcp_fsm_reset();
						return 0;
						
					//nová data + potvrzení pøijetí	
					case (TCP_PSH | TCP_ACK):

						//kontrola,zda ocekavane ACK cislo bylo prijato
						if(expected_ack_num == NTOHL(tcp_header->ack_num)){		//ACK cislo sedi, posledni paket potvrzen
							tcp_retransmit_timer = MAX_RETRANSMIT;
							local_seq_num = expected_ack_num;
							tx_unlock(TX3);
						}
						//patri ke stavu FSM_HALF_OPEN - potvrzeni posledniho datoveho paketu v pripade, ze jiz byl poslan i paket FIN ACK(ten se totiz pocita za 1 bajt)
						else if((socket.state == FSM_HALF_OPEN) & ((expected_ack_num - 1) == NTOHL(tcp_header->ack_num))){
							tcp_fsm_timer = FIN_WAIT1_TIMEOUT;
							local_seq_num = expected_ack_num - 1;
							tx_unlock(TX3);
						}
						else{
							return -1;	
						}
					
					//nová data	
					case TCP_PSH:		
						
						if(dest_seq_num != NTOHL(tcp_header->seq_num)){
							return tcp_send_header(TCP_ACK);
						}

						dest_seq_num += rx_left();
						
						//odeslani ACK
						tcp_send_header(TCP_ACK);
						//zavolani handleru
						(*(socket.app_handler))((struct ip_h *)&ip_header_ptr,(struct tcp_h *) &tcp_header);
						break;
						
					//ukonèení spojení (paket mùže obsahovat i data a potvrzení pøijetí paketu)	
					case (TCP_PSH | TCP_ACK | TCP_FIN):		
					case (TCP_FIN | TCP_ACK):
						
						//datova cast paketu je vetsi jak 0 -> prichozi data
						if(rx_left() > 0){
							if(dest_seq_num != NTOHL(tcp_header->seq_num)){
								//sekvencni cisla nesedi, data nedosla v poradi, odeslu potvrzeni poslednich prijatych dat
								return tcp_send_header(TCP_ACK);
							}
							dest_seq_num += rx_left();
							//zavolani handleru
							(*(socket.app_handler))((struct ip_h *)&ip_header_ptr,(struct tcp_h *) &tcp_header);
						}
						
						
						if(expected_ack_num != NTOHL(tcp_header->ack_num)){
							if( dest_seq_num == NTOHL(tcp_header->seq_num)){
								//return tcp_send_header(TCP_ACK);
								return -1;
							}
						}
						else{
							tcp_retransmit_timer = MAX_RETRANSMIT;
							local_seq_num = expected_ack_num;
							tx_unlock(TX3);
						}
					
					//ukonèení spojení
					case TCP_FIN:
							
						dest_seq_num++;
						
						if(socket.state == FSM_ESTABILISHED){	
								
							socket.state = FSM_CLOSE_WAIT;
							tcp_fsm_timer = CLOSE_WAIT_TIMEOUT;
							expected_ack_num = local_seq_num +1;		//+1 za odesilany FIN
							//odeslani FIN ACK
							tcp_send_header(TCP_FIN | TCP_ACK);	
						}
						else if(socket.state == FSM_HALF_OPEN){
						//odesle posledni potvrzovaci paket											
							tcp_send_header(TCP_ACK);
										
							expected_ack_num = local_seq_num;
							tcp_fsm_timer = TIME_WAIT_TIMEOUT;
							socket.state = FSM_TIME_WAIT;
						}	
						break;
				}
			}
			
			
			//volání FSM èasovaèem
			else{
				if(socket.state == FSM_ESTABILISHED){
					// preposlani nepotvrzeneho paketu
					if(expected_ack_num != local_seq_num){
						if(tcp_retransmit_timer-- == 0)			//paket se delsi dobu nedari dorucit - ukonceni spojeni
							tcp_fsm_reset();
							
						tx_load(TX3);
						return tcp_out(local_seq_num, dest_seq_num, TCP_PSH | TCP_ACK);
					}
				}
				else if(socket.state == FSM_HALF_OPEN){
					// preposlani nepotvrzenych paketu
					if(expected_ack_num != local_seq_num){

						if((expected_ack_num - 1) != local_seq_num ){	//nepotvrzena data
							if(tcp_retransmit_timer-- == 0)			//paket se delsi dobu nedari dorucit - ukonceni spojeni
								tcp_fsm_reset();
								
							tx_load(TX3);
							tcp_out(local_seq_num, dest_seq_num, TCP_PSH | TCP_ACK);
						}
						
						//nepotvrzen FIN						
						tcp_fsm_timer--;
						if(tcp_fsm_timer == 0 ){
							tcp_fsm_reset();
						}
						//odeslani FIN/ACK zpravy, aktivni ukonceni spojeni
						tcp_send_header(TCP_FIN | TCP_ACK);
						
					}
					
				}
			}
			break;


				
		//***************************************
		case FSM_CLOSE_WAIT:
			
			//prichozi paket
			if(tcp_header != NULL){
				tcp_fsm_timer = CLOSE_WAIT_TIMEOUT;
				//konec spojeni
				if( tcp_header->flags == TCP_ACK){
					
					if(expected_ack_num != NTOHL(tcp_header->ack_num)){		//ACK cislo nesedi, posledni paket nepotvrzen
							return -1;
						}
					tcp_retransmit_timer = MAX_RETRANSMIT;
					local_seq_num = expected_ack_num;
					tx_unlock(TX3);
					
					tcp_fsm_reset();
					break;	
				}
				
				else if( ((tcp_header->flags) & TCP_RST) > 0 ){
					
					tcp_fsm_reset();
					break;
				}

			}
			//volání FSM èasovaèem
			else{
				// preposlani nepotvrzeneho paketu
				if(expected_ack_num != (local_seq_num + 1)){		//kvuli expected_ack_num = local_seq_num+1; ve TCP_ESTABILISHED, prijem FIN paketu
					if(tcp_retransmit_timer-- == 0)			//paket se delsi dobu nedari dorucit - ukonceni spojeni
						tcp_fsm_reset();
						
					tx_load(TX3);
					return tcp_out(local_seq_num, dest_seq_num, TCP_PSH | TCP_ACK);
				}
				else{
					tcp_fsm_timer--;
					if(tcp_fsm_timer == 0 ){
						tcp_fsm_reset();
					}
					return tcp_send_header(TCP_FIN | TCP_ACK);	
				}
			}
			
			
			
		//***************************************
		case FSM_TIME_WAIT:
			
			//dosel paket od protejsku, musime preposlat ACK paket
			if(tcp_header != NULL){
				
				tcp_fsm_timer = TIME_WAIT_TIMEOUT;
				
				if( ((tcp_header->flags) & TCP_RST) > 0 ){
					tcp_fsm_reset();
					return 0;
				}	
				//odesle posledni potvrzovaci paket
				return tcp_send_header(TCP_ACK);
			}
			//volání FSM èasovaèem
			else{
				//timeout, az vyprsi,muzeme ukoncit spojeni
				tcp_fsm_timer--;
				if(tcp_fsm_timer == 0 ){
					tcp_fsm_reset();
				}
			}
			break;

		default:
				return -1;
	}
	
return 0;
}
