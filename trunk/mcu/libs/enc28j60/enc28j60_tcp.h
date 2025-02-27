/*******************************************************************************
   enc28j60_tcp.h: Transportní vrstva modelu TCP/IP
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
	\brief Transportní vrstva modelu TCP/IP. Pøíjem a odesílání TCP datagramù.
*/

#ifndef _ENC28J60_TCP_
#define _ENC28J60_TCP_

#include "enc28j60_ip.h"
 
#define TCP_HEADER_LEN	20		///< velikost TCP hlavièky

/**
 \cond
 **/
#define WINDOW_SIZE		1460	///< šíøka okna

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

	
#define SYN_TIMEOUT			40		//pri volani tcp_timer 2x/s odpovida 20s - zkráceno
#define TIME_WAIT_TIMEOUT	5		//zkraceno, mensi cekani po skonceni -> rychleji se uvolni TCP spoj
#define FIN_WAIT1_TIMEOUT	40
#define FIN_TIMEOUT			15
#define CLOSE_WAIT_TIMEOUT	40
#define MAX_RETRANSMIT		40


/**
 \endcond
 **/
 
/**
 \brief struktura tcp_h - TCP hlavièka.
 \struct tcp_h
 **/
struct tcp_h{
	unsigned int src_port;			///< zdrojový port
	unsigned int dest_port;			///< cílový port
	unsigned long seq_num;			///< sekvenèní èíslo paketu
	unsigned long ack_num;			///< potvrzovací èíslo
	unsigned char reserved:4;		///< rezervováno
	unsigned char data_offset:4;	///< velikost hlavièky v DOUBLEWORDech
	unsigned char flags;			///< TCP pøíznaky
	unsigned int window_size;		///< velikost okna
	unsigned int checksum;			///< kontrolní souèet
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
	unsigned int src_port;			///< zdrojový port
	unsigned int dest_port;			///< cílový port
	unsigned long dest_ip;			///< IP adresa cíle
	TCP_APP_HANDLER(*app_handler);	///< handler funkce, která je volána po pøíchodu datagramu na \a src_port
	unsigned char state;			///< stav TCP spojení (stavy koneèného automatu)
	unsigned char type;				///< typ socketu (klient/server)
};

/**
 \brief Funkce realizující TCP koneèný automat, volána buï pøi pøíchodu TCP datagramu, nebo èasovaèem(zajišuje pøeposílání nepotvrzených dat). 
 \param tcp_header - ukazatel na TCP hlavièku (pøíchod paketu) nebo NULL (volání èasovaèem)
 **/
char tcp_fsm(struct tcp_h * tcp_header);

/**
 \brief Funkce volaná funkcí ip_recv() po pøíchodu TCP datagramu.
 \param ip_header - ukazatel na IP hlavièku
 **/
void tcp_recv(struct ip_h* ip_header);

/**
 \endcond
 **/
 
/**
 \brief Odešle TCP datagram s daty uloženými v tx bufferu - volána uživatelem.
 \retval 0 - datagram odeslán
 \retval -1 - chyba pøi odesílání
 **/
char tcp_send();

/**
 \cond
 **/
 
/**
 \brief Odešle TCP datagram - volána koneèným automatem, interní funkce.
 \param seq - sekvenèní èíslo
 \param ack - potvrzovací èíslo
 \param flags - pøíznaky TCP paketu
 \retval 0 - datagram odeslán
 \retval -1 - chyba pøi odesílání
 **/
char tcp_out(unsigned long seq, unsigned long ack, unsigned char flags);

/**
 \brief Vypoèítá 16-bitový checksum z TCP datagramu - interní funkce.
 \param dest_ip - cílová IP adresa
 \param length - délka dat
 \returns kontrolní souèet
 **/
unsigned int tcp_checksum(unsigned long dest_ip,unsigned int length);

/**
 \brief Odešle prázdný TCP datagram,pouze hlavièku - volána koneèným automatem, interní funkce.
 \param flags - pøíznaky TCP paketu
 \retval 0 - datagram odeslán
 \retval -1 - chyba pøi odesílání
 **/
char tcp_send_header(unsigned char flags);

/**
 \brief Reset koneèného automatu
 **/
void tcp_fsm_reset();

/**
 \endcond
 **/
 
/**
 \brief Vytvoøí TCP socket, zaregistruje TCP handler k portu \a src_port.
 		\n je možné vytvoøit pouze jeden TCP socket.
 \param src_port - port socketu
 \param handler - Callback handler (TCP_APP_HANDLER()), funkce, která bude volána pøi pøíchodu TCP datagramu na port socketu. 
 \retval 0 - chyba, neplatné argumenty nebo není volný socket
 \retval jinak - èíslo registrovaného portu
 **/
unsigned char create_tcp_socket(unsigned int src_port, void* handler);

/**
 \brief Odstraní TCP socket.
 \retval 0 - socket odstranìn
 \retval -1 - chyba, socket je používán
 **/
char delete_tcp_socket();

/**
 \brief Zmìní TCP handler v socketu.
 \param new_handler - Callback handler (TCP_APP_HANDLER()).
 **/
void change_tcp_handler(void* new_handler);

/**
 \brief Pøipojí se ke vzdálenému poèítaèi.
 \param ip - IP adresa cílového poèítaèe
 \param port - cílový port
 \retval 0 - pøipojení se navazuje
 \retval -1 - chyba, socket je již pøipojen
 **/
char tcp_connect(unsigned long ip, unsigned int port);

/**
 \brief Naslouchání socketu na portu.
 \retval 1 - socket naslouchá na portu
 \retval 0 - chyba, socket je zaneprázdnìn
 **/
char tcp_bind();

/**
 \brief Zrušení naslouchání socketu na portu.
 \retval 1 - socket odpojen
 \retval 0 - chyba, socket je zaneprázdnìn, pravdìpodobnì je nìkdo pøipojen
 **/
char tcp_unbind();

/**
 \brief Ukonèí TCP spojení (ne zcela, pouze koneèný automat pøejde do stavu ukonèení spojení)
 **/
inline void tcp_disconnect();

/**
 \brief Vrací stav spojení
 \retval 1 - spojení navázáno, lze odesílat data
 \retval 0 - spojení nenavázáno nebo se navazuje
 **/
char tcp_connected();

/**
 \brief Vrací stav TCP socketu
 \retval 1 - TCP socket je neèinný
 \retval 0 - TCP socket je zaneprázdnìn
 **/
char tcp_is_idle();

/**
 \brief TCP èasovaè, funkci je nutné volat v èasových intervalech pro správnou funkci spojení (pøeposílání datagramù apod.).
 		\n Doba mezi voláními èasovaèe by mìla být nejménì 250ms a nemìla pøesáhnout 2 vteøiny.
 **/
inline void tcp_timer();

#endif /*_ENC28J60_TCP_*/
