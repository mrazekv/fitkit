/*******************************************************************************
   nrf24l01_spi.c: NRF24L01 low-level functions
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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

#include "nrf24l01_spi.h"
#include "nrf24l01_com.h"
#include "fspi.h"

/**
 \brief Inicializace NRF24L01
 **/
char NRF24L01_init(){
//	FSPI_Init(1); //14MHz SPI
	FSPI_Init(10);

	delay_ms(10);	
	control_reg_write(0x01); //EN = 1, CE=0
	delay_ms(500);

   P1DIR &= ~RF_IRQ; //vstup

   return NRF_init();
}



/**
 \brief Operace ètení, vrací status
 **/
unsigned char RF_cmd_read(unsigned char cmd, unsigned char* buf, unsigned char len)
{
  unsigned char status;
  unsigned char i;

  FSPI_set_CS(0); //CS -> 0
  status = FSPI_write_wait_read(cmd); // command
  for (i=0; i < len; i++)
  {
      buf[i] = FSPI_write_wait_read(0xFF); // read data
  }
  FSPI_set_CS(1); //CS -> 1

  return status;
}


/**
 \brief Operace zápisu, vrací status
 **/
unsigned char RF_cmd_write(unsigned char cmd, unsigned char* buf, unsigned char len)
{
  unsigned char status;
  unsigned char i;

  FSPI_set_CS(0); //CS -> 0
  status = FSPI_write_wait_read(cmd); //command
  if (buf) 
  {
     for (i=0; i < len; i++)
         FSPI_write_wait(buf[i]); // write data
  }
  FSPI_set_CS(1); //CS -> 1

  return status;
}

/**
 \brief Ètení z registru, vrací status
 **/
unsigned char RF_reg_read(unsigned char addr, unsigned char* data, unsigned char len)
{
 return RF_cmd_read(R_REGISTER | addr, data, len);
}

/**
 \brief Zápis do registru, vrací status
 **/
inline unsigned char RF_reg_write(unsigned char addr, unsigned char data)
{
  return RF_cmd_write(W_REGISTER | addr, &data, 1);
}

/**
 \brief Zápis vice hodnot do registru, vrací status
 **/
inline unsigned char RF_reg_write_bytes(unsigned char addr, unsigned char data, unsigned char len)
{
  unsigned char status;
  unsigned char i;

  FSPI_set_CS(0); //CS -> 0
  status = FSPI_write_wait_read(W_REGISTER | addr); //command
  for (i=0; i < len; i++)
      FSPI_write_wait(data); // write data
  FSPI_set_CS(1); //CS -> 1

  return status;

}

