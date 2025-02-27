/*******************************************************************************
   enc28j60_eth.h: Linkov� vrstva modelu TCP/IP
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
	\brief Linkov� vrstva modelu TCP/IP. P��jem a odes�l�n� Ethernetov�ch r�mc�.
*/

/**
 \cond
 **/

#ifndef _ENC28J60_ETH_
#define _ENC28J60_ETH_

#define ETH_PROTO_ARP 0x0806	///< typ protokolu vy��� vrstvy - ARP
#define ETH_PROTO_IP  0x0800	///< typ protokolu vy��� vrstvy - IP

#define ETH_HEADER_LEN 14		///<d�lka ethernetov� hlavi�ky

#define MAC_LEN 6				///<d�lka MAC adresy

#define CTRL_BYTE 0x00		///<hodnota bytu s nastaven�m detailu odes�l�n� (padding, CRC atd.)

/**
	\brief struktura eth_h - Ethernetov� hlavi�ka
	\struct eth_h
 **/
struct eth_h{
		unsigned char dest_addr[MAC_LEN];	///< MAC adresa c�le
		unsigned char src_addr[MAC_LEN];	///< MAC adresa odes�latele
		unsigned int type_len;				///< protokol vy��� vrstvy
};

/**
 \endcond
 **/

/**
 \brief Obsluha �adi�e ENC28J60. Mus� b�t vol�na v nekone�n� smy�ce.
 **/
void ENC28J60_idle();

/**
 \cond
 **/

/**
 \brief Funkce volan� funkc� ENC28J60_idle() p�i p��chodu nov�ho r�mce
 **/
void eth_recv();


/**
 \brief Ode�le Ethernetov� r�mec
 \param dest_addr - ukazatel na MAC adresu c�le (pole 6 char)
 \param type - typ protokolu vy��� vrstvy 
 **/
char eth_send(unsigned char* dest_addr,unsigned int type);

/**
 \endcond
 **/

/**
 \brief Nastav� lok�ln� MAC adresu (mus� b�t vol�na p�ed inicializac� �adi�e - p�ed vol�n�m \a ENC28J60_init() )
 		\n jinak se pou�ije dafaultn� MAC 01:24:8C:6B:38:64
 \param mac_ptr - ukazatel na MAC adresu (pole 6ti char)
 \retval 0 - MAC nelze zm�nit, ENC28J60 je ji� inicializov�n
 \retval 1 - zm�na prob�hla �sp�n�
 **/
inline char set_mac(unsigned char* mac_ptr);



#endif /*_ENC28J60_ETH_*/
