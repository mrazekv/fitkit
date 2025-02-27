/*******************************************************************************
   fpga.c:
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

#include <string.h>
#include "arch_specific.h"
#include "fitkitlib.h"
#include "define.h"
#include "globfun.h"
#include "uart.h"
#include "flash_fpga.h"
#include "spi.h"
#include "fpga.h"
#include "timer_b.h"

/******************************************************************************/
/* Operacni kody FPGA */


/**
 \brief Inicializace resetu pro FPGA
 **/
void FPGA_init(void) {

  FPGA_RESET_DIR |= FPGA_RESET_PIN;
  FPGA_RESET_OUT |= FPGA_RESET_PIN;
}


/**
 \brief Reset FPGA.
 */
void FPGA_reset(void){
  FPGA_RESET_OUT |= FPGA_RESET_PIN;
  delay_cycle(100);
  FPGA_RESET_OUT &= ~FPGA_RESET_PIN;

  //cekani na nabehnuti oscilatoru
  delay_ms(500);
}


/**
 \brief �ten� a z�pis bytu do FPGA
 \param rw - povolen� �ten� a (nebo) z�pisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - adresa cilov�ho za��zen� v FPGA (8 bit�)
 \param data - data urcen� pro odesl�n� do FPGA
 \return P�e�ten� byte z FPGA
 **/
unsigned char FPGA_SPI_RW_A8_D8(unsigned char rw, unsigned char addr,
                                unsigned char data) {
  unsigned char data_out;
  SPI_set_cs_FPGA(1);
  // zapis op. kodu
  SPI_read_write(OC_FPGA|rw);
  // zapis adresy
  SPI_read_write(addr);
  // zapis a cteni dat
  data_out = SPI_read_write(data);
  SPI_set_cs_FPGA(0);
  return data_out;
}

/**
 \brief �ten� a z�pis slova (16 bit�) do FPGA
 \param rw - povoleni �ten� a (nebo) z�pisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - adresa cilov�ho za��zen� v FPGA (16 bit�)
 \param data - data urcen� pro z�pis do FPGA
 \return P�e�ten� slovo z FPGA
 **/
unsigned int FPGA_SPI_RW_A8_D16(unsigned char rw, unsigned char addr,
                                unsigned int data) {
  unsigned char data_inL, data_inH;
  unsigned int data_out;
  data_inL = data & 0xFF;
  data_inH = (data >> 8) & 0xFF;
  SPI_set_cs_FPGA(1);
  // zapis op. kodu
  SPI_read_write(OC_FPGA|rw);
  // zapis adresy
  SPI_read_write(addr);
  // zapis a cteni dat
  data_out = SPI_read_write(data_inH) << 8;
  data_out = data_out + SPI_read_write(data_inL);
  SPI_set_cs_FPGA(0);
  return data_out;
}



/**
 \brief �ten� a z�pis bloku dat o delce n*8 bit� do FPGA s 8-mi bitovou adresou c�lov�ho za��zen�.
 \param rw - povoleni cteni a (nebo) zapisu (SPI_FPGA_ENABLE_WRITE, SPI_FPGA_ENABLE_READ)
 \param addr - 8-mi bitov� adresa c�lov�ho za��zen� v FPGA
 \param data - ukazatel na pole ur�en� k v�m�n� dat s FPGA
 \param n - d�lka bloku k v�m�n�
 \return V�dy 0
 **/
int FPGA_SPI_RW_A8_DN(unsigned char rw, unsigned char addr, unsigned char *data,
                      int n) {
  int i;
  SPI_set_cs_FPGA(1);
  // zapis op. kodu
  SPI_read_write(OC_FPGA|rw);
  // zapis adresy
  SPI_read_write(addr);
  //zapis dat
  if ((rw & SPI_FPGA_ENABLE_READ) == SPI_FPGA_ENABLE_READ) {
     // zapis a cteni dat
     for (i = 0; i < n; i++)
         data[i] = SPI_read_write(data[i]);
  } else {
     // zapis dat
     for (i = 0; i < n; i++)
         SPI_read_write(data[i]);
  }
  SPI_set_cs_FPGA(0);
  return 0;
}


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
                      unsigned char an, unsigned char dn) {
  int i;
  SPI_set_cs_FPGA(1);
  // zapis op. kodu
  SPI_read_write(OC_FPGA | rw);
  // zapis adresy
  for (i = an-1; i >= 0; i--)
    SPI_read_write((unsigned char)((addr >> (i*8)) & 0xFF));

  if ((rw & SPI_FPGA_ENABLE_READ) == SPI_FPGA_ENABLE_READ) {
     // zapis a cteni dat
     for (i = 0; i < dn; i++)
         data[i] = SPI_read_write(data[i]);
  } else {
     // zapis dat
     for (i = 0; i < dn; i++)
         SPI_read_write(data[i]);
  }
  SPI_set_cs_FPGA(0);
  return 0;
}
