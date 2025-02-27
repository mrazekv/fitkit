/*******************************************************************************
   xmodem.h:
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
     Implementace XModem protokolu.
*******************************************************************************/

#ifndef _XMODEM_H_
#define _XMODEM_H_

#define XMODEMAPI

/**
 \brief Naprogramovani FPGA konfiguracni informaci zaslanou Xmodemem
 \return viz XModemRxBufCB
 **/
long XModemProgFPGA();

unsigned char FLASH_WriteFile(unsigned short startPage);

/**
 \brief Fce pøijme celý soubor, pomocí protokolu XModem.
 \param func   - callback funkce, která se zavolá vždy po úspešném pøeètení paketu
               - musí vrátit 0 pøi úspìšném zpracování dat
 \param datasz - velikost cílových dat
 \param data   - pøijatá data
 \return  >=0 - bez chyby
      \n   <0 - chyba:
      \n     -1 - pøeruseni synchronizace vysílaèem.
      \n     -2 - chyba pøi synchronizaci s vysílaèem.
      \n     -3 - pøíliš mnohokrát opakováno pøeposlání paketu.
      \n     -4 - chyba v obslužné callback fci.
      \n     -6 - zrušení spojeni, pøišel BREAK.
      \n     -7 - velikost paketu 1024B není podporována.
 **/
long XModemRxBufCB(int (*func)(unsigned char* data, long datasz));

/**
 \brief Fce posle soubor, pomoci protokolu XModem.
 \param srcsz - velikost cilovych dat
 \param PacketLen128 - !=0 velikost paketu 128B, jinak 1024B
 \param func - callback pro ziskani dat k odeslani
 \param data - odesilana data
 \param datasz - velikost odesilanych dat
 \return  =0...ok
      \n !=0...chyba:
      \n    1...Chyba synchronizace vysilace s prijimacem.
      \n    2...Preruseni prenosu prijimacem.
      \n    3...Nastala chyba pri prenosu.
      \n    4...Neprisel posledni potvrzovaci znak.
      \n    5...Velikost paketu 1024B neni podporovana.
 **/
long XModemTxBufCB(long srcsz, char PacketLen128,
                   int (*func)(unsigned char* data, long datasz));

#endif /* _XMODEM_H_ */
