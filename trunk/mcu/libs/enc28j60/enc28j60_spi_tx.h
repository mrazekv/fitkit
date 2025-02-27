/*******************************************************************************
   enc28j60_spi_tx.h: Knihovna funkc� pro pr�ci s odes�lac�m bufferem
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
	\brief Knihovna funkc� pro pr�ci s odes�lac�m bufferem v ENC28J60.
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
 
///TCP protokol - pouze hlavi�ky
#define TCP_PROTO_HEADER_ONLY 5

//konstanty pro identifikaci hlavicek

#define CTRL		10	///<ID kontroln�ho bajtu
#define ETH_HEADER	11	///<ID ethernetov� hlavi�ky
#define IP_HEADER	12	///<ID IP hlavi�ky
#define ICMP_HEADER	13	///<ID ICMP hlavi�ky
#define TCP_HEADER	14	///<ID TCP hlavi�ky
#define UDP_HEADER	15	///<ID UDP hlavi�ky





//d�lky hlavi�ek

#define CTRL_LEN		1	///<d�lka �daj� pro nastaven� detail� odesl�n�, pat�� p�ed ka�d� odes�lan� paket
#define TCP_HEADER_LEN	20	///<d�lka TCP hlavi�ky
#define UDP_HEADER_LEN	8	///<d�lka UDP hlavi�ky
#define ETH_HEADER_LEN	14	///<d�lka ethernetov� hlavi�ky
#define IP_HEADER_LEN	20	///<d�lka IP hlavi�ky
#define ICMP_HEADER_LEN	8	///<d�lka ICMP hlavi�ky


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

#define TX4ST	4974	///< Za��tek bufferu pro ARP pakety (max. velikost 44B)
#define TX4ND	5018	///< konec bufferu pro ARP pakety, za n�m vynech�na mezera 7B pro status vektor 
#define TX1ST	5026	///<za��tek bufferu pro UDP paket (max. velikost 1514B)
#define TX1ND	6540	///<konec bufferu pro UDP paket, za n�m vynech�na mezera 7B pro status vektor 
#define TX3ST	6548	///<za��tek bufferu pro TCP paket (max. velikost 1514B)
#define TX3ND	8062	///<konec bufferu pro TCP paket, za n�m vynech�na mezera 7B pro status vektor
#define TX2ST	8070	///<za��tek bufferu pro ICMP a TCP automatick� odpov�di (ACK apod.) (max. velikost 114B)
#define TX2ND	8184	///<konec pomocn�ho bufferu, za n�m vynech�na mezera 7B pro status vektor


#define TIMEOUT 1000	///<doba �ek�n� na doodes�l�n� p�edchoz�ho paketu

//konstanty pro identifikaci aktualniho bufferu
#define TX1	32	///<buffer pro UDP pakety
#define TX2	64	///<buffer pro ICMP
#define TX3	128	///<buffer pro TCP pakety
#define TX4	192	///<buffer pro ARP

/**
 \endcond
 **/

/**
 \brief Inicializace bufferu pro odes�l�n� paketu.
 \param transport_protocol - jedna z konstant \a TCP_PROTO, \a UDP_PROTO a \a ICMP_PROTO
 							\n podle protokolu, kter�m se data budou odes�lat
 \retval 0 - inicializace prob�hla �sp�n�
 \retval -1 - chyba p�i inicializaci - UDP a ICMP - reinicializace bufferu, TCP - nepotvrzen� posledn� odeslan�ho paketu
 			\n ve v�ech p��padech m��e nastat chyba p���inou nedoodes�l�n� d��ve odes�lan�ho paketu - buffer se do �pln�ho odesl�n� nesm� m�nit  
 **/
char tx_init(unsigned char transport_protocol);


/**
 \brief Funkce vr�t� ukazatel pro z�pis v odes�lac�m bufferu na za��tek (jako po inicializaci).
 		Data se zachovaj�. Je tak mo�n� prov�st dodate�n� zm�ny v paketu p�ed odesl�n�m.
 \see tx_skip()
 **/
void tx_rewind();

/**
 \cond
 **/
 
/**
 \brief Funkce slou�� pro posunut� v bufferu na pozici pro z�pis hlavi�ek protokol� - intern� funkce.
 \param header jedna z konstant \a TCP_HEADER, \a UDP_HEADER, \a IP_HEADER, \a ETH_HEADER podle hlavi�ky, kterou chceme zapisovat
 **/
void tx_seek(unsigned char header);

/**
 \endcond
 **/

