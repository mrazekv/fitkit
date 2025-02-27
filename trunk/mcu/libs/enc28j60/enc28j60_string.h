/*******************************************************************************
   enc28j60_string.h: Knihovna funkc� pro pr�ci s �et�zci a pam�t�.
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
	\file enc28j60_string.h
	\brief Knihovna funkc� pro pr�ci s �et�zci a pam�t�.
*/

#ifndef _ENC28J60_STRING_
#define _ENC28J60_STRING_

/**
 \brief Makro vrac�, zda je znak \a x des�tkov� ��slice
 \param x kontrolovan� znak
 \retval 1 - znak je ��slic�
 \retval 0 - znak nen� ��slic�
 **/
#define is_digit(x) ( ((x >= '0') & (x <= '9')) ? 1 : 0)

/**
 \brief vlastn� implementace fce MEMCPY
 \param dst - ukazatel na c�l
 \param src - ukazatel na zdroj
 \param count - po�et bajt� ke kop�rov�n�
 **/
inline void* enc_memcpy(void* dst, void* src, unsigned int count);

/**
 \brief vlastn� implementace fce MEMCMP
 \param str1 - ukazatel na prvn� blok dat
 \param str2 - ukazatel na druh� blok dat
 \param count - po�et bajt� k porovn�v�n�
 \retval 0 - bloky jsou stejn�
 \retval jinak - bloky jsou rozd�ln�
 **/
inline unsigned int enc_memcmp(void* str1, void* str2,unsigned int count);


/**
 \brief vlastn� implementace fce MEMSET
 \param ptr - ukazatel na adresu, odkud chceme p�episovat
 \param value - hodnota, kterou bude ka�d� bajt bloku p�episov�n
 \param count - velikost bloku v bajtech
 **/
inline void* enc_memset(void* ptr, char value, unsigned int count);

 
/**
 \brief vlastn� implementace fce STRLEN
 \param ptr - ukazatel na �et�zec
 \returns d�lka �et�zce bez koncov�ho znaku (0)
 **/
inline unsigned int enc_strlen(void* ptr);

/**
 \brief Funkce p�evede �et�zec na ��sla reprezentuj�c� IP adresu a port.
 		\n �et�zec mus� b�t ve form�tu "IP_adresa:port" nebo "IP_adresa". Ve druh�m p��pad� bude navr�cen� ��slo portu 0. 
 \param ptr - ukazatel na �et�zec
 \param max_len - maxim�ln� d�lka �et�zce
 \param ip - vyparsovan� IP adresa
 \param port - vyparsovan� ��slo portu
 \retval 1 - p�evod byl �sp�n�, hodnoty parametr� \a ip a \a port jsou platn�
 \retval 0 - p�evod ne�sp�n�, hodnoty parametr� \a ip a \a port jsou nedefinovan�
 **/
unsigned char str_to_ip_port(char* ptr, char max_len, unsigned long* ip, unsigned int* port);

/**
 \brief Funkce p�evede ��slo reprezentuj�c� IP adresu na �et�zec zakon�en� znakem null.
 \param ptr - ukazatel na buffer, do kter�ho se p�eveden� �et�zec bude zapisovat
 \param max_len - maxim�ln� d�lka bufferu
 \param ip - ��slo reprezentuj�c� IP adresu
 \returns d�lka v�sledn�ho �et�zce (bez znaku null na konci)
 **/
unsigned char ip_to_str(char * ptr, char max_len, unsigned long ip);

#endif /*_ENC28J60_STRING_*/
