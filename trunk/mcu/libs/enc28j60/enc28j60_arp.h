/*******************************************************************************
   enc28j60_arp.h: Implementace protokolu ARP.
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
	\file enc28j60_arp.h
	\brief Implementace protokolu ARP.
*/

#ifndef _ENC28J60_ARP_
#define _ENC28J60_ARP_

/**
 \cond
 **/

#define ARP_REQUEST 1		///< kód ARP žádosti
#define ARP_REPLY 2			///< kód ARP odpovìdi

#define ETH_HW_TYPE 1		///< typ linkového protokolu - Ethernet
#define ARP_HEADER_LEN 28	///< délka ARP hlavièky
#define ARP_TABLE_SIZE 6	///< velikost ARP tabulky
#define ARP_RECORD_SIZE 10	///< velikost ARP záznamu (IP + MAC = 10B)
#define MAC_LEN 6			///< délka MAC adresy

/**
 \brief struktura arp_h - ARP hlavièka.
 \struct arp_h
 **/
struct arp_h{
	unsigned int hw_type;				///< typ linkového protokolu
	unsigned int proto_type;			///< protokol vyšší vrstvy
	unsigned char hw_addr_len;			///< délka HW adresy (délka MAC)
	unsigned char proto_addr_len;		///< délka adresy protokolu vyšší vrstvy
	unsigned int operation;				///< typ operace
	unsigned char sender_mac[MAC_LEN];	///< MAC odesílatele
	unsigned long sender_ip;			///< IP odesílatele
	unsigned char target_mac[MAC_LEN];	///< MAC cíle
	unsigned long target_ip; 			///< IP cíle
};

/**
 \brief struktura arp_record - ARP záznam, øádek ARP tabulky.
 \struct arp_record
 **/
struct arp_record{
	unsigned int ip;				///< IP adresa
	unsigned char mac[MAC_LEN];		///< MAC adresa
};


/**
 \brief Funkce volaná pøi pøíchodu ARP datagramu.
 **/
void arp_recv();

/**
 \endcond
 **/

/**
 \brief Vymaže ARP tabulku
 **/
void arp_table_clear();

/**
 \cond
 **/

/**
 \brief Odešle ARP dotaz.
 \param dest_ip - IP adresa cíle, jehož MAC chceme zjistit
 **/
char arp_send(unsigned long dest_ip);

/**
 \endcond
 **/

/**
 \brief Pøidá záznam do ARP tabulky.
 \param ip - IP adresa
 \param mac - MAC adresa
 **/
void arp_table_add(int ip,unsigned char* mac);

/**
 \cond
 **/
 
/**
 \brief Hledá MAC adresu v ARP tabulce podle IP adresy cíle.
 \param ip - IP adresa, ke které hledáme MAC
 \retval NULL - záznam nenelezen 
 \retval jinak - MAC adresa
 **/
unsigned char* arp_table_find(int ip);

/**
 \endcond
 **/
 
#endif /*_ENC28J60_ARP_*/