/**
 \brief Ulo�� pozici konce paketu a t�m ho p�iprav� na odesl�n�. 
 		Vol�n� t�to funkce je d�le�it�, ozna�� toti� konec paketu.
		V�echna data a� po takto ozna�en� konec se ode�lou, data zapsan� do bufferu za n�j se ignoruj�.
 **/
inline void tx_close();

/**
 \cond
 **/

/**
 \brief Ulo�� aktu�ln� stav bufferu - intern� funkce.
 \param buffer - jedna z konstant TX1, TX2, TX3 podle bufferu, kter� chceme ulo�it
 \see tx_act()
 \see tx_load()
 **/
void tx_save(unsigned char buffer);

/**
 \brief Na�te nastaven� a p�epne z�pis na buffer z parametru - intern� funkce.
 \param buffer - jedna z konstant \a TX1, \a TX2, \a TX3 podle bufferu, kter� chceme obnovit
 \see tx_act()
 \see tx_save()
 **/
void tx_load(unsigned char buffer);

/**
 \brief Vr�t� konstantu reprezentuj�c� pr�v� pou��van� buffer - intern� funkce.
 \returns jedna z konstant \a TX1, \a TX2 a \a TX3
 **/
unsigned char tx_act();

/**
 \endcond
 **/
 
/**
 \brief Zap�e znak do bufferu.
 \param c znak, kter� chceme zapsat
 \retval 0 - znak nezaps�n - buffer pln�
 \retval 1 - znak �sp�n� zaps�n
 \see tx_write()
 \see tx_write_str()
 **/
unsigned char tx_putc(char c);

/**
 \brief Zap�e blok dat velikosti \a length z adresy odkazovan� ukazatelem \a ptr do bufferu pro odesl�n�.
 \param ptr - ukazatel na data
 \param length - d�lka dat v bajtech
 \returns po�et zapsan�ch bajt�
 \see tx_putc()
 \see tx_write_str()
 **/
unsigned int tx_write(void* ptr, unsigned int length);

/**
 \brief Zap�e �et�zec ukon�en� hodnotou 0 z adresy odkazovan� ukazatelem \a str do bufferu pro odesl�n�.
 \param str - ukazatel na �et�zec
 \returns po�et zapsan�ch bajt�
 \see tx_putc()
 \see tx_write() 
 **/
#define tx_write_str(str) tx_write(str,enc_strlen(str))

/**
 \brief P�esko�� m�sto v odes�lac�m bufferu. Vhodn� zejm�na v kombinaci s funkc� tx_rewind(),
 		kdy chceme dodate�n� p�epsat n�kter� ��sti datagramu.
 \param length - d�lka skoku v bajtech.
 \returns po�et p�esko�en�ch bajt�
 **/
unsigned int tx_skip(unsigned int length);

/**
 \brief Vypln� v odes�lac�m bufferu \a length bajt� hodnotou \a value.
 \param value - hodnota, kterou se bude vypl�ovat buffer
 \param length - d�lka vypln�n�
 \returns po�et zapsan�ch bajt�
 **/
unsigned int tx_fill(unsigned char value, unsigned int length);

/**
 \cond
 **/
 
/**
 \brief Zprost�edkuje odesl�n� dat z odes�lac�ho bufferu v ENC28J60 do s�t� - intern� funkce.
 \retval 0 - odesl�n� se zda�ilo
 \retval -1 - chyba p�i odes�l�n�
 **/
char tx_send();

/**
 \endcond
 **/

/**
 \brief Vr�t� �daj o voln�m m�stu v bufferu.
 \returns po�et voln�ch bajt�
 **/
inline unsigned int tx_left();

/**
 \cond
 **/
 
/**
 \brief Vr�t� �daj o velikosti zapsan�ch dat - intern� funkce, �daj je platn� a� po ozna�en� konce dat funkc� tx_close().
 \returns po�et voln�ch bajt�
 **/
inline unsigned int tx_data_len();

/**
 \brief Vypo��t� checksum od aktu�ln� pozice a� do konce dat v odes�lac�m bufferu - intern� funkce.
 \returns checksum
 **/
unsigned int tx_checksum();

/**
 \brief Odemkne odes�lac� buffer, ten se pak m��e op�tovn� pou��t pro odesl�n� dat - intern� funkce. 
 \param buffer - jedna z konstant \a TX1, \a TX2, \a TX3 reprezentuj�c�ch p��slu�n� buffer
 **/
inline void tx_unlock(unsigned char buffer);

/**
 \endcond
 **/

#endif /*_ENC28J60_SPI_TX_*/
