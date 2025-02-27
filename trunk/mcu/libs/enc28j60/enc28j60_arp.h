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

#define ARP_REQUEST 1		///< k�d ARP ��dosti
#define ARP_REPLY 2			///< k�d ARP odpov�di

#define ETH_HW_TYPE 1		///< typ linkov�ho protokolu - Ethernet
#define ARP_HEADER_LEN 28	///< d�lka ARP hlavi�ky
#define ARP_TABLE_SIZE 6	///< velikost ARP tabulky
#define ARP_RECORD_SIZE 10	///< velikost ARP z�znamu (IP + MAC = 10B)
#define MAC_LEN 6			///< d�lka MAC adresy

/**
 \brief struktura arp_h - ARP hlavi�ka.
 \struct arp_h
 **/
struct arp_h{
	unsigned int hw_type;				///< typ linkov�ho protokolu
	unsigned int proto_type;			///< protokol vy��� vrstvy
	unsigned char hw_addr_len;			///< d�lka HW adresy (d�lka MAC)
	unsigned char proto_addr_len;		///< d�lka adresy protokolu vy��� vrstvy
	unsigned int operation;				///< typ operace
	unsigned char sender_mac[MAC_LEN];	///< MAC odes�latele
	unsigned long sender_ip;			///< IP odes�latele
	unsigned char target_mac[MAC_LEN];	///< MAC c�le
	unsigned long target_ip; 			///< IP c�le
};

/**
 \brief struktura arp_record - ARP z�znam, ��dek ARP tabulky.
 \struct arp_record
 **/
struct arp_record{
	unsigned int ip;				///< IP adresa
	unsigned char mac[MAC_LEN];		///< MAC adresa
};


/**
 \brief Funkce volan� p�i p��chodu ARP datagramu.
 **/
void arp_recv();

/**
 \endcond
 **/

/**
 \brief Vyma�e ARP tabulku
 **/
void arp_table_clear();

/**
 \cond
 **/

/**
 \brief Ode�le ARP dotaz.
 \param dest_ip - IP adresa c�le, jeho� MAC chceme zjistit
 **/
char arp_send(unsigned long dest_ip);

/**
 \endcond
 **/

/**
 \brief P�id� z�znam do ARP tabulky.
 \param ip - IP adresa
 \param mac - MAC adresa
 **/
void arp_table_add(int ip,unsigned char* mac);

/**
 \cond
 **/
 
/**
 \brief Hled� MAC adresu v ARP tabulce podle IP adresy c�le.
 \param ip - IP adresa, ke kter� hled�me MAC
 \retval NULL - z�znam nenelezen 
 \retval jinak - MAC adresa
 **/
unsigned char* arp_table_find(int ip);

/**
 \endcond
 **/
 
#endif /*_ENC28J60_ARP_*/
