/*******************************************************************************
   enc28j60_spi_tx.c: Knihovna funkcí pro práci s odesílacím bufferem
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

#include "enc28j60_spi_tx.h"
#include "enc28j60_spi.h"
#include "fspi.h"

//hlavni ukazatele pro praci s buffery
unsigned int tx_ptr;
unsigned int tx_end_ptr;
unsigned int TXST;
unsigned int TXND;

//promenna, znaci aktualni buffer
unsigned char act_tx;

//zamek bufferu
#define LOCKED 1
#define UNLOCKED 0
char tx1_lock = UNLOCKED;
char tx2_lock = UNLOCKED;
char tx3_lock = UNLOCKED;

//promenne pro ulozeni protokolu, ktery se pres ne odesila
char tx1_protocol;
char tx2_protocol;
char tx3_protocol;

//ukazatele pro jednotlive buffery, slouzi pro uchovavani hodnot behem prepinani
unsigned int tx1_ptr;			
unsigned int tx2_ptr;
unsigned int tx3_ptr;

unsigned int tx1_end_ptr;
unsigned int tx2_end_ptr;
unsigned int tx3_end_ptr;

//vnitrni pomocne funkce
char tx_protocol(unsigned char protocol);


//************************************************
//odesilaci buffer
//************************************************

/**
 \brief Inicializace bufferu pro odesílání paketu.
 **/
char tx_init(unsigned char protocol){
	
	int timer = TIMEOUT;					//timeout
		
	//kontrola dokonceni odesilani predchoziho paketu
	while((read_reg8(ECON1,NO_DUMMY) & ECON1_TXRTS) && (timer-- > 0) );
		
	if(timer == 0)						//timeout vyprsel
		return -1;	
			
	//buffer pro UDP pakety
	if(protocol == UDP_PROTO){
		
		if(tx1_lock == LOCKED){					//tx1 ma zamek,aby nedoslo k nahodnemu prepsani jeste
			return -1;							//neodeslaneho obsahu bufferu jinym. napr. pri odesilani  
		}										//paketu z obsluhy preruseni	

		tx1_lock = LOCKED;
		tx_save(act_tx);
		
		TXST = TX1ST;							//restart ukazatelu na zacatek a konec vyhrazene pameti
		TXND = TX1ND;
		act_tx = TX1;							//bufferem je TX1
	
		select_bank(0);
		
		write_reg16(ETXST,TXST);				// zapsani ukazatele na zacatek bufferu do ENC28
		write_reg16(ETXND,TXST);
		
		tx_ptr = TX1ST;							//inicializace ukazatele do bufferu
		tx1_ptr	= TX1ST;						//nastaveni pomocnych ukazatelu (jsou pro 
		tx1_end_ptr	= TX1ST;					// uchovani hodnot behem prepinani funkcemi tx_save a tx_load)
		tx1_protocol = protocol;
		
		return tx_protocol(protocol);
	}
	
	//buffer pro TCP pakety
	else if(protocol == TCP_PROTO){
		
		if(tx3_lock == LOCKED){					//tx3 ma zamek, zde u TCP slouzi k tomu, aby nebylo mozne
			return -1;							//odesilat dalsi paket, dokud predchozi nebyl odeslan  
		}										//   zamek odemyka stavovy automat TCP spojeni
		tx3_lock = LOCKED;
		
		tx_save(act_tx);
		
		TXST = TX3ST;							//restart ukazatelu na zacatek a konec vyhrazene pameti
		TXND = TX3ND;
		act_tx = TX3;							//bufferem je TX3
	
		select_bank(0);
		
		write_reg16(ETXST,TXST);				// zapsani ukazatele na zacatek bufferu do ENC28
		write_reg16(ETXND,TXST);
		
		tx_ptr = TX3ST;							//inicializace ukazatele do bufferu
		tx3_ptr	= TX3ST;						//nastaveni pomocnych ukazatelu (jsou pro 
		tx3_end_ptr	= TX3ST;					// uchovani hodnot behem prepinani funkcemi tx_save a tx_load)
		tx3_protocol = protocol;
		
		return tx_protocol(protocol);
	}
		
	//buffer pro automaticke odpovedi a ARP dotazy
	else if((protocol == ICMP_PROTO) | (protocol == TCP_PROTO_HEADER_ONLY)){
		
		if(tx2_lock == LOCKED){					//tx2 ma zamek,aby nedoslo k nahodnemu prepsani jeste
			return -1;							//neodeslaneho obsahu bufferu jinym. napr. pri odesilani  
		}										//paketu z obsluhy preruseni	

		tx2_lock = LOCKED;
		tx_save(act_tx);		
		
		TXST = TX2ST;							//restart ukazatelu na zacatek a konec vyhrazene pameti
		TXND = TX2ND;
		act_tx = TX2;							//bufferem je TX2
	
		select_bank(0);
		
		write_reg16(ETXST,TXST);				// zapsani ukazatele na zacatek bufferu do ENC28
		write_reg16(ETXND,TXST);				// --||-- konec
		
		tx_ptr = TX2ST;							//inicializace ukazatele do bufferu
		tx2_ptr = TX2ST;						//nastaveni pomocnych ukazatelu (jsou pro 
		tx2_end_ptr = TX2ST;					// uchovani hodnot behem prepinani funkcemi tx_save a tx_load)
		tx2_protocol = protocol;
		
		return tx_protocol(protocol);
	}
	else if((protocol == ARP_PROTO)){
		
		tx_save(act_tx);		
		
		TXST = TX4ST;							//restart ukazatelu na zacatek a konec vyhrazene pameti
		TXND = TX4ND;
		act_tx = TX4;							//bufferem je TX4
	
		select_bank(0);
		
		write_reg16(ETXST,TXST);				// zapsani ukazatele na zacatek bufferu do ENC28
		write_reg16(ETXND,TXST);				// --||-- konec
		
		tx_ptr = TX4ST;							//inicializace ukazatele do bufferu
		
		return tx_protocol(protocol);
	}
	
	
return -1;
}

