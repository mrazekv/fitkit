/*******************************************************************************
   enc28j60_spi.c: Knihovna funkcí pro komunikaci s ENC28J60
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

#include "enc28j60_spi.h"
#include "enc28j60_arp.h"
#include "fspi.h"


unsigned char current_bank = 0;
extern unsigned char local_mac[MAC_LEN];
extern unsigned int rx_next_pkt;
char enc_initialized = 0;
/**
 \brief Inicializace a start ENC28J60
 **/
char ENC28J60_init(){

	FSPI_Init(1); //14MHz SPI
	delay_ms(10);	
	control_reg_write(0x01); //EN = 1, RST=0
	delay_ms(10);
	control_reg_write(0x03); //EN = 1, RST=1
	delay_ms(10);

	enc_reset();
	delay_ms(10);
	
	bf_clear(ECON1, ECON1_RXEN);
		
	select_bank(0);
	write_reg16(ERXST,RX_START);		//zacatek RX bufferu
	write_reg16(ERXND,RX_END);			//konec RX bufferu
	write_reg16(ERXRDPT,RX_END);		//ptr pro cteni z bufferu
	write_reg16(ERDPT,RX_START);		//ptr pro zapis do bufferu

	arp_table_clear();
	rx_next_pkt = RX_START;

	bf_set(ECON2, ECON2_AUTOINC);		//autoinkrementace buffer-pointeru a 
	bf_clear(ECON2,ECON2_PWRSV);		//probuzeni
	delay_ms(10);						//cekani na ustaleni oscilatoru


	select_bank(1);
	write_reg8(ERXFCON, 0xA1);			//vstupni filter - uni/broadcast

	select_bank(2);						//banka 2
	//inicializace MAC
	write_reg8(MACON2,0);
	write_reg8(MACON1,MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);	//nastaveni TXPAUS, RXPAUS, MARXEN
	write_reg8(MACON3,MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FULDPX);	//nastaveni PADCFG, TXCRCEN, FULDPX
	write_reg8(MACON4,MACON4_DEFER);		//nastaveni DEFER
	
	write_reg16(MAMXFL,0x05EE);			//max. velikost ramce 1518B
	write_reg8(MABBIPG,0x15);					//Back-to-Back Inter-Packet Gap - pro full duplex
	write_reg8(MAIPGL, 0x12);
	
	write_phy_reg16(PHCON1,PHCON1_PDPXMD);  //pro full duplex
		
	write_phy_reg16(PHIE, PHIE_PLNKIE | PHIE_PGEIE);
	read_phy_reg16(PHIR);

	select_bank(3);		   				//banka 3
	write_reg8(MAADR1, local_mac[0]);	//nastaveni MAC adresy
	write_reg8(MAADR2, local_mac[1]);
	write_reg8(MAADR3, local_mac[2]);
	write_reg8(MAADR4, local_mac[3]);
	write_reg8(MAADR5, local_mac[4]);
	write_reg8(MAADR6, local_mac[5]);

	write_reg8(EIE,0);					//zakazani preruseni
	bf_set(ECON1, ECON1_RXEN);			//povoleni vstupu

	enc_initialized = 1;

return 0;
}



/**
 \brief Pøeète 8-bitový kontrolní registr
 **/
inline unsigned char read_reg8(unsigned char addr, unsigned char read_dummy){
	
	unsigned char data;

	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_RCR | addr);

	if (read_dummy)
		FSPI_write_wait(DUMMY);
	
	data = FSPI_write_wait_read(DUMMY);
	FSPI_set_CS(1); //CS -> 1

   return data;
}

/**
 \brief Pøeète 16-bitový kontrolní registr
 **/
inline unsigned int read_reg16(unsigned char addr, unsigned char read_dummy){

return read_reg8(addr,read_dummy) | (read_reg8(addr+1,read_dummy) << 8);
}

/**
 \brief Zapíše data do 8-bitového kontrolního registru
 **/
inline void write_reg8(unsigned char addr, unsigned char data){
	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_WCR | addr);
	FSPI_write_wait(data);
	FSPI_set_CS(1); //CS -> 1
return;
}

/**
 \brief Zapíše data do 16-bitového kontrolního registru
 **/
inline void write_reg16(unsigned char addr, unsigned int data){
		
	write_reg8(addr,(char)data);
	write_reg8(addr+1,(char)(data>>8));
	
}

/**
 \brief Pøeète 16-bitový PHY registr
 **/
inline int read_phy_reg16(unsigned char addr){

	select_bank(2);	
	write_reg8(MIREGADR, addr);
	write_reg8(MICMD, MICMD_MIIRD);
	
 	while(read_reg8(MISTAT,DUMMY) & MISTAT_BUSY);
	write_reg8(MICMD, 0);
	
return read_reg16(MIRD,DUMMY);			
}

/**
 \brief Zapíše data do 16-bitového PHY registru
 **/
inline void write_phy_reg16(unsigned char addr, unsigned int data){
	
	select_bank(2);
	write_reg8(MIREGADR, addr);
	write_reg16(MIWR, data);

return;
}

/**
 \brief BFS (Bit Field Set) - Nastaví bity v registru podle masky, operace OR
 **/
