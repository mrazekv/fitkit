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
 \brief Fce vr�t� binarn� podobu znaku 'chr'.
        nejvyznamn�j�� bit na pozici nula
 \param buf - buffer, kter� ma alokovan�ch alespo� 9 byt�
 \param chr - p�ev�d�n� znak
 **/
void chr2bin(unsigned char *buf, unsigned char chr);

/**
 \brief Fce chr2hex() vr�t� hexa podobu znaku 'chr'.
    - nejv�znamn�j�� bit na pozici nula
 \param buf - buffer, kter� m� alokovan�ch alespo� 9 bytu
 \param chr - p�ev�d�n� znak
 **/
void chr2hex(unsigned char *buf, unsigned char chr);

/**
 \brief Fce vr�t� ��selnou reprezentaci znaku '1' -> 1.
 \param chr - p�ev�d�n� znak
 \return �islo (digit)
 **/
unsigned char chr2digit(unsigned char chr);

/**
 \brief Fce vr�t� znak v jeho ��seln� reprezentaci 1 -> '1'.
 \param chr - p�ev�d�n� ��slo
 \return znak
 **/
char digit2chr(char chr);

/**
 \brief Fce obrat� po�ad� bit� ve znaku 'chr'.
 \param chr - p�ev�d�n� znak
 \return znak s obr�cen�m po�ad�m bit�
 **/
unsigned char rchr(unsigned char chr);

/*******************************************************************************
 * Fce prevadi long na string v hexa.
 * - input...prevadena hodnota
 * - buf.....vysledny retezec
*******************************************************************************/
//void long2strhex(long input, char *str);

/**
 \brief Fce p�ev�d� string na long.
 \param  buf - vstupn� �et�zec
 \return long ��slo (32 bitu)
 **/
long str2long(char *buf);

/**
 \brief Fce p�ev�d� string na 2x long.
 \param buf - vstupn� �etezec obsahuj�c� 2 ��sla odd�len� mezerou.
 \param n1 - ukazatel k ulo�en� 1. ��sla
 \param n2 - ukazatel k ulo�en� 2. ��sla
 \return Prvn� nalezen� ��slo v �et�zci (=n1)
 **/
long str2long_2x(char *buf, long *n1, long *n2);

/**
 \brief Fce p�evrac� �et�zec (znaky v opa�n�m sledu).
 \param buf - vstupn� �et�zec
 \return p�evr�cen� �et�zec
 **/
char *strrev(char *buf);

/**
 \brief P�evede �et�zec na velk� p�smena
 \param dest - vystupni buffer
 \param buf - vstupni string
 **/
void strupper(unsigned char *dest, unsigned char *buf);

/**
 \brief Fce p�ev�d� long na string.
 \param buf - v�tupn� �et�zec
 \param input - long ��slo
 \param radix - maximaln� hodnota 1. digitu (u hexa 16, dec 10 ...)
 **/
void long2str(long input, char *buf, long radix);


/**
 \brief Fce hex2chr() vr�t� hexa ��slo des�tkov�
 \param buf - buffer s 1-2 znakov�m hexa ��slem
 \return des�tkov� v�sledek
 **/
unsigned char hex2chr(unsigned char *buf);

#endif /* _GLOBFUN_H_ */
