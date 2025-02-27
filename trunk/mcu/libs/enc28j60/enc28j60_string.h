/*******************************************************************************
   enc28j60_string.h: Knihovna funkcí pro práci s øetìzci a pamìtí.
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
	\brief Knihovna funkcí pro práci s øetìzci a pamìtí.
*/

#ifndef _ENC28J60_STRING_
#define _ENC28J60_STRING_

/**
 \brief Makro vrací, zda je znak \a x desítková èíslice
 \param x kontrolovaný znak
 \retval 1 - znak je èíslicí
 \retval 0 - znak není èíslicí
 **/
#define is_digit(x) ( ((x >= '0') & (x <= '9')) ? 1 : 0)

/**
 \brief vlastní implementace fce MEMCPY
 \param dst - ukazatel na cíl
 \param src - ukazatel na zdroj
 \param count - poèet bajtù ke kopírování
 **/
inline void* enc_memcpy(void* dst, void* src, unsigned int count);

/**
 \brief vlastní implementace fce MEMCMP
 \param str1 - ukazatel na první blok dat
 \param str2 - ukazatel na druhý blok dat
 \param count - poèet bajtù k porovnávání
 \retval 0 - bloky jsou stejné
 \retval jinak - bloky jsou rozdílné
 **/
inline unsigned int enc_memcmp(void* str1, void* str2,unsigned int count);


/**
 \brief vlastní implementace fce MEMSET
 \param ptr - ukazatel na adresu, odkud chceme pøepisovat
 \param value - hodnota, kterou bude každý bajt bloku pøepisován
 \param count - velikost bloku v bajtech
 **/
inline void* enc_memset(void* ptr, char value, unsigned int count);

 
/**
 \brief vlastní implementace fce STRLEN
 \param ptr - ukazatel na øetìzec
 \returns délka øetìzce bez koncového znaku (0)
 **/
inline unsigned int enc_strlen(void* ptr);

/**
 \brief Funkce pøevede øetìzec na èísla reprezentující IP adresu a port.
 		\n Øetìzec musí být ve formátu "IP_adresa:port" nebo "IP_adresa". Ve druhém pøípadì bude navrácené èíslo portu 0. 
 \param ptr - ukazatel na øetìzec
 \param max_len - maximální délka øetìzce
 \param ip - vyparsovaná IP adresa
 \param port - vyparsované èíslo portu
 \retval 1 - pøevod byl úspìšný, hodnoty parametrù \a ip a \a port jsou platné
 \retval 0 - pøevod neúspìšný, hodnoty parametrù \a ip a \a port jsou nedefinované
 **/
unsigned char str_to_ip_port(char* ptr, char max_len, unsigned long* ip, unsigned int* port);

/**
 \brief Funkce pøevede èíslo reprezentující IP adresu na øetìzec zakonèený znakem null.
 \param ptr - ukazatel na buffer, do kterého se pøevedený øetìzec bude zapisovat
 \param max_len - maximální délka bufferu
 \param ip - èíslo reprezentující IP adresu
 \returns délka výsledného øetìzce (bez znaku null na konci)
 **/
unsigned char ip_to_str(char * ptr, char max_len, unsigned long ip);

#endif /*_ENC28J60_STRING_*/
