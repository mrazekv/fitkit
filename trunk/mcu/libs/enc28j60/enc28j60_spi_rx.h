/*******************************************************************************
   enc28j60_spi_rx.h: Knihovna funkc� pro pr�ci s p�ij�mac�m bufferem
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
	\file enc28j60_spi_rx.h
	\brief Knihovna funkc� pro pr�ci s p�ij�mac�m bufferem v ENC28J60.
*/

#ifndef _ENC28J60_SPI_RX_
#define _ENC28J60_SPI_RX_

#define NULL 0		///< NULL pointer

/*
#define RX_START	3126
#define RX_END		0x1FFF
*/
/*
zmena, dodany radic mel obcasne problemy s inicializaci jednoho registru, ERXWRPT,
je pouze pro cteni a inicializuje se automaticky podle hodnoty ERXST, ale obcas
se v nem hodnota ERXST neobjevila, byla tam 0, proto jsem ukazatele zamenil,
aby byla hodnota ERXST a potazmo ERXWRPT 0 a tak i pri teto chybe pracoval radic spravne
*/

/**
 \cond
 **/

#define RX_START	0		///<za��tek bufferu pro p��jem paket�
#define RX_END		4972	///<konec bufferu pro p��jem paket�

#define STATUS_LEN	4		///<delka stavov�ho vektoru um�st�n�ho p�ed ka�d�m p�ijat�m paketem


/**
 \brief Inicializace bufferu s p��choz�m paketem - intern� funkce.
 \retval 0 - inicializace prob�hla �sp�n�
 \retval -1 - chyba p�i inicializaci - ��dn� p��choz� paket nebo op�tovn� vol�n� fce bez p�edchoz�ho pou�it� rx_free() 
 **/
char rx_init();

/**
 \brief Uzav�e buffer a uvoln� m�sto pro dal�� p��choz� pakety - intern� funkce.
 \retval 0  - Uvoln�n� m�sta prob�hlo �sp�n�
 \retval -1  - chyba p�i uvol�ov�n� m�sta, uvoln�n� bufferu bez p�edchoz� inicializace
 **/
char rx_free();

/**
 \endcond
 **/

/**
 \brief P�e�te bajt z bufferu.
 \returns hodnota bajtu , -1 p�i konci paketu
 **/
inline unsigned int rx_getc();

/**
 \brief P�e�te blok dat velikosti \a length z bufferu a zap�e na adresu odkazovanou ukazatelem \a ptr.
 \param ptr - ukazatel, kam se bude zapisovat
 \param length - d�lka dat v bajtech
 \returns po�et p�e�ten�ch bajt�
 **/
unsigned int rx_read(void* ptr, int length);

/**
 \brief P�e�te �et�zec ukon�en� hodnotou LF z bufferu a zap�e na adresu odkazovanou ukazatelem \a ptr.
 \param ptr - ukazatel na �et�zec, 0 pro p�es
 \param max_length - maxim�ln� d�lka �et�zce
 \returns po�et p�e�ten�ch bajt�
 **/
unsigned int rx_getline(void* ptr, int max_length);

/**
 \brief Vr�t� po�et zb�vaj�c�ch bajt� v bufferu.
 \returns po�et zb�vaj�c�ch bajt�
 **/
inline int rx_left();

/**
 \brief Kontroluje zda je buffer p�e�ten cel�.
 \retval 0 - v bufferu zb�vaj� nep�e�ten� data
 \retval 1 - buffer je p�e�ten cel�
 **/
inline unsigned char rx_isempty();

/**
 \cond
 **/
 
/**
 \brief o��zne padding paketu (dopln�n� nulami na d�lku 60 bajt� u ethernetu) - intern� funkce.
 \param length - d�lka paddingu 
 **/
inline void rx_cut_padding(unsigned char length);

/**
 \endcond
 **/

/**
 \brief p�esko�� \a len bajt� v bufferu.
 \param len - po�et p�esko�en�ch bajt� 
 **/
#define rx_skip(len) rx_read(NULL, len)


#endif /*_ENC28J60_SPI_RX_*/
