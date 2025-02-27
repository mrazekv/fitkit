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

/**
	\file nrf24l01_com.h
	\brief Knihovna funkcí pro komunikaci s NRF24L01
*/

#ifndef _NRF24L01_COM_
#define _NRF24L01_COM_

/**
 \brief Inicializace NRF24L01
 \retval 0 - inicializace probìhla úspìšnì
 \retval -1 - chyba pøi inicializaci
 **/
char NRF_setup();

void NRF_config(char channel, char payload_len, char recv_addr[5], char rst_rx0addr);
inline void NRF_setTXAddress( char tx_addr[5]);
inline char NRF_isSending();
inline char NRF_isRxEmpty();
inline void NRF_sendByte(char ch);
inline char NRF_getByte();
inline char NRF_dataReady();
void NRF_enableRx();

#endif /*_NRF24L01_COM_*/
