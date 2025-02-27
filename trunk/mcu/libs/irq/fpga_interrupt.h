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
   #define BASE_ADDR_INTERRUPT 0x80 ///< Bázová adresa SPI pro øadiè interrupt_controller
#endif

#ifndef MCU_INTERRUPT_PINS
  #define MCU_INTERRUPT_PINS BIT7 + BIT3 ///< Definice pinu, na který je pøipojen vstup pøerušení z FPGA
#endif

/**
 \brief Pøeète vektor pøerušení, pøi ètení vektoru pøerušení se vektor pøerušení vynuluje.
 \return Vektor pøerušení. Každý bit hodnoty 1 oznamuje, že komponeneta v FPGA, pøípojená na daný bit, vyvolala IRQ.
 **/
#define read_interrupt_vector_from_fpga() \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_READ, BASE_ADDR_INTERRUPT, 0);

/**
 \brief Nastavení maskování pøerušení v FPGA.
 \param interrupt_vector - bitová maska pøerušení v FPGA
 **/
#define set_interrupt_mask_fpga(interrupt_vector) \
        FPGA_SPI_RW(AN_D8, SPI_FPGA_ENABLE_WRITE, BASE_ADDR_INTERRUPT, interrupt_vector);

/**
 \brief Inicializace pøerušení od FPGA.
 \param bits - bitová maska pro pøerušení v FPGA. Jednièkové bity povolují pøerušení.
 **/
inline int fpga_interrupt_init(unsigned char bits);


/**
 \brief Zakázání pøerušení na bitech zadaných v parametru (úprava maskování). 
 \n pr:
    \n disable_interup_bits(BIT2+BIT6)
    \n Zakáže pøerušení z bitu 2. a 6. v FPGA (nastaví masku).
    \n Ostatní bity zùstanou na pùvodních hodnotách.
 \param bits - zakázané (maskované) bity pøerušeni
 **/
void inline disable_interrupt_bits(unsigned char bits);


/**
 \brief Povolení pøerušení na bitech zadaných v parametru (úprava maskování).
  \n pr:
    \n enable_interup_bits(BIT2+BIT6)
    \n Povolí pøerušení z bitu 2. a 5. v FPGA (nastaví masku).
    \n Ostatní bity zùstanou na pùvodních hodnotách.
 \param bits - povolené (odmaskované) bity pøerušení
 **/
void inline enable_interrupt_bits(unsigned char bits);


/**
 \brief Funkce musí být definována v aplikaci. Je automaticky volána, pokud pøijde pøerušení z FPGA.
 \param bits - urèuje, které komponenty o pøerušení požádaly.
 **/
extern void fpga_interrupt_handler(unsigned char bits);

#endif  //_FPGA_INTERRUP_H_

