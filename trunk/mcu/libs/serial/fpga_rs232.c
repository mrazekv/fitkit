/*******************************************************************************
   fpga_rs_232.c:
   Copyright (C) 2006 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Jan Markovic <xmarko04@stud.fit.vutbr.cz>
              Jakub Vavra  <xvavra07@stud.fit.vutbr.cz>

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
#include <signal.h>
#include <math.h>
#include <stdio.h>

#include <fitkitlib.h>

#include "fpga_rs232.h"


/// buffer pro vysilani
volatile unsigned char FPGA_TransmitBuf[FPGA_LengthBuffers_Tx];
/// index bytu v buffru pri zapisu do bufru
volatile unsigned char FPGA_TransmitIndex;
/// index bytu v buffru pri cteni z bufru pro vysilani    
volatile unsigned char FPGA_TransmitIndexSend;
/// kolik bytu v buffru
volatile unsigned char FPGA_FPGA_TransmitNum;

/// buffer pro prijem
volatile unsigned char FPGA_ReceiveBuf[FPGA_LengthBuffers_Rx];
/// index bytu v buffru pri cteni z bufru
volatile unsigned char FPGA_ReceiveIndex;
/// index bytu v buffru pri zapisu do bufru
volatile unsigned char FPGA_ReceiveIndexRec;
/// kolik bytu v buffru
volatile unsigned char FPGA_ReceiveNum;


//zacatek fronty v bufferu
volatile unsigned char FPGA_Tx_Begin;
//konec fronty v bufferu
volatile unsigned char FPGA_Tx_End;
//priznak ukonceneho prenosu
volatile unsigned char FPGA_TransmissionEnd;



/**
 * Pocatecni inicializace rs232 pro FPGA
 */
void FPGA_RS232_Init(){

    // nejake inicializace preruseni
    FPGA_Tx_Begin = 0;
    FPGA_Tx_End = 0;
    FPGA_TransmissionEnd = 1;

    FPGA_ReceiveIndex      = 0;
    FPGA_ReceiveIndexRec   = 0;
    FPGA_ReceiveNum        = 0;
}


/**
 * Pocet znaku ve vstupnim bufferu z rs232 z FPGA
 * aneb test na prazdnost bufferu
 */
char FPGA_rs232_UseBuf(){
    return FPGA_ReceiveNum;
}

/**
 * Pocet znaku ve vytupnim bufferu do rs232 na FPGA
 * aneb test na prazdnost bufferu
 */
char FPGA_rs232_TX_UseBuf(){
  return ((FPGA_Tx_Begin<=FPGA_Tx_End)?(FPGA_Tx_End-FPGA_Tx_Begin):(FPGA_LengthBuffers_Tx+FPGA_Tx_End-FPGA_Tx_Begin)); 
}


/**
 * Cteni znaku z bufferu seriove linky FPGA.
 * - vraci jeden znak (8 bitu)
 **/
unsigned char FPGA_RS232_ReceiveChar(void){
  unsigned char Uchar;
  Uchar = 0;
  if (FPGA_ReceiveNum > 0)
    {
     Uchar = FPGA_ReceiveBuf[FPGA_ReceiveIndex];
     FPGA_ReceiveIndex++;
     if (FPGA_ReceiveIndex >= FPGA_LengthBuffers_Rx) FPGA_ReceiveIndex = 0;
     FPGA_ReceiveNum--;
    }
  return (Uchar);
}


/**
 * Obsluha preruseni pri primu dat z rs232 v FPGA
 * precteni znaku z rs232 v FPGA a ulozeni do bufferu
 */
void fpga_interrupt_rs232_rxd(void){

    WDG_reset();                                // software watchdog

    unsigned char ch = read_bit_fpga_rs_232(BASE_ADDR_RS_232_USB);

    if (FPGA_ReceiveNum < FPGA_LengthBuffers_Rx)
    {
        FPGA_ReceiveBuf[FPGA_ReceiveIndexRec] = ch;  // zapis dat do buffru
        FPGA_ReceiveIndexRec++;
        if (FPGA_ReceiveIndexRec >= FPGA_LengthBuffers_Rx)
            FPGA_ReceiveIndexRec = 0;
        FPGA_ReceiveNum++;
    }
}


/**
 * Obsluha preruseni pri odesilani dat z rs232 v FPGA
 * zapsani znaku na rs232 v FPGA a smazani z bufferu
 */
void fpga_interrupt_rs232_txd(void){
     WDG_reset();                                 // software watchdog
     FPGA_TransmissionEnd = 1;
     if (FPGA_rs232_TX_UseBuf()!=0){
       //odeslani znaku
       FPGA_TransmissionEnd = 0;
       send_bit_fpga_rs_232(FPGA_TransmitBuf[FPGA_Tx_Begin],BASE_ADDR_RS_232_USB);
       // posun ukazatele znaku k odeslani
       if ((FPGA_Tx_Begin+1) == FPGA_LengthBuffers_Tx){ FPGA_Tx_Begin=0;}
         else {FPGA_Tx_Begin+=1;}
       //pokud je buffer nyni prazdny, presuneme ukazatele na zacatek
       if (FPGA_Tx_Begin==FPGA_Tx_End) {
         FPGA_Tx_Begin=FPGA_Tx_End=1;
       }
     }
}

 /**
 * Zapis znaku do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendChar(unsigned char schar){
       WDG_reset();                                 // software watchdog
      // test zna je buffer prazdny a predchozi prenos byl ukoncen
      if ((FPGA_rs232_TX_UseBuf()==0)&&(FPGA_TransmissionEnd == 1)){
          FPGA_TransmissionEnd= 0; //Snulujeme priznak
          send_bit_fpga_rs_232(schar,BASE_ADDR_RS_232_USB); //a znak rovnou posleme
          //RS232_SendChar(schar);
          return;
      }
      // test zda neni buffer plny
      if (FPGA_rs232_TX_UseBuf()+1 < FPGA_LengthBuffers_Tx ){
          FPGA_TransmitBuf[FPGA_Tx_End]=schar; //vlozime znak
          //posun ukazatele konce fronty
          if (FPGA_Tx_End+1==FPGA_LengthBuffers_Tx){FPGA_Tx_End=0;}
             else {FPGA_Tx_End+=1;}
       } else {
             // buffer je plny - nedelame nic
         }
}

 /**
 * Zapis retezce do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendString(unsigned char *ptrStr)
{
  unsigned char Index = 0;

  while (ptrStr[Index] != 0)
    {
     WDG_reset();                                // software watchdog
     FPGA_Buff_SendChar(ptrStr[Index]);
     Index++;
    }
}

/**
 * Zapis retezce a CRLF do odesilaciho bufferu, pripadne jeho nasledne odeslani.
 */
void FPGA_Buff_SendStringCRLF(unsigned char *ptrStr)
{
  unsigned char Index = 0;

  while (ptrStr[Index] != 0)
    {
     WDG_reset();                                // software watchdog
     FPGA_Buff_SendChar(ptrStr[Index]);
     Index++;
    }
  FPGA_Buff_SendChar(CR);
  FPGA_Buff_SendChar(LF);
}

/**
 * Zaslani CRLF na rs232 na fpga
 */
void FPGA_SendCRLF(){
  FPGA_Buff_SendChar(CR);
  FPGA_Buff_SendChar(LF);
}
