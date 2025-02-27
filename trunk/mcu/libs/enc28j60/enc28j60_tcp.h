/*******************************************************************************
   enc28j60_tcp.h: Transportn� vrstva modelu TCP/IP
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

/**
	\file enc28j60_tcp.h
	\brief Transportn� vrstva modelu TCP/IP. P��jem a odes�l�n� TCP datagram�.
*/

#ifndef _ENC28J60_TCP_
#define _ENC28J60_TCP_

#include "enc28j60_ip.h"
 
#define TCP_HEADER_LEN	20		///< velikost TCP hlavi�ky

/**
 \cond
 **/
#define WINDOW_SIZE		1460	///< ���ka okna

//TCP flagy
#define TCP_CWR 0x80
#define TCP_ECE 0x40
#define TCP_URG 0x20
#define TCP_ACK 0x10
#define TCP_PSH 0x08
#define TCP_RST 0x04
#define TCP_SYN 0x02
#define TCP_FIN 0x01


#define TCP_SERVER	1
#define TCP_CLIENT	2


//TCP FSM stavy
#define FSM_IDLE			0			
#define FSM_NOT_CONNECTED	1
#define FSM_LISTEN			2
#define FSM_SYN				3
#define FSM_SYN_ACK			4

#define FSM_ESTABILISHED	5

#define FSM_FIN_WAIT1		6
#define FSM_HALF_OPEN		7
#define FSM_CLOSE_WAIT		8
#define FSM_TIME_WAIT		9

	
#define SYN_TIMEOUT			40		//pri volani tcp_timer 2x/s odpovida 20s - zkr�ceno
#define TIME_WAIT_TIMEOUT	5		//zkraceno, mensi cekani po skonceni -> rychleji se uvolni TCP spoj
#define FIN_WAIT1_TIMEOUT	40
#define FIN_TIMEOUT			15
#define CLOSE_WAIT_TIMEOUT	40
#define MAX_RETRANSMIT		40


/**
 \endcond
 **/
 
/**
 \brief struktura tcp_h - TCP hlavi�ka.
 \struct tcp_h
 **/
struct tcp_h{
	unsigned int src_port;			///< zdrojov� port
	unsigned int dest_port;			///< c�lov� port
	unsigned long seq_num;			///< sekven�n� ��slo paketu
	unsigned long ack_num;			///< potvrzovac� ��slo
	unsigned char reserved:4;		///< rezervov�no
	unsigned char data_offset:4;	///< velikost hlavi�ky v DOUBLEWORDech
	unsigned char flags;			///< TCP p��znaky
	unsigned int window_size;		///< velikost okna
	unsigned int checksum;			///< kontroln� sou�et
	unsigned int urgent_ptr;		///< Urgent pointer
};

///deklarace TCP callback handleru
#define TCP_APP_HANDLER(x) void (x)(struct ip_h *ip_header, struct tcp_h *tcp_header)

/**
 \cond
 **/
 
/**
 \brief struktura tcp_socket - TCP socket.
 \struct tcp_socket
 **/
struct tcp_socket{
	unsigned int src_port;			///< zdrojov� port
	unsigned int dest_port;			///< c�lov� port
	unsigned long dest_ip;			///< IP adresa c�le
	TCP_APP_HANDLER(*app_handler);	///< handler funkce, kter� je vol�na po p��chodu datagramu na \a src_port
	unsigned char state;			///< stav TCP spojen� (stavy kone�n�ho automatu)
	unsigned char type;				///< typ socketu (klient/server)
};

/**
 \brief Funkce realizuj�c� TCP kone�n� automat, vol�na bu� p�i p��chodu TCP datagramu, nebo �asova�em(zaji��uje p�epos�l�n� nepotvrzen�ch dat). 
 \param tcp_header - ukazatel na TCP hlavi�ku (p��chod paketu) nebo NULL (vol�n� �asova�em)
 **/
char tcp_fsm(struct tcp_h * tcp_header);

/**
 \brief Funkce volan� funkc� ip_recv() po p��chodu TCP datagramu.
 \param ip_header - ukazatel na IP hlavi�ku
 **/
void tcp_recv(struct ip_h* ip_header);

/**
 \endcond
 **/
 
