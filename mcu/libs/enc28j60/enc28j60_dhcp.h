/*******************************************************************************
   enc28j60_dhcp.h: Implementace protokolu DHCP.
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
	\file enc28j60_dhcp.h
	\brief Implementace protokolu DHCP.
*/

#ifndef _ENC28J60_DHCP_
#define _ENC28J60_DHCP_

/**
 \cond
 **/

#define BOOTP_HEADER_LEN 44			///< Délka struktury bootp_h
#define BOOTP_HEADER_LEN_TOTAL 236	///< Calková délka hlavièky protokolu BOOTP
 
/**
	\brief struktura bootp_h - Hlavièka protokolu BOOTP. (nekompletní, pouze nutné èásti)
	\struct bootp_h
 **/
 
struct bootp_h{
	unsigned char opcode;					///< kód operace (request/reply)
	unsigned char hw_type;					///< typ HW adresy
	unsigned char hw_address_length;		///< délka HW adresy
	unsigned char hop_count;
	unsigned long transaction_id;			///< ID transakce
	unsigned int seconds;
	unsigned int flags;
	unsigned long client_ip;
	unsigned long your_ip;					///< pøidìlená IP adresa
	unsigned long server_ip;
	unsigned long gateway_ip;
	unsigned char client_hw_address[16];	///< HW adresa klienta
};


#define BOOTP_REQUEST 1
#define BOOTP_REPLY 2

#define BOOTP_HW_ETH 1

#define BOOTP_FLAG_BROADCAST 0x8000

//DHCP porty
#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define MAGIC_LEN 4
#define MAGIC_COOKIE 0x63538263

//parametry DHCP
#define DHCP_PAD 0
#define DHCP_SUBNET_MASK 1
#define DHCP_ROUTER 3
#define DHCP_DNS_SERVER 6
#define DHCP_REQUESTED_IP 50
#define DHCP_MESSAGE_TYPE 53
#define DHCP_SERVER_ID 54
#define DHCP_END 255

//DHCP typy zprav
#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5
#define DHCP_NACK 6
#define DHCP_RELEASE 7

#define DHCP_START 0

#define DHCP_TIMEOUT	30			///< jednoduchý timeout - pro reset dhcp klienta, když server neodpovídá
/**
 \endcond
 **/
 
/**
 \brief Inicializace DHCP klienta.
 \retval 0 - chyba, není volný UDP socket
 \retval jinak - inicializace se zdaøila
 **/
char dhcp_init();

/**
 \brief Restart DHCP klienta.
 \retval 0 - chyba, není volný UDP socket
 \retval jinak - inicializace se zdaøila
 **/
char dhcp_reset();

/**
 \brief DHCP èasovaè, funkci je nutné volat v èasových intervalech pro správnou funkci DHCP.
 		\n Doba mezi voláními èasovaèe by mìla být mezi 500 a 1000ms.
 **/
void dhcp_timer();

/**
 \brief Vrátí stav DHCP.
 \retval 0 - IP adresa nepøidìlena
 \retval 1 - IP adresa pøidìlena, lze odesílat a pøijímat data
 **/
char dhcp_ready();

#define TIMERA_ISR_INIT() /* inicializace casovace, 500ms */ \
   { CCTL0 = CCIE;	/* povoleni preruseni pro casovac, rezim vystupni komparace	 */ \
	  CCR0 = 0x4000;	   /* 16384 = 0x4000; preruseni kazdych 500ms */ \
	  TACTL = TASSEL_1 + MC_2; } /* ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim */ \

#define TIMERA_ISR_DHCP() \
  interrupt (TIMERA0_VECTOR) Timer_A (void) { \
     dhcp_timer();		/*dhcp casovac*/ \
	  CCR0 += 0x4000;	  /*preruseni kazdych 500ms*/ \
  }

#define TIMERA_ISR_DHCP_TCP() \
  interrupt (TIMERA0_VECTOR) Timer_A (void) { \
     dhcp_timer();	  /*dhcp casovac*/ \
     tcp_timer();		  /*tcp casovac*/ \
	  CCR0 += 0x4000;	  /*preruseni kazdych 500ms*/ \
  }

#endif /*_ENC28J60_DHCP_*/
