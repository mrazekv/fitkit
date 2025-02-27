/*******************************************************************************
   enc28j60_eth.h: Linková vrstva modelu TCP/IP
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
	\file enc28j60_eth.h
	\brief Linková vrstva modelu TCP/IP. Pøíjem a odesílání Ethernetových rámcù.
*/

/**
 \cond
 **/

#ifndef _ENC28J60_ETH_
#define _ENC28J60_ETH_

#define ETH_PROTO_ARP 0x0806	///< typ protokolu vyšší vrstvy - ARP
#define ETH_PROTO_IP  0x0800	///< typ protokolu vyšší vrstvy - IP

#define ETH_HEADER_LEN 14		///<délka ethernetové hlavièky

#define MAC_LEN 6				///<délka MAC adresy

#define CTRL_BYTE 0x00		///<hodnota bytu s nastavením detailu odesílání (padding, CRC atd.)

/**
	\brief struktura eth_h - Ethernetová hlavièka
	\struct eth_h
 **/
struct eth_h{
		unsigned char dest_addr[MAC_LEN];	///< MAC adresa cíle
		unsigned char src_addr[MAC_LEN];	///< MAC adresa odesílatele
		unsigned int type_len;				///< protokol vyšší vrstvy
};

/**
 \endcond
 **/

/**
 \brief Obsluha øadièe ENC28J60. Musí být volána v nekoneèné smyèce.
 **/
void ENC28J60_idle();

/**
 \cond
 **/

/**
 \brief Funkce volaná funkcí ENC28J60_idle() pøi pøíchodu nového rámce
 **/
void eth_recv();


/**
 \brief Odešle Ethernetový rámec
 \param dest_addr - ukazatel na MAC adresu cíle (pole 6 char)
 \param type - typ protokolu vyšší vrstvy 
 **/
char eth_send(unsigned char* dest_addr,unsigned int type);

/**
 \endcond
 **/

/**
 \brief Nastaví lokální MAC adresu (musí být volána pøed inicializací øadièe - pøed voláním \a ENC28J60_init() )
 		\n jinak se použije dafaultní MAC 01:24:8C:6B:38:64
 \param mac_ptr - ukazatel na MAC adresu (pole 6ti char)
 \retval 0 - MAC nelze zmìnit, ENC28J60 je již inicializován
 \retval 1 - zmìna probìhla úspìšnì
 **/
inline char set_mac(unsigned char* mac_ptr);



#endif /*_ENC28J60_ETH_*/
