/*******************************************************************************
   nrf24l01_spi.h: NRF24L01 low-level functions
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

/**
	\file nrf24l01_spi.h
	\brief Knihovna funkcí pro komunikaci s NRF24L01 prostøednictvím rozhraní SPI.
*/

#ifndef _NRF24L01_SPI_
#define _NRF24L01_SPI_

/**
 \cond
 **/
#define BASE_ADDR_CTRLREG 0xE0
#define control_reg_write(data) FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_CTRLREG, (data))
#define control_reg_read() (FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ, BASE_ADDR_CTRLREG, 0))

/**
 \endcond
 **/

/**
 \brief Inicializace NRF24L01 (modifikuje port P1)
 \retval 0 - inicializace probìhla úspìšnì
 \retval -1 - chyba pøi inicializaci
 **/
char NRF24L01_init();

/**
 \brief RF_CE_clr - nastavi signal CE na hodnotu log. 0
 **/
#define RF_CE_clr() control_reg_write(0x01)

/**
 \brief RF_CE_set - nastavi signal CE na hodnotu log. 1
 **/
#define RF_CE_set() control_reg_write(0x03)

#define RF_IRQ BIT7

#define RF_is_IRQ() ((P1IN & BIT7) == 0)

//******************************************
//fce cteni/zapis registru
//******************************************

/**
 \brief Operace ètení
 \param cmd - kód operace
 \param buf - cílový buffer
 \param len - poèet bytù
 \returns obsah stavového registru
 **/
unsigned char RF_cmd_read(unsigned char cmd, unsigned char* buf, unsigned char len);


/**
 \brief Operace zápisu
 \param cmd - kód operace
 \param buf - zdrojový buffer
 \param len - poèet bytù
 \returns obsah stavového registru
 **/
unsigned char RF_cmd_write(unsigned char cmd, unsigned char* buf, unsigned char len);

/**
 \brief Ètení obsahu registru
 \param addr - adresa registru
 \param data - cílový buffer
 \param len - poèet bytù
 **/
unsigned char RF_reg_read(unsigned char addr, unsigned char* data, unsigned char len);


/**
 \brief Zápis jednoho bytu do registru
 \param addr - adresa registru
 \param data - zapisovaná hodnota
 **/
inline unsigned char RF_reg_write(unsigned char addr, unsigned char data);

/**
 \brief Zápis n totoznych bytu do registru
 \param addr - adresa registru
 \param data - zapisovaná hodnota
 \param len - poèet bytù
 **/
inline unsigned char RF_reg_write_bytes(unsigned char addr, unsigned char data, unsigned char len);


/**
 \brief Zápis jednoho a více bytù do registru
 \param addr - adresa registru
 \param buf - zdrojový buffer
 \param len - poèet bytù
 **/
#define RF_reg_write_buf(addr, buf, len) RF_cmd_write(W_REGISTER | (addr), (buf), (len))


/**
 \cond
 **/



//******************************************
//SPI INSTRUKCE
//******************************************

#define   NRF_NOP         0xFF
#define   R_REGISTER      0x00
#define   W_REGISTER      0x20
#define   R_RX_PAYLOAD    0x61
#define   W_TX_PAYLOAD    0xA0
#define   W_TX_PAYLOAD_NO_ACK    0xB0
#define   FLUSH_TX        0xE1
#define   FLUSH_RX        0xE2
#define   REUSE_TX_PL     0xE3
#define   R_RX_PL_WID     0x60


//******************************************
//ADRESY REGISTRU
//******************************************
#define   NRF_CONFIG       0x00
#define   NRF_EN_AA        0x01
#define   NRF_EN_RXADDR    0x02
#define   NRF_SETUP_AW     0x03
#define   NRF_SETUP_RETR   0x04
#define   NRF_RF_CH        0x05
#define   NRF_RF_SETUP     0x06
#define   NRF_STATUS       0x07
#define   NRF_OBSERVE_TX   0x08
#define   NRF_RPD          0x09
#define   NRF_RX_ADDR_P0   0x0A
#define   NRF_RX_ADDR_P1   0x0B
#define   NRF_RX_ADDR_P2   0x0C
#define   NRF_RX_ADDR_P3   0x0D
#define   NRF_RX_ADDR_P4   0x0E
#define   NRF_RX_ADDR_P5   0x0F
#define   NRF_TX_ADDR      0x10
#define   NRF_RX_PW_P0     0x11
#define   NRF_RX_PW_P1     0x12
#define   NRF_RX_PW_P2     0x13
#define   NRF_RX_PW_P3     0x14
#define   NRF_RX_PW_P4     0x15
#define   NRF_RX_PW_P5     0x16
#define   NRF_FIFO_STATUS  0x17
#define   NRF_DYNPD        0x1C
#define   NRF_FEATURE      0x1D

//***********************************
//BITY REGISTRU
//***********************************

// status registr
#define   TX_FULL         0x01
#define   MAX_RT          0x10
#define   TX_DS           0x20
#define   RX_DR           0x40

/**
 \endcond
 **/

#endif /*_NRF24L01_SPI_*/
