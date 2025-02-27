/*******************************************************************************
   enc28j60_spi_tx.h: Knihovna funkcí pro práci s odesílacím bufferem
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

/**
	\file enc28j60_spi_tx.h
	\brief Knihovna funkcí pro práci s odesílacím bufferem v ENC28J60.
*/

#ifndef _ENC28J60_SPI_TX_
#define _ENC28J60_SPI_TX_


//protokoly
#define TCP_PROTO   1	///<TCP protokol
#define UDP_PROTO   2	///<UDP protokol
#define ICMP_PROTO	3	///<ICMP protokol
#define ARP_PROTO	4	///<ARP protokol

/**
 \cond
 **/
 
///TCP protokol - pouze hlavièky
#define TCP_PROTO_HEADER_ONLY 5

//konstanty pro identifikaci hlavicek

#define CTRL		10	///<ID kontrolního bajtu
#define ETH_HEADER	11	///<ID ethernetové hlavièky
#define IP_HEADER	12	///<ID IP hlavièky
#define ICMP_HEADER	13	///<ID ICMP hlavièky
#define TCP_HEADER	14	///<ID TCP hlavièky
#define UDP_HEADER	15	///<ID UDP hlavièky





//délky hlavièek

#define CTRL_LEN		1	///<délka údajù pro nastavení detailù odeslání, patøí pøed každý odesílaný paket
#define TCP_HEADER_LEN	20	///<délka TCP hlavièky
#define UDP_HEADER_LEN	8	///<délka UDP hlavièky
#define ETH_HEADER_LEN	14	///<délka ethernetové hlavièky
#define IP_HEADER_LEN	20	///<délka IP hlavièky
#define ICMP_HEADER_LEN	8	///<délka ICMP hlavièky


/*
#define TX1ST	0			//zacatek UDP paketu
#define TX1ND	1514		//konec mista pro UDP paket, vynechana mezera 7B pro status vektor 
#define TX2ST	1522		//zacatek pomocneho bufferu, pro ARP (a ICMP) dotazy
#define TX2ND	1596		//konec mista pro ARP paket, vynechana mezera 7B pro status vektor
#define TX3ST	1604		//zacatek TCP paketu
#define TX3ND	3118 		//konec mista pro TCP paket
*/

/*
zmena, dodany radic mel obcasne problemy s inicializaci jednoho registru, ERXWRPT,
je pouze pro cteni a inicializuje se automaticky podle hodnoty ERXST, ale obcas
se v nem hodnota ERXST neobjevila, byla tam 0, proto jsem ukazatele zamenil,
aby byla hodnota ERXST a potazmo ERXWRPT 0 a tak i pri teto chybe pracoval radic spravne
*/

#define TX4ST	4974	///< Zaèátek bufferu pro ARP pakety (max. velikost 44B)
#define TX4ND	5018	///< konec bufferu pro ARP pakety, za ním vynechána mezera 7B pro status vektor 
#define TX1ST	5026	///<zaèátek bufferu pro UDP paket (max. velikost 1514B)
#define TX1ND	6540	///<konec bufferu pro UDP paket, za ním vynechána mezera 7B pro status vektor 
#define TX3ST	6548	///<zaèátek bufferu pro TCP paket (max. velikost 1514B)
#define TX3ND	8062	///<konec bufferu pro TCP paket, za ním vynechána mezera 7B pro status vektor
#define TX2ST	8070	///<zaèátek bufferu pro ICMP a TCP automatické odpovìdi (ACK apod.) (max. velikost 114B)
#define TX2ND	8184	///<konec pomocného bufferu, za ním vynechána mezera 7B pro status vektor


#define TIMEOUT 1000	///<doba èekání na doodesílání pøedchozího paketu

//konstanty pro identifikaci aktualniho bufferu
#define TX1	32	///<buffer pro UDP pakety
#define TX2	64	///<buffer pro ICMP
#define TX3	128	///<buffer pro TCP pakety
#define TX4	192	///<buffer pro ARP

/**
 \endcond
 **/

/**
 \brief Inicializace bufferu pro odesílání paketu.
 \param transport_protocol - jedna z konstant \a TCP_PROTO, \a UDP_PROTO a \a ICMP_PROTO
 							\n podle protokolu, kterým se data budou odesílat
 \retval 0 - inicializace probìhla úspìšnì
 \retval -1 - chyba pøi inicializaci - UDP a ICMP - reinicializace bufferu, TCP - nepotvrzení poslednì odeslaného paketu
 			\n ve všech pøípadech mùže nastat chyba pøíèinou nedoodesílání døíve odesílaného paketu - buffer se do úplného odeslání nesmí mìnit  
 **/
char tx_init(unsigned char transport_protocol);


