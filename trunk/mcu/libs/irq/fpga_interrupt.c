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


volatile unsigned char interrupt_mask; ///< Aktuálnì nastavená maska pøerušení, která je nastavena i v FPGA


/**
 \brief Povolení pøerušení na bitech zadaných v parametru (úprava maskování).
  \n pr:
    \n enable_interup_bits(BIT2+BIT6)
    \n Povolí pøerušení z bitu 2. a 5. v FPGA (nastaví masku).
    \n Ostatní bity zùstanou na pùvodních hodnotách.
 \param bits - povolené (odmaskované) bity pøerušení
 **/
void inline enable_interrupt_bits(unsigned char bits){
   interrupt_mask |= bits;
   set_interrupt_mask_fpga(interrupt_mask);

}

/**
 \brief Zakázání pøerušení na bitech zadaných v parametru (úprava maskování). 
 \n pr:
    \n disable_interup_bits(BIT2+BIT6)
    \n Zakáže pøerušení z bitu 2. a 6. v FPGA (nastaví masku).
    \n Ostatní bity zùstanou na pùvodních hodnotách.
 \param bits - zakázané (maskované) bity pøerušeni
 **/
void inline disable_interrupt_bits(unsigned char bits){
   interrupt_mask &= ~bits;
   set_interrupt_mask_fpga(interrupt_mask);
}



/**
 \brief Inicializace pøerušení od FPGA.
 \param bits - bitová maska pro pøerušení v FPGA. Jednièkové bity povolují pøerušení
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
  \brief Obsluha pøerušení z FPGA na portu 1.
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