/*Fce pro preskoceni mista pro hlavicky podle pouziteho protokolu*/
char tx_protocol(unsigned char protocol){
	
	switch(protocol){
		case TCP_PROTO_HEADER_ONLY:
		case TCP_PROTO:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN + IP_HEADER_LEN + TCP_HEADER_LEN;
			break;	
		case UDP_PROTO:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN;		
			break;
		case ICMP_PROTO:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN + IP_HEADER_LEN + ICMP_HEADER_LEN;		
			break;		
		case ARP_PROTO:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN;			
			break;		
		default:
			return -1;	
	}
	
	select_bank(0);
	write_reg16(EWRPT,tx_ptr);							//posun na zacatek datove casti
		
return 0;
}	

/**
 \brief Funkce vrátí ukazatel pro zápis v odesílacím bufferu na zaèátek (jako po inicializaci).
 		Data se zachovají. Je tak možné provést dodateèné zmìny v paketu pøed odesláním.
 **/
void tx_rewind(){
	
	switch(act_tx){
		case TX1:
			tx_protocol(tx1_protocol);
			break;
		case TX2:
			tx_protocol(tx2_protocol);
			break;
		case TX3:
			tx_protocol(tx3_protocol);
			break;	
	}
}



/**
 \brief Funkce slouží pro posunutí v bufferu na pozici pro zápis hlavièek protokolù - interní funkce.
 **/
void tx_seek(unsigned char header){			
	switch(header){
		case CTRL:
			tx_ptr = TXST;
			break;
		case ETH_HEADER:
			tx_ptr = TXST + CTRL_LEN;
			break;
		case IP_HEADER:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN;
			break;
		case ICMP_HEADER:
		case TCP_HEADER:
		case UDP_HEADER:
			tx_ptr = TXST + CTRL_LEN + ETH_HEADER_LEN + IP_HEADER_LEN;
			break;
	}
	
	select_bank(0);
	write_reg16(EWRPT,tx_ptr);
return;
}	

