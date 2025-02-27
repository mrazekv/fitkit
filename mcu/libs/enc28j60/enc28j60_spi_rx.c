/*******************************************************************************
   enc28j60_spi_rx.c: Knihovna funkcí pro práci s pøijímacím bufferem
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Martin Musil <xmusil34 AT fit.vutbr.cz>

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

#include "enc28j60_spi_rx.h"
#include "enc28j60_spi.h"


// pomocne ukazatele 
unsigned int rx_next_pkt;						//ukazatele na dalsi paket
unsigned int rx_pkt_len;						//zbyvajici delka paketu
unsigned char receive_status[STATUS_LEN];		//status vektor
unsigned char rx_initialized;					//zamek bufferu 

//***************************************
//prijimaci buffer
//***************************************

/**
 \brief Inicializace bufferu s pøíchozím paketem - interní funkce.
 **/
char rx_init(){

	select_bank(1);
	
	if((read_reg8(EPKTCNT,NO_DUMMY) > 0) & (rx_initialized == 0)){					//pocet paketu
		rx_initialized = 1;
		select_bank(0);

		spi_read((char*) &rx_next_pkt, PTR_LEN);		//zapamatovani next_packet pointeru;
		spi_read(&receive_status,STATUS_LEN);			//precteni status-vektoru

		unsigned int rec_len = *receive_status;

		int ERDPT_tmp = read_reg16(ERDPT,NO_DUMMY);

		if(ERDPT_tmp > rx_next_pkt){												//paket je na "hrane" bufferu (cast na konci pameti,cast na zacatku)
			rx_pkt_len = RX_END - ERDPT_tmp + rx_next_pkt - RX_START;					//vypocet delky paketu
		}
		else{
			rx_pkt_len = rx_next_pkt - ERDPT_tmp;
		}

		if((rec_len & 0x0001) > 0)		//licha delka prijateho ramce - konec ramce je doplnen, aby zacatek noveho ramce lezel na sude adrese
			rx_pkt_len -= 5;	// od delky odecteme 4 bajty CRC a bajtovou mezeru mezi pakety
		else	
			rx_pkt_len -= 4;	   	   		   	 					
	}
	else return -1;

return 0;
}

/**
 \brief Uzavøe buffer a uvolní místo pro další pøíchozí pakety - interní funkce.
 **/
char rx_free(){
	
	if(rx_initialized == 1){
		select_bank(0);

		write_reg16(ERDPT,rx_next_pkt);//nastaveni ERDPT na next_packet adresu;

		write_reg16(ERXRDPT,rx_next_pkt);//posunuti ERXRDPT --> uvolneni pameti

		bf_set(ECON2,ECON2_PKTDEC);
		rx_initialized = 0;
		return 0;
	}
return -1;
}

/**
 \brief Pøeète bajt z bufferu.
 **/
inline unsigned int rx_getc(){
	   
	if(rx_pkt_len--)
		return (unsigned int) spi_getc();//kontrola mezi paketu;
	else
		return -1;
};

/**
 \brief Pøeète blok dat velikosti \a length z bufferu a zapíše na adresu odkazovanou ukazatelem \a ptr.
 **/
unsigned int rx_read(void* ptr, int length){

	if(rx_pkt_len == 0)
		return 0;
	else if(length >= rx_pkt_len){
		int bytes_readed = rx_pkt_len;
		spi_read(ptr, rx_pkt_len);	
		rx_pkt_len = 0;
		return bytes_readed;
	}
	else{
		spi_read(ptr, length);
		rx_pkt_len -= length;
		return length;
	}
}

/**
 \brief Pøeète øetìzec ukonèený hodnotou LF z bufferu a zapíše na adresu odkazovanou ukazatelem \a ptr.
 **/
unsigned int rx_getline(void* ptr, int max_length){

	max_length--;

	if(rx_pkt_len == 0){
		((char*)ptr)[0] = 0;	
		return 0;
	}
	else if(max_length >= rx_pkt_len){
		int bytes_readed = spi_getline(ptr, rx_pkt_len);
		((char*)ptr)[bytes_readed] = 0;	
		rx_pkt_len -= bytes_readed;
		return bytes_readed;
	}
	else{
		int bytes_readed = spi_getline(ptr, max_length);
		((char*)ptr)[bytes_readed] = 0;
		rx_pkt_len -= bytes_readed;
		return bytes_readed;
	}
}

/**
 \brief Vrátí poèet zbývajících bajtù v bufferu.
 **/
inline int rx_left(){

return rx_pkt_len;
}

/**
 \brief Kontroluje zda je buffer pøeèten celý.
 **/
inline unsigned char rx_isempty(){

return (rx_pkt_len > 0) ? 0 : 1;
}	

/**
 \brief oøízne padding paketu (doplnìní nulami na délku 60 bajtù u ethernetu) - interní funkce.
 **/
inline void rx_cut_padding(unsigned char length){
	
	rx_pkt_len -= length;
return;
} 		
