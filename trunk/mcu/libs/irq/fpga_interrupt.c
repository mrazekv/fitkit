/*******************************************************************************
   fpga_interupt.c:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Copyright (C) 2006 CAMEA ltd. & R.B.E
   Author(s): Radomir Bardas
              Jan Markovic <xmarko04@stud.fit.vutbr.cz>

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

#include <fitkitlib.h>
#include <isr_compat.h>
#include "fpga_interrupt.h"


volatile unsigned char interrupt_mask; ///< Aktu�ln� nastaven� maska p�eru�en�, kter� je nastavena i v FPGA


/**
 \brief Povolen� p�eru�en� na bitech zadan�ch v parametru (�prava maskov�n�).
  \n pr:
    \n enable_interup_bits(BIT2+BIT6)
    \n Povol� p�eru�en� z bitu 2. a 5. v FPGA (nastav� masku).
    \n Ostatn� bity z�stanou na p�vodn�ch hodnot�ch.
 \param bits - povolen� (odmaskovan�) bity p�eru�en�
 **/
void inline enable_interrupt_bits(unsigned char bits){
   interrupt_mask |= bits;
   set_interrupt_mask_fpga(interrupt_mask);

}

/**
 \brief Zak�z�n� p�eru�en� na bitech zadan�ch v parametru (�prava maskov�n�). 
 \n pr:
    \n disable_interup_bits(BIT2+BIT6)
    \n Zak�e p�eru�en� z bitu 2. a 6. v FPGA (nastav� masku).
    \n Ostatn� bity z�stanou na p�vodn�ch hodnot�ch.
 \param bits - zak�zan� (maskovan�) bity p�eru�eni
 **/
void inline disable_interrupt_bits(unsigned char bits){
   interrupt_mask &= ~bits;
   set_interrupt_mask_fpga(interrupt_mask);
}



/**
 \brief Inicializace p�eru�en� od FPGA.
 \param bits - bitov� maska pro p�eru�en� v FPGA. Jedni�kov� bity povoluj� p�eru�en�
 **/
int fpga_interrupt_init(unsigned char bits){
    // nastaveni na strane FPGA (nastaveni masky preruseni)
    interrupt_mask = 0;
    enable_interrupt_bits(bits);
    // inicializace na strane MCU
    P1DIR &= ~(MCU_INTERRUPT_PINS);  // nastaveni vstupu pro interup (P1M3)
    P1IES = 0x00;                   // mod interaptu (nastupni hrana)
    P1IE  = MCU_INTERRUPT_PINS;      //
    P1SEL &= ~(MCU_INTERRUPT_PINS);  //
    return 0;
}



/**
  \brief Obsluha p�eru�en� z FPGA na portu 1.
 */
//interrupt(PORT1_VECTOR) fpga_interrupt(void){
ISR(PORT1, fpga_interrupt) 
{
   unsigned char interrupt_vector_fpga;

   do { // while
    
      //precteni vektoru preruseni z FPGA
      interrupt_vector_fpga = read_interrupt_vector_from_fpga();

      fpga_interrupt_handler(interrupt_vector_fpga);
   } while ((P1IN) & (MCU_INTERRUPT_PINS));  // pokud behem obsluhy preruseni prislo dalsi preruseni
   
   // Vsechna preruseni byla obslouzena
   P1IFG &= ~(BIT3 + BIT7);
}