/**
 \brief Funkce vrátí ukazatel pro zápis v odesílacím bufferu na zaèátek (jako po inicializaci).
 		Data se zachovají. Je tak možné provést dodateèné zmìny v paketu pøed odesláním.
 \see tx_skip()
 **/
void tx_rewind();

/**
 \cond
 **/
 
/**
 \brief Funkce slouží pro posunutí v bufferu na pozici pro zápis hlavièek protokolù - interní funkce.
 \param header jedna z konstant \a TCP_HEADER, \a UDP_HEADER, \a IP_HEADER, \a ETH_HEADER podle hlavièky, kterou chceme zapisovat
 **/
void tx_seek(unsigned char header);

/**
 \endcond
 **/

/**
 \brief Uloží pozici konce paketu a tím ho pøipraví na odeslání. 
 		Volání této funkce je dùležité, oznaèí totiž konec paketu.
		Všechna data až po takto oznaèený konec se odešlou, data zapsaná do bufferu za nìj se ignorují.
 **/
inline void tx_close();

/**
 \cond
 **/

/**
 \brief Uloží aktuální stav bufferu - interní funkce.
 \param buffer - jedna z konstant TX1, TX2, TX3 podle bufferu, který chceme uložit
 \see tx_act()
 \see tx_load()
 **/
void tx_save(unsigned char buffer);

/**
 \brief Naète nastavení a pøepne zápis na buffer z parametru - interní funkce.
 \param buffer - jedna z konstant \a TX1, \a TX2, \a TX3 podle bufferu, který chceme obnovit
 \see tx_act()
 \see tx_save()
 **/
void tx_load(unsigned char buffer);

/**
 \brief Vrátí konstantu reprezentující právì používaný buffer - interní funkce.
 \returns jedna z konstant \a TX1, \a TX2 a \a TX3
 **/
unsigned char tx_act();

/**
 \endcond
 **/
 
/**
 \brief Zapíše znak do bufferu.
 \param c znak, který chceme zapsat
 \retval 0 - znak nezapsán - buffer plný
 \retval 1 - znak úspìšnì zapsán
 \see tx_write()
 \see tx_write_str()
 **/
unsigned char tx_putc(char c);

/**
 \brief Zapíše blok dat velikosti \a length z adresy odkazované ukazatelem \a ptr do bufferu pro odeslání.
 \param ptr - ukazatel na data
 \param length - délka dat v bajtech
 \returns poèet zapsaných bajtù
 \see tx_putc()
 \see tx_write_str()
 **/
unsigned int tx_write(void* ptr, unsigned int length);

/**
 \brief Zapíše øetìzec ukonèený hodnotou 0 z adresy odkazované ukazatelem \a str do bufferu pro odeslání.
 \param str - ukazatel na øetìzec
 \returns poèet zapsaných bajtù
 \see tx_putc()
 \see tx_write() 
 **/
#define tx_write_str(str) tx_write(str,enc_strlen(str))

/**
 \brief Pøeskoèí místo v odesílacím bufferu. Vhodné zejména v kombinaci s funkcí tx_rewind(),
 		kdy chceme dodateènì pøepsat nìkteré èásti datagramu.
 \param length - délka skoku v bajtech.
 \returns poèet pøeskoèených bajtù
 **/
unsigned int tx_skip(unsigned int length);

/**
 \brief Vyplní v odesílacím bufferu \a length bajtù hodnotou \a value.
 \param value - hodnota, kterou se bude vyplòovat buffer
 \param length - délka vyplnìní
 \returns poèet zapsaných bajtù
 **/
unsigned int tx_fill(unsigned char value, unsigned int length);

/**
 \cond
 **/
 
/**
 \brief Zprostøedkuje odeslání dat z odesílacího bufferu v ENC28J60 do sítì - interní funkce.
 \retval 0 - odeslání se zdaøilo
 \retval -1 - chyba pøi odesílání
 **/
char tx_send();

/**
 \endcond
 **/

/**
 \brief Vrátí údaj o volném místu v bufferu.
 \returns poèet volných bajtù
 **/
inline unsigned int tx_left();

/**
 \cond
 **/
 
/**
 \brief Vrátí údaj o velikosti zapsaných dat - interní funkce, údaj je platný až po oznaèení konce dat funkcí tx_close().
 \returns poèet volných bajtù
 **/
inline unsigned int tx_data_len();

/**
 \brief Vypoèítá checksum od aktuální pozice až do konce dat v odesílacím bufferu - interní funkce.
 \returns checksum
 **/
unsigned int tx_checksum();

/**
 \brief Odemkne odesílací buffer, ten se pak mùže opìtovnì použít pro odeslání dat - interní funkce. 
 \param buffer - jedna z konstant \a TX1, \a TX2, \a TX3 reprezentujících pøíslušný buffer
 **/
inline void tx_unlock(unsigned char buffer);

/**
 \endcond
 **/

#endif /*_ENC28J60_SPI_TX_*/
