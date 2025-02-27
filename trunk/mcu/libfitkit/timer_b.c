/*******************************************************************************
   timerB.c:
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

*******************************************************************************/

#include "arch_specific.h"
#include "define.h"
#include "timer_b.h"

/**
 \brief Inicializace èasovaèe (TIMERB)
 **/
void timerb_init(void) {
  // TimerB zvysi svou hodnotu kazdych 8/f_SMCLK sekund
  // 8 / 14.745 MHz = 0.5425us

  TBCTL  =  CNTL_0 | TBCLR | TBSSEL_2 | ID_2;      // nulovani casovace, halt, clock SMCLK, divider 4, 16bit counter
  TBCTL |=  MC_2;                                  // kontinualni mod, start timer
}

/**
 \brief Zpožïovací fce - používá èasovaè TIMERB, který musí být aktivní.
 \param msecs - délka zpoždìní v milisekundách
 **/
void delay_ms(unsigned long msecs) {
  unsigned long Pomdelay_cycle;
  unsigned int  FirstTBR;
  unsigned int  SecondTBR;

  msecs   = msecs * 1843;  // 1ms ~= 1843 * perioda casovace (1843 * 0.5425us)
  FirstTBR = TBR;
  Pomdelay_cycle = 0;

  while (Pomdelay_cycle < msecs) 
  {

    WDG_reset();  //nulovani watchdogu

    SecondTBR = TBR;
    if (SecondTBR != FirstTBR) 
    {
       if (SecondTBR > FirstTBR)
         Pomdelay_cycle = Pomdelay_cycle + (unsigned long)(SecondTBR - FirstTBR);
       else
         Pomdelay_cycle = Pomdelay_cycle + (unsigned long)((TBCCR0 - FirstTBR) + SecondTBR + 1);

       FirstTBR = SecondTBR;
    }
    
  }
}