/**
 \brief Uloží pozici konce paketu a tím ho pøipraví na odeslání. 
 		Volání této funkce je dùležité, oznaèí totiž konec paketu.
		Všechna data až po takto oznaèený konec se odešlou, data zapsaná do bufferu za nìj se ignorují.
 **/
inline void tx_close(){	
	select_bank(0);
	tx_end_ptr = tx_ptr;
	write_reg16(ETXND,tx_end_ptr-1);		//tx_end_ptr ukazatel ukazuje na misto, kde jeste neni nic zapsano, proto -1
}											//ETXND musi ukazovat na posledni bajt dat


/**
 \brief Uloží aktuální stav bufferu - interní funkce.
 **/
void tx_save(unsigned char buffer){
		
		//ulozeni stavu predchoziho bufferu
		switch(buffer){
			case TX1:
				tx1_ptr = tx_ptr;		//ulozeni stavu TX1
				tx1_end_ptr	= tx_end_ptr;
				break;
				
			case TX2:
				tx2_ptr = tx_ptr;		//ulozeni stavu TX2
				tx2_end_ptr	= tx_end_ptr;
				break;
								
			case TX3:
				tx3_ptr = tx_ptr;		//ulozeni stavu TX3
				tx3_end_ptr	= tx_end_ptr;
				break;
		}
return;
}


/**
 \brief Naète nastavení a pøepne zápis na buffer z parametru - interní funkce.
 **/
void tx_load(unsigned char buffer){
	
		switch(buffer){
			case TX1:	
				TXST = TX1ST;			//obnoveni stavu pro TX1
				TXND = TX1ND;
				tx_ptr = tx1_ptr;
				tx_end_ptr = tx1_end_ptr;
				act_tx = TX1;
				break;
			
			case TX2:
				TXST = TX2ST;			//obnoveni stavu pro TX2
				TXND = TX2ND;
				tx_ptr = tx2_ptr;
				tx_end_ptr = tx2_end_ptr;
				act_tx = TX2;
				break;
				
			case TX3:
				TXST = TX3ST;			//obnoveni stavu pro TX3
				TXND = TX3ND;
				tx_ptr = tx3_ptr;
				tx_end_ptr = tx3_end_ptr;
				act_tx = TX3;
				break;
		}			
	
	write_reg16(EWRPT,tx_ptr);		// nastaveni ukazatele pro zapis
	write_reg16(ETXST,TXST);		// zapsani ukazatele na zacatek bufferu do ENC28
	write_reg16(ETXND,tx_end_ptr-1);		//tx_end_ptr ukazatel ukazuje na misto, kde jeste neni nic zapsano, proto -1
											//ETXND musi ukazovat na posledni bajt dat		

return;	
}

/**
 \brief Vrátí konstantu reprezentující právì používaný buffer - interní funkce.
 **/
unsigned char tx_act(){
	
	return act_tx;	
}

/**
 \brief Zapíše znak do bufferu.
 **/
unsigned char tx_putc(char c){
	
	if(tx_ptr > TXND)
		return 0;   
	else{
		spi_putc(c);
		tx_ptr++;
		return 1;
	}
}

/**
 \brief Zapíše blok dat velikosti \a length z adresy odkazované ukazatelem \a ptr do bufferu pro odeslání.
 **/
unsigned int tx_write(void* ptr, unsigned int length){

	int remaining = tx_left();

	if(tx_ptr > TXND)
		return 0;
	else if(length >= remaining){
		spi_write(ptr, remaining);
		tx_ptr += remaining;	
		return remaining;
	}
	else{
		spi_write(ptr, length);
		tx_ptr += length;
		return length;
	}
return 0;	
}

/**
 \brief Vyplní v odesílacím bufferu \a length bajtù hodnotou \a value.
 **/
