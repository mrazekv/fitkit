/*******************************************************************************
   thermometer.c: Funkce pro mereni teploty
   Copyright (C) 2007 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Vojtech Cizinsky <xcizin01 AT stud.fit.vutbr.cz>

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

#include <msp430x16x.h>
#include <stdlib.h>
#include "thermometer.h"

/*******************************************************************************
 *  Zjisteni hodnoty kanalu 10 AD prevodniku MCU (teplotni sensor)
 *  Bere se prumer z osmi hodnot 
*******************************************************************************/
unsigned long thermometer_measure(void) {
   unsigned long ReturnValue;
  
   ADC12CTL0 = ADC12ON | SHT0_15 | MSH | REFON;  // ADC on, int. ref. on (1,5 V),
                                                 // multiple sample & conversion
   ADC12CTL1 = ADC12SSEL_2 | ADC12DIV_7 | CSTARTADD_0 | CONSEQ_1 | SHP;  

   ADC12MCTL0 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL1 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL2 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL3 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL4 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL5 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL6 = SREF_1 | INCH_10;                 // int. ref., channel 10
   ADC12MCTL7 = EOS | SREF_1 | INCH_10;           // int. ref., channel 10, last seg.
  
   ADC12CTL0 |= ENC;                              // povoleni prevodu
   ADC12CTL0 |= ADC12SC;                          // sample & convert
  
   while (ADC12CTL0 & ADC12SC);                   // cekani na prevod
  
   ADC12CTL0 &= ~ENC;                             

   ReturnValue = ADC12MEM0;                       // secteni hodnot
   ReturnValue += ADC12MEM1;
   ReturnValue += ADC12MEM2;
   ReturnValue += ADC12MEM3;
   ReturnValue += ADC12MEM4;
   ReturnValue += ADC12MEM5;
   ReturnValue += ADC12MEM6;
   ReturnValue += ADC12MEM7;

   ReturnValue >>= 3;                             // zprumerovani hodnot

   return ReturnValue;
}

/*******************************************************************************
 *  Prevod namerene hodnoty na teplotu ve stupnich Celsia
*******************************************************************************/
int thermometer_gettemp(void) {
   unsigned long ReturnValue;

   ReturnValue = thermometer_measure();

   if (ReturnValue < 2691) ReturnValue = 2691;     // lower bound = 0C 
   ReturnValue = 1000*(ReturnValue - 2691) / 972;  // convert AD-value to a temperature

   // prevod na teplotu v °C dle katalogu
   //return (((ReturnValue*1.5)/4095)-0.986)/0.000355 + (THERMOMETER_CALIBRATION)*10; 

   return ReturnValue;
}


/*******************************************************************************
 *  Inicializace generatoru nahodnych cisel
*******************************************************************************/
void thermometer_init_rand(void) {
   srand(thermometer_measure());
}


