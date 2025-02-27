/*******************************************************************************
   enc28j60_ip.h: Sitova vrstva modelu TCP/IP
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
	\file enc28j60_ip.h
	\brief S�ov� vrstva modelu TCP/IP. P��jem a odes�l�n� IP paket�.
*/

#ifndef _ENC28J60_IP_
#define _ENC28J60_IP_


//****************************
//prevody
//****************************
/// little na big endian(na s�ti), pro int(16b)
#define HTONS(x) ((((x)&0xff)<<8) | ((x)>>8))
/// little na big endian(na s�ti), pro long(32b)
#define HTONL(x) ((((x)&0xff)<<24) | (((x)&0x00ff0000l)>>8) | (((x)&0x0000ff00l)<<8) | ((x)>>24))

#define NTOHS HTONS	///< big(na s�ti) na little endian, pro int(16b)
#define NTOHL HTONL	///< big(na s�ti) na little endian, pro long(32b)

/**
 \brief Makro pro p�evod 4xchar na IP (unsigned long)
 **/
#define CHAR2IP(x0,x1,x2,x3) ( ((unsigned long)x0<<24)|((unsigned long)x1<<16)|((unsigned int)x2<<8)|x3)

/**
 \cond
 **/

#define IP_LEN 4			///< d�lka IP adresy
#define DEFAULT_TTL 128		///< defaultn� TTL
#define MIN_IP_LEN 60 - ETH_HEADER_LEN	///< minim�ln� d�lka IP paketu

#define IPv4 4				///< k�d protokolu IPv4
 
#define IP_FRAG_OFFSET   0x1fff
#define IP_FRAG_MORE     0x2000
#define IP_FRAG_DONT     0x4000
#define IP_FRAG_RESERVED 0x8000

#define IP_BROADCAST 0xFFFFFFFFL		///< broadcastov� IP adresa


#define IP_PROTO_ICMP 1					///< protokol vy��� vrstvy - ICMP
#define IP_PROTO_TCP  6					///< protokol vy��� vrstvy - TCP
#define IP_PROTO_UDP 17					///< protokol vy��� vrstvy - UDP

#define ARP_TIMEOUT 12000				///< max. doba �ek�n� na ARP odpov��

/**
 \endcond
 **/

/**
 \brief struktura ip_h - IP hlavi�ka.
 	\n pozor, bitov� polo�ky se mus� ukl�dat v "opa�n�m po�ad�" (kv�li big endianu na s�ti)
 \struct ip_h
 **/
struct ip_h{
	unsigned char header_len:4;		///< d�lka hlavi�ky v double-wordech (4B)
	unsigned char ipv:4;			///< verze IP protokolu
	unsigned char tos;				///< typ slu�by
	unsigned int length;			///< d�lka datagramu
	unsigned int identification;	///< identifikace paketu
	unsigned int fragment;			///< offset fragmentu a flagy
	unsigned char ttl;				///< Time to Live
	unsigned char protocol;			///< protokol vy��� vrstvy
	unsigned int checksum;			///< kontroln� sou�et z hlavi�ky
	unsigned long src_ip;			///< IP odes�latele
	unsigned long dest_ip;			///< IP p��jemce
};

/**
 \cond
 **/

/******************/

/**
 \brief Funkce volan� funkc� eth_recv() p�i p��chodu IP paketu.
 **/
void ip_recv(void);

/**
 \brief Ode�le IP paket.
 \param dest_addr - IP adresa c�le
 \param protocol - protokol vy��� vrstvy
 \retval 0 - odesl�no
 \retval -1 - chyba p�i odesl�n�
 \retval -2 - ARP protokol nezjistil MAC adresu c�le, neodesl�no
 **/
char ip_send(unsigned long dest_addr,unsigned char protocol);

/**
 \brief Vypo��t� 16-bitov� checksum z bloku dat
 \param pointer - ukazatel na blok dat
 \param count - d�lka bloku v bajtech
 \returns checksum 
 **/
unsigned int count_checksum(void* pointer,int count);

/**
 \endcond
 **/
/******************/

/**
 \brief Nastaven� lok�ln� IP
 \param a0,a1,a2,a3 - bajty IP adresy, nap�. 192.168.1.2
 \see set_ip_l()
 **/
inline void set_ip(unsigned char a0,unsigned char a1,unsigned char a2, unsigned char a3);

/**
 \brief Nastaven� lok�ln� IP
 \param ip - IP adresa, nap� 0xC0800102
 \see set_ip()
 **/
inline void set_ip_l(unsigned long ip);

/**
 \brief Vr�t� lok�ln� IP adresu
 \returns lok�ln� IP adresa, nap� 0xC0800102
 **/
inline unsigned long get_ip();


/******************/

/**
 \brief Nastaven� masky
 \param m0,m1,m2,m3 - bajty IP adresy, nap�. 255.255.255.0
 \see set_netmask_l()
 **/
inline void set_netmask(unsigned char m0,unsigned char m1,unsigned char m2, unsigned char m3);

/**
 \brief Nastaven� masky
 \param netmask - maska, nap�. 0xFFFFFF00
 \see set_netmask()
 **/
inline void set_netmask_l(unsigned long netmask);

/**
 \brief Vr�t� masku pods�t�
 \returns maska, nap� 0xFFFFFF00
 **/
inline unsigned long get_netmask();



/******************/


/**
 \brief Nastaven� IP br�ny
 \param g0,g1,g2,g3 - bajty IP adresy, nap�. 192.168.1.2
 \see set_gateway_l()
 **/
inline void set_gateway(unsigned char g0,unsigned char g1,unsigned char g2, unsigned char g3);

/**
 \brief Nastaven� IP br�ny
 \param gateway - IP br�ny, nap� 0xC0800102
 \see set_gateway()
 **/
inline void set_gateway_l(unsigned long gateway);

/**
 \brief Vr�t� IP adresu br�ny
 \returns IP adresa br�ny, nap� 0xC0800102
 **/
inline unsigned long get_gateway();


/******************/

/**
 \brief Nastaven� IP DNS serveru
 \param d0,d1,d2,d3 - bajty IP adresy, nap�. 192.168.1.2
 \see set_dns_server_l()
 **/
inline void set_dns_server(unsigned char d0,unsigned char d1,unsigned char d2, unsigned char d3);

/**
 \brief Nastaven� IP DNS serveru
 \param dns - IP DNS serveru, nap� 0xC0800102
 \see set_dns_server()
 **/
inline void set_dns_server_l(unsigned long dns);

/**
 \brief Vr�t� IP adresu DNS
 \returns IP adresa DNS, nap� 0xC0800102
 **/
inline unsigned long get_dns_server();



#endif /*_ENC28J60_IP_*/