unsigned int tx_fill(unsigned char value, unsigned int length){
	
	int remaining = tx_left();
	
	if(tx_ptr > TXND)
		return 0;
	else if(length >= remaining){
		spi_fill(value, remaining);
		tx_ptr += remaining;	
		return remaining;
	}
	else{
		spi_fill(value, length);
		tx_ptr += length;
		return length;
	}
return 0;
}

/**
 \brief Pøeskoèí místo v odesílacím bufferu. Vhodné zejména v kombinaci s funkcí tx_rewind(),
 		kdy chceme dodateènì pøepsat nìkteré èásti datagramu.
 **/
unsigned int tx_skip(unsigned int length){
	
	int remaining = tx_left();
	
	if(tx_ptr > TXND)
		return 0;
	else if(length >= remaining){
		tx_ptr += remaining;
		write_reg16(EWRPT,tx_ptr);	
		return remaining;
	}
	else{
		tx_ptr += length;
		write_reg16(EWRPT,tx_ptr);
		return length;
	}
return 0;
}

/**
 \brief Zprostøedkuje odeslání dat z odesílacího bufferu v ENC28J60 do sítì - interní funkce.
 **/
char tx_send(){	

	int timer = TIMEOUT;					//timeout
	
	while(( (read_reg8(ECON1,NO_DUMMY) & ECON1_TXRTS) > 0) & (timer-- > 0) );	//kontrola odeslani predchoziho paketu

	
	if(timer == 0){						//timeout vyprsel
		if(act_tx == TX1){				//odemknuti TX1 po nevydarenem odeslani
			tx_unlock(TX1);
		}
		else if(act_tx == TX2){
			tx_unlock(TX2);	
		}
		return -1;
	}

	// prepsani registru s nastavenim  paddingu,crc... sam od sebe se nejak "maze"
	//mozna hw zavada vybranyho kousku
	select_bank(2);
	write_reg8(MACON3,MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FULDPX);

	bf_set(ECON1,ECON1_TXRTS);					

	if(act_tx == TX1){					//odemknuti TX1
		tx_unlock(TX1);
	}
	else if(act_tx == TX2){
		tx_unlock(TX2);	
	}
return 0;	
}


/**
 \brief Vrátí údaj o volném místu v bufferu.
 **/
inline unsigned int tx_left(){

return TXND - tx_ptr +1;	//kompenzace, TXND je posledni bajt zpravy, tx_ptr tam musi umoznit zapisovat
}

/**
 \brief Vrátí údaj o velikosti zapsaných dat - interní funkce, údaj je platný až po uzavøení bufferu funkcí tx_close().
 **/
inline unsigned int tx_data_len(){
	
return tx_end_ptr - tx_ptr; 
}

/**
 \brief Vypoèítá checksum od aktuální pozice až do konce dat v odesílacím bufferu - interní funkce.
 **/
unsigned int tx_checksum(){

	//vyuziti vnitrniho DMA radice v ENC28J60 pro spocitani checksumu
	select_bank(0);
	write_reg16(EDMAST,tx_ptr);
	write_reg16(EDMAND,tx_end_ptr -1);
	bf_clear(EIR,EIR_DMAIF);
	
	bf_set(ECON1,ECON1_CSUMEN | ECON1_DMAST);
	
	while((read_reg8(EIR,NO_DUMMY) & EIR_DMAIF ) == 0);
		
	return read_reg16(EDMACS,NO_DUMMY);

}


/**
 \brief Odemkne odesílací buffer, ten se pak mùže opìtovnì použít pro odeslání dat - interní funkce. 
 **/
inline void tx_unlock(unsigned char buffer){
	
	if(buffer == TX1)
		tx1_lock = UNLOCKED;
	else if(buffer == TX2)
		tx2_lock = UNLOCKED;
	else if(buffer == TX3)
		tx3_lock = UNLOCKED;
		
return;
}
