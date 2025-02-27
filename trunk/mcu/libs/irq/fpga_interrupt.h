/*******************************************************************************
   fpga_interupt.h:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Copyright (C) 2006 CAMEA ltd. & R.B.E
   Author(s): Jan Markovic <xmarko04@stud.fit.vutbr.cz>

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


#ifndef _FPGA_INTERRUPT_H_
#define _FPGA_INTERRUPT_H_

#include <spi/spi_config.h> 

#ifndef BASE_ADDR_INTERRUPT
   #define BASE_ADDR_INTERRUPT 0x80 ///< B�zov� adresa SPI pro �adi� interrupt_controller
#endif

#ifndef MCU_INTERRUPT_PINS
  #define MCU_INTERRUPT_PINS BIT7 + BIT3 ///< Definice pinu, na kter� je p�ipojen vstup p�eru�en� z FPGA
#endif

/**
 \brief P�e�te vektor p�eru�en�, p�i �ten� vektoru p�eru�en� se vektor p�eru�en� vynuluje.
 \return Vektor p�eru�en�. Ka�d� bit hodnoty 1 oznamuje, �e komponeneta v FPGA, p��pojen� na dan� bit, vyvolala IRQ.
 **/
#define read_interrupt_vector_from_fpga() \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_READ, BASE_ADDR_INTERRUPT, 0);

/**
 \brief Nastaven� maskov�n� p�eru�en� v FPGA.
 \param interrupt_vector - bitov� maska p�eru�en� v FPGA
 **/
#define set_interrupt_mask_fpga(interrupt_vector) \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_WRITE, BASE_ADDR_INTERRUPT, interrupt_vector);

/**
 \brief Inicializace p�eru�en� od FPGA.
 \param bits - bitov� maska pro p�eru�en� v FPGA. Jedni�kov� bity povoluj� p�eru�en�.
 **/
inline int fpga_interrupt_init(unsigned char bits);


/**
 \brief Zak�z�n� p�eru�en� na bitech zadan�ch v parametru (�prava maskov�n�). 
 \n pr:
    \n disable_interup_bits(BIT2+BIT6)
    \n Zak�e p�eru�en� z bitu 2. a 6. v FPGA (nastav� masku).
    \n Ostatn� bity z�stanou na p�vodn�ch hodnot�ch.
 \param bits - zak�zan� (maskovan�) bity p�eru�eni
 **/
void inline disable_interrupt_bits(unsigned char bits);


/**
 \brief Povolen� p�eru�en� na bitech zadan�ch v parametru (�prava maskov�n�).
  \n pr:
    \n enable_interup_bits(BIT2+BIT6)
    \n Povol� p�eru�en� z bitu 2. a 5. v FPGA (nastav� masku).
    \n Ostatn� bity z�stanou na p�vodn�ch hodnot�ch.
 \param bits - povolen� (odmaskovan�) bity p�eru�en�
 **/
void inline enable_interrupt_bits(unsigned char bits);


/**
 \brief Funkce mus� b�t definov�na v aplikaci. Je automaticky vol�na, pokud p�ijde p�eru�en� z FPGA.
 \param bits - ur�uje, kter� komponenty o p�eru�en� po��daly.
 **/
extern void fpga_interrupt_handler(unsigned char bits);

#endif  //_FPGA_INTERRUP_H_