inline void bf_set(unsigned char addr, unsigned char mask){
   FSPI_set_CS(0); //CS -> 0
   FSPI_write_wait(ENC_BFS | addr);
   FSPI_write_wait(mask);
   FSPI_set_CS(1); //CS -> 1
}

/**
 \brief BFC (Bit Field Clear) - Vynuluje bity v registru podle masky, operace NOT AND
 **/
inline void bf_clear(unsigned char addr, unsigned char mask){
   FSPI_set_CS(0); //CS -> 0
   FSPI_write_wait(ENC_BFC | addr);
   FSPI_write_wait(mask);
   FSPI_set_CS(1); //CS -> 1
}

/**
 \brief Resetuje ENC28J60
 **/
inline void enc_reset(){
   FSPI_set_CS(0); //CS -> 0
   FSPI_write_wait(ENC_RST);
   FSPI_set_CS(1); //CS -> 1
   enc_initialized = 0;
}

/**
 \brief Pøepne banku registrù v ENC28J60
 **/
inline void select_bank(unsigned char bank){
	
	if(bank < 4 && current_bank != bank){
		current_bank = bank;
		bf_clear(ECON1, 0x03);
		bf_set(ECON1, bank);
	}
return;
}


//******************************************
//fce cteni/zapis do bufferu
//******************************************

/**
 \brief Pøeète bajt z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 **/
unsigned char spi_getc(){
	
	unsigned char data;

	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_RBM);
	data = FSPI_write_wait_read(DUMMY);
	FSPI_set_CS(1); //CS -> 1

   return data;
}

/**
 \brief Pøeète blok pamìti z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 **/
unsigned int spi_read(void* ptr, unsigned int length){

	if(ptr != NULL){						//cteni do pole
		unsigned char* ptr_temp = ptr;
	
		FSPI_set_CS(0); //CS -> 0
		FSPI_write_wait(ENC_RBM);
	
		while(length--)
			*ptr_temp++ = FSPI_write_wait_read(DUMMY);
	
		FSPI_set_CS(1); //CS -> 1
	}
	else{									//zahazovani
		FSPI_set_CS(0); //CS -> 0
		FSPI_write_wait(ENC_RBM);
	
		while(length--)
			FSPI_write_wait_read(DUMMY);
	
		FSPI_set_CS(1); //CS -> 1
	}
return length;
}

/**
 \brief Pøeète øádek textu ukonèený znakem LF z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 		\n vhodné pro textové protokoly jako je napø. telnet
 **/
unsigned int spi_getline(void* ptr, unsigned int max_length){

	unsigned char c;
	unsigned int countdown = max_length;

	if(ptr != NULL){						//cteni do pole
		unsigned char* ptr_temp = ptr;
	
		FSPI_set_CS(0); //CS -> 0
		FSPI_write_wait(ENC_RBM);
	
	
		while(countdown != 0){
			c = FSPI_write_wait_read(DUMMY);
			*ptr_temp++ = c;
			countdown--;
			
			if(c == 10)						// 10 - Line Feed
				break;
		}
		FSPI_set_CS(1); //CS -> 1
	}
	else{									//zahazovani
		FSPI_set_CS(0); //CS -> 0
		FSPI_write_wait(ENC_RBM);
	
		while(countdown != 0){
			c = FSPI_write_wait_read(DUMMY);
			countdown--;
			
			if(c == 10)						// 10 - Line Feed
				break;
		}
		FSPI_set_CS(1); //CS -> 1
	}
	
return max_length - countdown;
}

/**
 \brief Zapíše bajt do bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 **/
void spi_putc(unsigned char c){

	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_WBM);
	FSPI_write_wait(c);

	FSPI_set_CS(1); //CS -> 1

}

/**
 \brief Zapíše blok pamìti do bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 **/
void spi_write(void* ptr, unsigned int length){
	
	unsigned char* ptr_temp = ptr;
	
	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_WBM);
	
	while(length--)
		FSPI_write_wait(*ptr_temp++);
	
	FSPI_set_CS(1); //CS -> 1

}

/**
 \brief Vyplní èást bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 **/
void spi_fill(unsigned char value, unsigned int length){
	
	FSPI_set_CS(0); //CS -> 0
	FSPI_write_wait(ENC_WBM);
	
	while(length--)
		FSPI_write_wait(value);
	
	FSPI_set_CS(1); //CS -> 1	
}

/**
 \brief Zjistí zmìnu stavu linky (vypnuto->zapnuto a opaènì), èasovì nenároèná operace
 **/
inline unsigned char link_change(){
	
	if((read_reg8(EIR,NO_DUMMY) & EIR_LINKIF) > 0){
		read_phy_reg16(PHIR);
		return 1;
	}

return 0;	
}

/**
 \brief Vrátí stav linky, èasovì hodnì nároèná operace(~1ms) - ètení PHY registrù z ENC28J60
 **/
inline unsigned char link_up(){
	
	//read_phy_reg16(PHIR);			//smazani priznaku zmeny linky
	
	if((read_phy_reg16(PHSTAT2) & PHSTAT2_LSTAT) == 0 ){		//kabel odpojen
		return 0;
	}
	
return 1;
}

