/*******************************************************************************
   enc28j60_udp.h: Transportní vrstva modelu TCP/IP, protokol UDP
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
	\file enc28j60_udp.h
	\brief Transportní vrstva modelu TCP/IP. Pøíjem a odesílání UDP datagramù.
*/

#ifndef _ENC28J60_UDP_
#define _ENC28J60_UDP_

#include "enc28j60_ip.h"


#define UDP_HEADER_LEN 8	///< délka UDP hlavièky

/**
 \brief struktura udp_h - UDP hlavièka.
 \struct udp_h
 **/
struct udp_h{
	unsigned int src_port;		///< zdrojový port
	unsigned int dest_port;		///< cílový port
	unsigned int length;		///< délka hlavièky + dat v bytech
	unsigned int checksum;		///< kontrolní souèet z hlavièky i dat
};




#define SOCKET_COUNT 6	///< poèet UDP socketù

///deklarace UDP callback handleru
#define UDP_APP_HANDLER(x) void (x)(struct ip_h *ip_header, struct udp_h *udp_header)

/**
 \cond
 **/

/**
 \brief struktura udp_socket - UDP socket.
 \struct udp_socket
 **/
struct udp_socket{
	unsigned int port;					///< èíslo portu
	UDP_APP_HANDLER(*app_handler);		///< Callback handler, funkce, která je volána po pøíchodu datagramu na \a port
};

/**
 \brief Funkce volaná funkcí ip_recv() po pøíchodu UDP datagramu.
 \param ip_header - ukazatel na IP hlavièku
 **/
void udp_recv(struct ip_h* ip_header);

/**
 \endcond
 **/
 
/**
 \brief Odešle UDP datagram s daty uloženými v tx bufferu.
 \param dest_port - cílový port
 \param src_port - zdrojový port
 \param ip - IP adresa cíle
 \retval 0 - datagram odeslán
 \retval -1 - chyba pøi odesílání
 \retval -2 - ARP protokol nezjistil MAC adresu cíle, neodesláno
 **/
char udp_send(unsigned int dest_port,unsigned int src_port, unsigned long ip);

/**
 \cond
 **/
 
/**
 \brief Vypoèítá 16-bitový checksum z UDP datagramu - interní funkce.
 \param dest_ip - cílová IP adresa
 \param length - délka dat
 \returns kontrolní souèet
 **/
unsigned int udp_checksum(unsigned long dest_ip,int length);

/**
 \endcond
 **/

/**
 \brief Zaregistruje handler k èíslu portu.
 \param port - èíslo portu
 \param handler - Callback handler, funkce, která bude volána pøi pøíchodu UDP datagramu k pøíslušnému portu.
 \retval 0 - chyba, neplatné argumenty nebo není volný socket
 \retval jinak - èíslo registrovaného portu
 **/
unsigned int udp_bind(unsigned int port, void* handler);

/**
 \brief Odregistruje Callback handler od èísla portu.
 \param port - èíslo portu
 **/
void udp_unbind(unsigned int port);

#endif /*_ENC28J60_UDP_*/
