/*******************************************************************************
   nrf24l01_com.h: NRF24L01 high-level API
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

#include "fspi.h"
#include "nrf24l01_spi.h"
#include "nrf24l01_com.h"

char tx_active;
char reset_rx0addr;

/**
 \brief Inicializace NRF24L01
 **/
char NRF_init(){
  unsigned char status;
  char buf[32];

  tx_active = 0;
  reset_rx0addr = 0;

  // Discard transmission
  RF_CE_clr();
 
  delay_ms(200);

  // power down
  status = RF_reg_write(NRF_CONFIG, 0x00);

  // pockat min 1.5ms
  delay_ms(5);

  // Flush TX FIFO
  RF_cmd_write(FLUSH_TX, 0, 0);
  // Flush RX FIFO
  RF_cmd_write(FLUSH_RX, 0, 0);

  // Channel 1
  RF_reg_write(NRF_RF_CH, 0x01);
  
  // Write CONFIG register (addres - 0x00)
  // 00001010 - CRC enable, power-up, prim_RX
  RF_reg_write(NRF_CONFIG, 0x0B);

  // pockat min 1.5ms
  delay_ms(5);

  // Clear TX_DS bit in status register
  RF_reg_write(NRF_STATUS, TX_DS | MAX_RT | RX_DR);
  // Flush TX FIFO
  RF_cmd_write(FLUSH_TX, 0, 0);
  // Flush RX FIFO
  RF_cmd_write(FLUSH_RX, 0, 0);

  status = RF_reg_write(NRF_CONFIG, 0x0B);
  if (status == 0x0E) return 1;

  term_send_str("NRF_setup failed, status: "); term_send_hex(status); term_send_crlf();

  return 0;
}

void NRF_config(char channel, char payload_len, char recv_addr[5], char rst_rx0addr)
{
   reset_rx0addr = rst_rx0addr;

   // Select RF channel
   RF_reg_write(NRF_RF_CH, channel);

   // Set payload length
	RF_reg_write(NRF_RX_PW_P0, payload_len & 0xF);
	RF_reg_write(NRF_RX_PW_P1, payload_len & 0xF);

   //Write RX_ADDR_P1 register -> Set receive address data Pipe1 -> address in RX_ADDRESS_P1 array
   RF_reg_write_buf(NRF_RX_ADDR_P1, &(recv_addr[0]), 5);

   //enable auto ack
   RF_reg_write(NRF_EN_AA, 0x2F);
   //enable retransmit (15x)
   RF_reg_write(NRF_SETUP_RETR, 0x0F);  
   
   // Start receiver
   NRF_enableRx();

   // Flush RX FIFO
   RF_cmd_write(FLUSH_RX, 0, 0);
}

void NRF_enableRx() 
{
   // Start receiver 
   tx_active = 0;
   RF_CE_clr();
   RF_reg_write(NRF_CONFIG, 0x0B);
   if (reset_rx0addr)
      RF_reg_write_bytes(NRF_RX_ADDR_P0, 0, 5);
   RF_CE_set();
   RF_reg_write(NRF_STATUS, (TX_DS | MAX_RT));
}


inline void NRF_setTXAddress(char tx_addr[5])
{
   //Write RX_ADDR_P0 register -> Set receive address data Pipe0 -> address in RX_ADDRESS_P0 array
   RF_reg_write_buf(NRF_RX_ADDR_P0, &(tx_addr[0]), 5);

   //Write TX_ADDR register -> Transmit address. Used for a PTX device only. Address in TX_ADDRESS array
   RF_reg_write_buf(NRF_TX_ADDR, &(tx_addr[0]), 5);
}


inline char NRF_dataReady() 
{
   unsigned char status;
   status = RF_cmd_write(NRF_NOP, 0, 0);
   return (status & RX_DR);
}


inline char NRF_getByte()
{
   char buf[16];      
   //get payload  
   RF_cmd_read(R_RX_PAYLOAD, &(buf[0]), 1);
   //reset status bit
   RF_reg_write(NRF_STATUS, RX_DR);
   return buf[0];
}

inline void NRF_sendByte(char byte) 
{
    unsigned char status; int t;

    status = RF_cmd_write(NRF_NOP, 0, 0);

    //cekani, az je odeslan predchozi paket
    while (tx_active) {
	    if ((status & (TX_DS | MAX_RT)) != 0) {
		    tx_active = 0;
		    break;
	    }
       status = RF_cmd_write(NRF_NOP, 0, 0);
    } 

    RF_CE_clr();

    tx_active = 1;

    RF_reg_write(NRF_CONFIG, 0x0A);
    RF_cmd_write(FLUSH_TX, 0, 0);
 
    status = RF_cmd_write(W_TX_PAYLOAD, &byte, 1);

    //odeslat data
    RF_CE_set();
}

inline char NRF_isSending()
{
   unsigned char status;

   if (tx_active) {
      status = RF_cmd_write(NRF_NOP, 0, 0);
      if ((status & (TX_DS | MAX_RT)) != 0) {
			NRF_enableRx();
			return 0; 
		}
      return 1;
	}
	return 0;
}

inline char NRF_isRxEmpty()
{
   unsigned char regd;

   RF_reg_read(NRF_FIFO_STATUS, &regd, 1);
	return (regd & 1);
}
