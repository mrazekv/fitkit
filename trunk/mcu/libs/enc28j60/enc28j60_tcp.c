/*******************************************************************************
   enc28j60_tcp.c: Transportní vrstva modelu TCP/IP
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
#include "enc28j60_ip.h"
#include "enc28j60_tcp.h"


unsigned long dest_seq_num = 0;
unsigned long local_seq_num = 0;
unsigned long expected_ack_num = 0;

struct ip_h* ip_header_ptr;

extern unsigned long local_ip;
extern unsigned int tcp_fsm_timer;

struct tcp_socket socket;


/**
 \brief Funkce volaná funkcí ip_recv() po pøíchodu TCP datagramu.
 **/
void tcp_recv(struct ip_h* ip_header){

	struct tcp_h tcp_header;
	
	if(rx_read(&tcp_header, TCP_HEADER_LEN) != TCP_HEADER_LEN)
		return;
		
	//port filtr
	if( HTONS(tcp_header.dest_port) != socket.src_port)
		return;	
		
	//IP filtr
	if(socket.state != FSM_LISTEN){
		
		if(socket.dest_port != NTOHS(tcp_header.src_port))
			return;
		
		if(socket.dest_ip != NTOHL(ip_header->src_ip))
			return;
	}	
	
	//pokud je hlavicka delsi nez obvyklych 20B (volitelne polozky), zahodime je
	if( ( ((unsigned char)tcp_header.data_offset) <<2) > TCP_HEADER_LEN){		//header_len *4
		rx_skip ( ( ((unsigned char)tcp_header.data_offset) <<2) - TCP_HEADER_LEN);
	}
			
	ip_header_ptr = ip_header;	

	//volani konecneho automatu
	tcp_fsm(&tcp_header);
	
return;	
}



/**
 \brief Odešle TCP datagram - volána uživatelem.
 **/
char tcp_send(){
	
	if(tcp_connected()){
		tx_seek(TCP_HEADER);
		tx_save(TX3);
		expected_ack_num = local_seq_num + tx_data_len() - TCP_HEADER_LEN;	
		
		if(tcp_fsm(NULL) == -1){
			return -1;
		}
		else{
			return 0;
		}
	}
return -1;
}


/**
 \brief Odešle TCP datagram - volána koneèným automatem, interní funkce.
 **/
char tcp_out(unsigned long seq, unsigned long ack, unsigned char flags){
/*
	if(!link_up())			//kabel odpojen,nebudeme odesilat
		return -1;
*/
	struct tcp_h tcp_header;

	tcp_header.src_port = HTONS(socket.src_port);
	tcp_header.dest_port = HTONS(socket.dest_port);
	tcp_header.seq_num = HTONL(seq);
	
	if((flags & TCP_ACK) != 0)
		tcp_header.ack_num = HTONL(ack);
	else
		tcp_header.ack_num = 0;
		
	tcp_header.reserved = 0;
	tcp_header.data_offset = TCP_HEADER_LEN >> 2;
	tcp_header.flags = flags;
	tcp_header.window_size = HTONS(WINDOW_SIZE);
	tcp_header.checksum = 0;
	tcp_header.urgent_ptr = 0;
		
	tx_seek(TCP_HEADER);
	tx_write(&tcp_header, TCP_HEADER_LEN);

	tx_seek(TCP_HEADER);
	tcp_header.checksum = HTONS(tcp_checksum(socket.dest_ip, tx_data_len()));
		
	tx_write(&tcp_header, TCP_HEADER_LEN);
	
	

return ip_send(socket.dest_ip,IP_PROTO_TCP);	
}


/**
 \brief Vypoèítá 16-bitový checksum z TCP datagramu - interní funkce.
 **/
unsigned int tcp_checksum(unsigned long dest_ip,unsigned int length){

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
	
	sum += IP_PROTO_TCP;
	sum += length;	
	
	sum += (unsigned int) ~tx_checksum();
	
	while(sum >>16)
		sum = (sum & 0xFFFF) + (sum >> 16);

return (unsigned int) ~sum;
}

