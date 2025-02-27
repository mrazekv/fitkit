/*******************************************************************************
   globfunc.c:
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


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "arch_specific.h"
#include "define.h"
#include "globfun.h"

/**
 \brief Fce vrátí binarní podobu znaku 'chr'.
        nejvyznamnìjší bit na pozici nula
 \param buf - buffer, který ma alokovaných alespoò 9 bytù
 \param chr - pøevádìný znak
 **/
void chr2bin(unsigned char *buf, unsigned char chr) {
  int ii;
  for (ii=7; ii >= 0; ii--) {
    buf[ii] = ((0x01 & chr) == 0x01)? '1':'0';
    chr = (chr >> 1);
  }
  buf[8] = 0; // ukonceni retezce
}

/**
 \brief Fce vrátí znak v jeho èíselné reprezentaci 1 -> '1'.
 \param chr - pøevádìné èíslo
 \return znak
 **/
char digit2chr(char chr) {
  if ((chr >= 0) && (chr <= 9))
    return (chr + '0');

  chr -= 10;
  return ('A' + chr);
}


/**
 \brief Fce chr2hex() vrátí hexa podobu znaku 'chr'.
    - nejvýznamnìjší bit na pozici nula
 \param buf - buffer, který má alokovaných alespoò 9 bytu
 \param chr - pøevádìný znak
 **/
void chr2hex(unsigned char *buf, unsigned char chr) {
  buf[0] = digit2chr((chr >> 4) & 0x0F);        // horni 4.bity
  buf[1] = digit2chr(chr & 0x0F);               // spodni 4.bity
  buf[2] = 0;                                   // ukoncovaci byte
}

/**
 \brief Fce vrátí èíselnou reprezentaci znaku '1' -> 1.
 \param chr - pøevádìný znak
 \return èislo (digit)
 **/
unsigned char chr2digit(unsigned char chr) {
  if ((chr >= '0') && (chr <= '9'))
    return (chr - '0');
  if ((chr >= 'A') && (chr <= 'F'))
    return (chr - 'A' + 10);
  if ((chr >= 'a') && (chr <= 'f'))
    return (chr - 'a' + 10);
  return 0;
}

/**
 \brief Fce hex2chr() vrátí hexa èíslo desítkovì
 \param buf - buffer s 1-2 znakovým hexa èíslem
 \return desítkový výsledek
 **/
unsigned char hex2chr(unsigned char *buf) {
  unsigned char res = chr2digit(buf[0]);
  if (buf[1] != 0) {
	  res = (res << 4) + chr2digit(buf[1]);
  }
  return res;
}

/**
 \brief Fce obratí poøadí bitù ve znaku 'chr'.
 \param chr - pøevádìný znak
 \return znak s obráceným poøadím bitù
 **/
unsigned char rchr(unsigned char chr) {
  unsigned char ii, reschr = 0x00;
  for (ii=0; ii<8; ii++) {
    if ((chr & 0x01) == 0x01)
      reschr |= (0x80>>ii);
    chr = chr>>1;
  }
  return reschr;
}


/**
 \brief Fce pøevádí string na long.
 \param  buf - vstupní øetìzec
 \return long èíslo (32 bitu)
 **/
long str2long(char *buf) {
  int ii, len;
  long res = 0;

  // odstraneni mezer na zacatku
  for (ii=0; buf[ii] == ' '; ii++) {
  }
  buf += ii;

  // delka cislic
  for (len=0; isdigit(buf[len]); len++) {
  }

  // prevod stringu na cislo
  for (ii=0; ii<len; ii++) {
    res += chr2digit(buf[ii]);
    if (ii != len-1)
      res*=10;
  }

  return res;
}


/**
 \brief Fce pøevádí string na 2x long.
 \param buf - vstupní øetezec obsahující 2 èísla oddìlená mezerou.
 \param n1 - ukazatel k uložení 1. èísla
 \param n2 - ukazatel k uložení 2. èísla
 \return První nalezené èíslo v øetìzci (=n1)
 **/
long str2long_2x(char *buf, long *n1, long *n2) {
  int ii, len, c1, c2;
  c1 = 0;
  c2 = 0;
  // odstraneni mezer na zacatku
  for (ii=0; buf[ii] == ' '; ii++) {
  }
  buf += ii;

  // delka cislic
  for (len=0; isdigit(buf[len]); len++) {
  }

  // prevod stringu na cislo1
  for (ii=0; ii<len; ii++) {
    c1 += chr2digit(buf[ii]);
    if (ii != len-1) c1 *=10;
  }
  // preskoceni 1. cisla
  buf += len;

  // odstraneni mezer mezi cisly
  for (ii=0; buf[ii] == ' '; ii++) {
  }
  buf += ii;

  // delka cislic
  for (len=0; isdigit(buf[len]); len++){
  }

  // prevod stringu na cislo1
  for (ii=0; ii<len; ii++)
  {
     c2 += chr2digit(buf[ii]);
     if (ii != len-1) c2 *=10;
  }

  *n1 = c1;
  *n2 = c2;

  return c1;
}


/**
 \brief Fce pøevrací øetìzec (znaky v opaèném sledu).
 \param buf - vstupní øetìzec
 \return pøevrácený øetìzec
 **/
char *strrev(char *buf) {
  int ii, jj = strlen(buf);
  char chr;
  for (ii=0; ii < (strlen(buf)>>1); ii++) {
    jj--;
    chr = buf[ii];
    buf[ii] = buf[jj];
    buf[jj] = chr;
  }
  return buf;
}

/**
 \brief Pøevede øetìzec na velká písmena
 \param dest - vystupni buffer
 \param buf - vstupni string
 **/
void strupper(unsigned char *dest, unsigned char *buf)
{
 int i;
 dest[0] = 0;
 for (i=0;;i++) {
    dest[i] = ((buf[i] > 96) && (buf[i] < 123)) ? buf[i] - 32 : buf[i];
    if (buf[i] == 0) return;
 }
}

/**
 \brief Fce pøevádí long na string.
 \param buf - výtupní øetìzec
 \param input - long èíslo
 \param radix - maximalní hodnota 1. digitu (u hexa 16, dec 10 ...)
 **/
void long2str(long input, char *buf, long radix) {
  int      ii  = 0;
  unsigned char rest;
  char     isMinus = 0;

  if (radix == 16)
    buf[ii++] = 'h';

  if (input == 0) {
     buf[ii++] = '0';
  }
  else if (input < 0) {
    input *= -1;
    isMinus = 1;
  }

  // prevod cisla na string
  for (; input!=0; ii++) {
    rest = input % radix;
    buf[ii] = digit2chr(rest);
    input -= rest;
    input /= radix;
  }

  // pokud zaporne cislo, tak vlozim minus
  if (isMinus)
    buf[ii++] = '-';

  // ukoncim retezec
  buf[ii] = 0;
  buf = strrev(buf);
}
