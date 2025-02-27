/*******************************************************************************
   globfunc.h:
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

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

   Popis:
     Definice globalnich funkci.
*******************************************************************************/

#ifndef _GLOBFUN_H_
#define _GLOBFUN_H_


/**
 \brief Fce vrátí binarní podobu znaku 'chr'.
        nejvyznamnìjší bit na pozici nula
 \param buf - buffer, který ma alokovaných alespoò 9 bytù
 \param chr - pøevádìný znak
 **/
void chr2bin(unsigned char *buf, unsigned char chr);

/**
 \brief Fce chr2hex() vrátí hexa podobu znaku 'chr'.
    - nejvýznamnìjší bit na pozici nula
 \param buf - buffer, který má alokovaných alespoò 9 bytu
 \param chr - pøevádìný znak
 **/
void chr2hex(unsigned char *buf, unsigned char chr);

/**
 \brief Fce vrátí èíselnou reprezentaci znaku '1' -> 1.
 \param chr - pøevádìný znak
 \return èislo (digit)
 **/
unsigned char chr2digit(unsigned char chr);

/**
 \brief Fce vrátí znak v jeho èíselné reprezentaci 1 -> '1'.
 \param chr - pøevádìné èíslo
 \return znak
 **/
char digit2chr(char chr);

/**
 \brief Fce obratí poøadí bitù ve znaku 'chr'.
 \param chr - pøevádìný znak
 \return znak s obráceným poøadím bitù
 **/
unsigned char rchr(unsigned char chr);

/*******************************************************************************
 * Fce prevadi long na string v hexa.
 * - input...prevadena hodnota
 * - buf.....vysledny retezec
*******************************************************************************/
//void long2strhex(long input, char *str);

/**
 \brief Fce pøevádí string na long.
 \param  buf - vstupní øetìzec
 \return long èíslo (32 bitu)
 **/
long str2long(char *buf);

/**
 \brief Fce pøevádí string na 2x long.
 \param buf - vstupní øetezec obsahující 2 èísla oddìlená mezerou.
 \param n1 - ukazatel k uložení 1. èísla
 \param n2 - ukazatel k uložení 2. èísla
 \return První nalezené èíslo v øetìzci (=n1)
 **/
long str2long_2x(char *buf, long *n1, long *n2);

/**
 \brief Fce pøevrací øetìzec (znaky v opaèném sledu).
 \param buf - vstupní øetìzec
 \return pøevrácený øetìzec
 **/
char *strrev(char *buf);

/**
 \brief Pøevede øetìzec na velká písmena
 \param dest - vystupni buffer
 \param buf - vstupni string
 **/
void strupper(unsigned char *dest, unsigned char *buf);

/**
 \brief Fce pøevádí long na string.
 \param buf - výtupní øetìzec
 \param input - long èíslo
 \param radix - maximalní hodnota 1. digitu (u hexa 16, dec 10 ...)
 **/
void long2str(long input, char *buf, long radix);


/**
 \brief Fce hex2chr() vrátí hexa èíslo desítkovì
 \param buf - buffer s 1-2 znakovým hexa èíslem
 \return desítkový výsledek
 **/
unsigned char hex2chr(unsigned char *buf);

#endif /* _GLOBFUN_H_ */