/**
 \brief Odešle prázdný TCP datagram,pouze hlavièku - volána koneèným automatem, interní funkce.
 **/
char tcp_send_header(unsigned char flags){
	
	if( tx_init(TCP_PROTO_HEADER_ONLY) < 0)
		return -1;
	tx_close();
	
return tcp_out(local_seq_num,dest_seq_num, flags);
}

/**
 \brief Reset koneèného automatu
 **/
void tcp_fsm_reset(){
	
	tx_unlock(TX3);
	
	if(socket.type == TCP_SERVER)
		socket.state = FSM_LISTEN;	
	else 	
		socket.state = FSM_IDLE;
}

/**
 \brief Vytvoøí TCP socket, zaregistruje TCP handler k portu \a src_port.
 		\n je možné vytvoøit pouze jeden TCP socket.
 **/
unsigned char create_tcp_socket(unsigned int src_port, void* handler){
		
	if(src_port != 0){
		socket.src_port = src_port;
		socket.app_handler = handler;
		socket.state = FSM_IDLE;
		return src_port;
	}
	else 
		return 0;	
}

/**
 \brief Odstraní TCP socket
 \retval 0 - socket odstranìn
 \retval -1 - chyba, socket je používán
 **/
char delete_tcp_socket(){

	if(tcp_is_idle()){
		socket.src_port = 0;
		socket.app_handler = NULL;
		socket.state = FSM_IDLE;
		return 0;
	}	
	else 
		return -1;
}


/**
 \brief Zmìní TCP handler v socketu.
 **/
void change_tcp_handler(void* new_handler){
	
	socket.app_handler = new_handler;
	
return;	
}

/**
 \brief Pøipojí se ke vzdálenému poèítaèi.
 **/
char tcp_connect(unsigned long ip, unsigned int port){
	
	if(socket.state == FSM_IDLE){
	socket.dest_ip = ip;
	socket.dest_port = port;
	socket.state = FSM_NOT_CONNECTED;
	socket.type = TCP_CLIENT;
	tcp_fsm(NULL);
	return 0;
	}
	else return -1;
}

/**
 \brief Naslouchání socketu na portu.
 **/
char tcp_bind(){
	
	if((socket.state == FSM_IDLE) | ( socket.state == FSM_LISTEN)){
		socket.state = FSM_LISTEN;
		socket.type = TCP_SERVER;
		return 1;
	}
	
return 0;
}

/**
 \brief Zrušení naslouchání socketu na portu.
 **/
char tcp_unbind(){
	if(( socket.state == FSM_LISTEN)){
		socket.dest_port = 0;
		socket.state = FSM_IDLE;
		socket.type = TCP_SERVER;
		return 1;
	}
return 0;
}

/**
 \brief Ukonèí TCP spojení (ne zcela, pouze koneèný automat pøejde do stavu ukonèení spojení)
 **/
inline void tcp_disconnect(){
	if(socket.state == FSM_ESTABILISHED){
		tcp_fsm_timer = FIN_WAIT1_TIMEOUT;
		socket.state = FSM_HALF_OPEN;
		expected_ack_num++;				//+1 za FIN
		tcp_fsm(NULL);
	}
}

/**
 \brief Vrací stav spojení
 **/
char tcp_connected(){
	
	if(socket.state == FSM_ESTABILISHED)
		return 1;
		
else return 0;
}

/**
 \brief Vrací stav TCP socketu
 **/
char tcp_is_idle(){
	
	if((socket.state == FSM_IDLE) | ( socket.state == FSM_LISTEN)){
		return 1;
	}
	
return 0;

}
/**
 \brief TCP èasovaè, funkci je nutné volat v èasových intervalech pro správnou funkci spojení (pøeposílání datagramù apod.).
 		\n doba mezi voláními èasovaèe by mìla být nejménì 250ms a nemìla pøesáhnout 2 vteøiny.
 **/
inline void tcp_timer(){

tcp_fsm(NULL);	
}
