/*******************************************************************************
   enc28j60_spi_rx.h: Knihovna funkcí pro práci s pøijímacím bufferem
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
	\brief Knihovna funkcí pro práci s pøijímacím bufferem v ENC28J60.
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

#define RX_START	0		///<zaèátek bufferu pro pøíjem paketù
#define RX_END		4972	///<konec bufferu pro pøíjem paketù

#define STATUS_LEN	4		///<delka stavového vektoru umístìného pøed každým pøijatým paketem


/**
 \brief Inicializace bufferu s pøíchozím paketem - interní funkce.
 \retval 0 - inicializace probìhla úspìšnì
 \retval -1 - chyba pøi inicializaci - žádný pøíchozí paket nebo opìtovné volání fce bez pøedchozího použití rx_free() 
 **/
char rx_init();

/**
 \brief Uzavøe buffer a uvolní místo pro další pøíchozí pakety - interní funkce.
 \retval 0  - Uvolnìní místa probìhlo úspìšnì
 \retval -1  - chyba pøi uvolòování místa, uvolnìní bufferu bez pøedchozí inicializace
 **/
char rx_free();

/**
 \endcond
 **/

/**
 \brief Pøeète bajt z bufferu.
 \returns hodnota bajtu , -1 pøi konci paketu
 **/
inline unsigned int rx_getc();

/**
 \brief Pøeète blok dat velikosti \a length z bufferu a zapíše na adresu odkazovanou ukazatelem \a ptr.
 \param ptr - ukazatel, kam se bude zapisovat
 \param length - délka dat v bajtech
 \returns poèet pøeètených bajtù
 **/
unsigned int rx_read(void* ptr, int length);

/**
 \brief Pøeète øetìzec ukonèený hodnotou LF z bufferu a zapíše na adresu odkazovanou ukazatelem \a ptr.
 \param ptr - ukazatel na øetìzec, 0 pro pøes
 \param max_length - maximální délka øetìzce
 \returns poèet pøeètených bajtù
 **/
unsigned int rx_getline(void* ptr, int max_length);

/**
 \brief Vrátí poèet zbývajících bajtù v bufferu.
 \returns poèet zbývajících bajtù
 **/
inline int rx_left();

/**
 \brief Kontroluje zda je buffer pøeèten celý.
 \retval 0 - v bufferu zbývají nepøeètená data
 \retval 1 - buffer je pøeèten celý
 **/
inline unsigned char rx_isempty();

/**
 \cond
 **/
 
/**
 \brief oøízne padding paketu (doplnìní nulami na délku 60 bajtù u ethernetu) - interní funkce.
 \param length - délka paddingu 
 **/
inline void rx_cut_padding(unsigned char length);

/**
 \endcond
 **/

/**
 \brief pøeskoèí \a len bajtù v bufferu.
 \param len - poèet pøeskoèených bajtù 
 **/
#define rx_skip(len) rx_read(NULL, len)


#endif /*_ENC28J60_SPI_RX_*/
