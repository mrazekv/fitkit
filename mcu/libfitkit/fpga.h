/*******************************************************************************
   fpga.h:
   Copyright (C) 2006 Brno University of Technology,
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

*******************************************************************************/
#ifndef _FPGA_H_
#define _FPGA_H_

#define OC_FPGA 0x10   ///< baze OC pro FPGA (platne jsou horni 4 bity)

#define SPI_FPGA_ENABLE_WRITE 0x01   ///< Definice bitu reprezentujici zapis do FPGA
#define SPI_FPGA_ENABLE_READ  0x02   ///< Definice bitu reprezentujici cteni z FPGA


/******************************************************************************/
/* reset pro FPGA */
#define FPGA_RESET_OUT    P3OUT
#define FPGA_RESET_DIR    P3DIR
#define FPGA_RESET_PIN    BIT0               ///< signal RESET pro komponenty uvnitr FPGA


/**
 \brief Inicializace resetu pro FPGA
 **/
void FPGA_init(void);


/**
 \brief Reset FPGA.
 */
void FPGA_reset(void);


/**
 \brief Ukon�en� komunikace s FPGA p�es SPI
 **/
#define FPGA_Close()  SPI_Close()

/**
 \brief �ten� a z�pis bloku dat o d�lce DN byt� do FPGA s AN bytovou adresou 
 \param rw - povoleni �ten� a (nebo) z�pisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - adresa �adi�e v FPGA
 \param data - ukazatel na pole ur�en� k v�m�n� dat s FPGA
 \param an - po�et byt� adresy (1-4)
 \param dn - d�lka bloku k v�m�n�
 \return V�dy 0
 **/
int FPGA_SPI_RW_AN_DN(unsigned char rw, unsigned long addr, unsigned char *data, 
                      unsigned char an, unsigned char dn);

/**
 \brief �ten� a z�pis bloku dat o delce n*8 bit� do FPGA s 8-mi bitovou adresou c�lov�ho za��zen�.
 \param rw - povoleni cteni a (nebo) zapisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - 8-mi bitov� adresa c�lov�ho za��zen� v FPGA
 \param data - ukazatel na pole ur�en� k v�m�n� dat s FPGA
 \param n - d�lka bloku k v�m�n�
 \return V�dy 0
 **/
int FPGA_SPI_RW_A8_DN(unsigned char rw, unsigned char addr, unsigned char *data,
                      int n);

/**
 \brief �ten� a z�pis bytu do FPGA
 \param rw - povolen� �ten� a (nebo) z�pisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - adresa cilov�ho za��zen� v FPGA (8 bit�)
 \param data - data urcen� pro odesl�n� do FPGA
 \return P�e�ten� byte z FPGA
 **/
unsigned char FPGA_SPI_RW_A8_D8(unsigned char rw, unsigned char addr,
                                unsigned char data);

/**
 \brief �ten� a z�pis slova (16 bit�) do FPGA
 \param rw - povoleni �ten� a (nebo) z�pisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - adresa cilov�ho za��zen� v FPGA (16 bit�)
 \param data - data urcen� pro z�pis do FPGA
 \return P�e�ten� slovo z FPGA
 **/
unsigned int FPGA_SPI_RW_A8_D16(unsigned char rw, unsigned char addr,
                                unsigned int data);

#endif /* _FPGA_H_ */
