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

/**
	\file enc28j60_icmp.h
	\brief Implementace protokolu ICMP, umožòuje pøijímat a odesílat ICMP datagramy.
*/

#ifndef _ENC28J60_ICMP_
#define _ENC28J60_ICMP_

#include "enc28j60_ip.h"

/**
 \cond
 **/
#define ICMP_ECHO_REPLY		0
#define ICMP_ECHO			0x08
#define ICMP_ECHO_CODE		0
/**
 \endcond
 **/

/**
 \brief struktura icmp_h - ICMP hlavièka.
 \struct icmp_h
 **/
struct icmp_h{
	unsigned char type;			///< Typ ICMP zprávy
	unsigned char code;			///< Kód, rozšíøení specifikace typu zprávy
	unsigned int checksum;		///< Kontrolní souèet z hlavièky a dat
	unsigned int id;			///< Identifikátor datagramu
	unsigned int seq;			///< sekvenèní èíslo ICMP datagramu.
};


///deklarace ICMP callback handleru
#define ICMP_APP_HANDLER(x) void (x)(struct ip_h *ip_header, struct icmp_h *icmp_header)

/**
 \cond
 **/

/**
 \brief Funkce volaná funkcí ip_recv() po pøíchodu ICMP datagramu.
 \param ip_header - ukazatel na IP hlavièku.
 **/
void icmp_recv(struct ip_h* ip_header);

/**
 \endcond
 **/

/**
 \brief Odešle ICMP datagram s daty uloženými v tx bufferu.
 \param ip - cílová IP adresa
 \param type - Typ ICMP zprávy
 \param code - Kód (podle typu zprávy)
 \param id - Identifikátor ICMP datagramu
 \param sequence - Sekvenèní èíslo datagramu
 \retval 0 - datagram odeslán
 \retval -1 - chyba pøi odesílání
 \retval -2 - ARP protokol nezjistil MAC adresu cíle, neodesláno
 **/
char icmp_send(unsigned long ip, unsigned char type, unsigned char code, unsigned int id, unsigned int sequence );

/**
 \brief Pøepne automatickou odpovìï na ICMP zprávy na manuální. ICMP komunikace bude zcela v režii programátora.
 \param handler - Callback handler, funkce bude volána pøi pøíchodu ICMP datagramu.
 **/
void icmp_bind(void* handler);

/**
 \brief Pøepne manuální odpovìï na ICMP zprávy zpìt na automatickou.
 **/
void icmp_unbind();
#endif /*_ENC28J60_ICMP_*/