/**
 \brief Ode�le TCP datagram s daty ulo�en�mi v tx bufferu - vol�na u�ivatelem.
 \retval 0 - datagram odesl�n
 \retval -1 - chyba p�i odes�l�n�
 **/
char tcp_send();

/**
 \cond
 **/
 
/**
 \brief Ode�le TCP datagram - vol�na kone�n�m automatem, intern� funkce.
 \param seq - sekven�n� ��slo
 \param ack - potvrzovac� ��slo
 \param flags - p��znaky TCP paketu
 \retval 0 - datagram odesl�n
 \retval -1 - chyba p�i odes�l�n�
 **/
char tcp_out(unsigned long seq, unsigned long ack, unsigned char flags);

/**
 \brief Vypo��t� 16-bitov� checksum z TCP datagramu - intern� funkce.
 \param dest_ip - c�lov� IP adresa
 \param length - d�lka dat
 \returns kontroln� sou�et
 **/
unsigned int tcp_checksum(unsigned long dest_ip,unsigned int length);

/**
 \brief Ode�le pr�zdn� TCP datagram,pouze hlavi�ku - vol�na kone�n�m automatem, intern� funkce.
 \param flags - p��znaky TCP paketu
 \retval 0 - datagram odesl�n
 \retval -1 - chyba p�i odes�l�n�
 **/
char tcp_send_header(unsigned char flags);

/**
 \brief Reset kone�n�ho automatu
 **/
void tcp_fsm_reset();

/**
 \endcond
 **/
 
/**
 \brief Vytvo�� TCP socket, zaregistruje TCP handler k portu \a src_port.
 		\n je mo�n� vytvo�it pouze jeden TCP socket.
 \param src_port - port socketu
 \param handler - Callback handler (TCP_APP_HANDLER()), funkce, kter� bude vol�na p�i p��chodu TCP datagramu na port socketu. 
 \retval 0 - chyba, neplatn� argumenty nebo nen� voln� socket
 \retval jinak - ��slo registrovan�ho portu
 **/
unsigned char create_tcp_socket(unsigned int src_port, void* handler);

/**
 \brief Odstran� TCP socket.
 \retval 0 - socket odstran�n
 \retval -1 - chyba, socket je pou��v�n
 **/
char delete_tcp_socket();

/**
 \brief Zm�n� TCP handler v socketu.
 \param new_handler - Callback handler (TCP_APP_HANDLER()).
 **/
void change_tcp_handler(void* new_handler);

/**
 \brief P�ipoj� se ke vzd�len�mu po��ta�i.
 \param ip - IP adresa c�lov�ho po��ta�e
 \param port - c�lov� port
 \retval 0 - p�ipojen� se navazuje
 \retval -1 - chyba, socket je ji� p�ipojen
 **/
char tcp_connect(unsigned long ip, unsigned int port);

/**
 \brief Naslouch�n� socketu na portu.
 \retval 1 - socket naslouch� na portu
 \retval 0 - chyba, socket je zanepr�zdn�n
 **/
char tcp_bind();

/**
 \brief Zru�en� naslouch�n� socketu na portu.
 \retval 1 - socket odpojen
 \retval 0 - chyba, socket je zanepr�zdn�n, pravd�podobn� je n�kdo p�ipojen
 **/
char tcp_unbind();

/**
 \brief Ukon�� TCP spojen� (ne zcela, pouze kone�n� automat p�ejde do stavu ukon�en� spojen�)
 **/
inline void tcp_disconnect();

/**
 \brief Vrac� stav spojen�
 \retval 1 - spojen� nav�z�no, lze odes�lat data
 \retval 0 - spojen� nenav�z�no nebo se navazuje
 **/
char tcp_connected();

/**
 \brief Vrac� stav TCP socketu
 \retval 1 - TCP socket je ne�inn�
 \retval 0 - TCP socket je zanepr�zdn�n
 **/
char tcp_is_idle();

/**
 \brief TCP �asova�, funkci je nutn� volat v �asov�ch intervalech pro spr�vnou funkci spojen� (p�epos�l�n� datagram� apod.).
 		\n Doba mezi vol�n�mi �asova�e by m�la b�t nejm�n� 250ms a nem�la p�es�hnout 2 vte�iny.
 **/
inline void tcp_timer();

#endif /*_ENC28J60_TCP_*/
