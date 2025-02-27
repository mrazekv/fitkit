/*******************************************************************************
   enc28j60_spi.h: Knihovna funkcí pro komunikaci s ENC28J60
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
	\file enc28j60_spi.h
	\brief Knihovna funkcí pro komunikaci s ENC28J60 prostøednictvím rozhraní SPI.
*/

#ifndef _ENC28J60_SPI_
#define _ENC28J60_SPI_

//tato knihovna zahrnuje dalsi 2 casti
#include "enc28j60_spi_rx.h"
#include "enc28j60_spi_tx.h"

/**
 \cond
 **/
#define NULL 0

#define BASE_ADDR_CTRLREG 0xF0
#define control_reg_write(data) FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_CTRLREG, data)
#define control_reg_read() (FPGA_SPI_RW_A8_D8(SPI_FPGA_ENABLE_READ, BASE_ADDR_CTRLREG, 0))


#define ETH_CRC_LEN	4	///<délka ethernetového CRC
#define PTR_LEN 2		///<délka ukazatele v pamìti ENC28J60

/**
 \endcond
 **/

/**
 \brief Inicializace a start ENC28J60
 \retval 0 - inicializace probìhla úspìšnì
 \retval -1 - chyba pøi inicializaci
 **/
char ENC28J60_init();

/**
 \cond
 **/

//******************************************
//fce cteni/zapis registru
//******************************************
#define NO_DUMMY 0	///< kratší cyklus ètení
#define DUMMY 0xAA	///< delší cyklus ètení

/**
 \brief Pøeète 8-bitový kontrolní registr
 \param addr - adresa registru
 \param read_dummy - konstanta NO_DUMMY pro kratší cyklus ètení (ETH registry), \a DUMMY pro delší cyklus ètení (MAC a MII registry)
 \returns hodnota registru
 **/
inline unsigned char read_reg8(unsigned char addr, unsigned char read_dummy);

/**
 \brief Pøeète 16-bitový kontrolní registr
 \param addr - adresa registru
 \param read_dummy - konstanta NO_DUMMY pro kratší cyklus ètení (ETH registry), \a DUMMY pro delší cyklus ètení (MAC a MII registry)
 \returns hodnota registru
 **/
inline unsigned int read_reg16(unsigned char addr, unsigned char read_dummy);

/**
 \brief Zapíše data do 8-bitového kontrolního registru
 \param addr - adresa registru
 \param data - data pro zápis
 **/
inline void write_reg8(unsigned char addr, unsigned char data);

/**
 \brief Zapíše data do 16-bitového kontrolního registru
 \param addr - adresa registru
 \param data - data pro zápis
 **/
inline void write_reg16(unsigned char addr, unsigned int data);

/**
 \brief Pøeète 16-bitový PHY registr
 \param addr - adresa registru
 \returns hodnota registru
 **/
inline int read_phy_reg16(unsigned char addr);

/**
 \brief Zapíše data do 16-bitového PHY registru
 \param addr - adresa registru
 \param data - data pro zápis
 **/
inline void write_phy_reg16(unsigned char addr, unsigned int data);

/**
 \brief BFS (Bit Field Set) - Nastaví bity v registru podle masky, operace OR
 \param addr - adresa registru
 \param mask - maska bitù
 **/
inline void bf_set(unsigned char addr, unsigned char mask);


/**
 \brief BFC (Bit Field Clear) - Vynuluje bity v registru podle masky, operace NOT AND
 \param addr - adresa registru
 \param mask - maska bitù
 **/
inline void bf_clear(unsigned char addr, unsigned char mask);


/**
 \brief Resetuje ENC28J60
 **/
inline void enc_reset();

/**
 \brief Pøepne banku registrù v ENC28J60
 \param bank - adresa registru
 \returns hodnota registru
 **/
inline void select_bank(unsigned char bank);



//******************************************
//fce cteni/zapis do bufferu
//******************************************

/**
 \brief Pøeète bajt z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 \returns ètený bajt
 **/
unsigned char spi_getc();

/**
 \brief Pøeète blok pamìti z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 \param ptr - ukazatel do bufferu, kam se budou data zapisovat
 \param length - délka bloku dat
 \returns poèet pøeètených bajtù
 **/
unsigned int spi_read(void* ptr, unsigned int length);

/**
 \brief Pøeète øádek textu ukonèený znakem LF z bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem ERDPT
 		\n vhodné pro textové protokoly jako je napø. telnet
 \param ptr - ukazatel do bufferu, kam se budou data zapisovat
 \param max_length - maximální délka øádku
 \returns poèet pøeètených znakù vèetnì CR a LF
 **/
unsigned int spi_getline(void* ptr, unsigned int max_length);

/**
 \brief Zapíše bajt do bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 \param c - bajt dat
 **/
void spi_putc(unsigned char c);

/**
 \brief Zapíše blok pamìti do bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 \param ptr - ukazatel do bufferu, odkud se budou data èíst
 \param length - délka dat
 **/
void spi_write(void* ptr, unsigned int length);

/**
 \brief Vyplní èást bufferu v ENC28J60 - poèáteèní adresa je dána kontrolním registrem EWRPT
 \param value - hodnota, kterou se buffer vyplní
 \param length - délka vyplnìní
 **/
void spi_fill(unsigned char value, unsigned int length);

/**
 \brief Zjistí zmìnu stavu linky (vypnuto->zapnuto a opaènì), èasovì nenároèná operace
 \retval 1 - stav linky se zmìnil
 \retval 0 - beze zmìny
 \see link_up()
 **/
inline unsigned char link_change();

/**
 \endcond
 **/

/**
 \brief Vrátí stav linky, èasovì hodnì nároèná operace(~1ms) - ètení PHY registrù z ENC28J60
 \retval 1 - linka je funkèní
 \retval 0 - odpojeno
 \see link_change()
 **/
inline unsigned char link_up();

/**
 \cond
 **/

//******************************************
//SPI PRIKAZY
//******************************************

//SPI prikazy
#define ENC_RCR 0x00
#define ENC_RBM 0x3A
#define ENC_WCR 0x40
#define ENC_WBM 0x7A
#define ENC_BFS 0x80
#define ENC_BFC 0xA0
#define ENC_RST 0xFF

//******************************************
//ADRESY REGISTRU
//******************************************

//banka 0
#define ERDPTL 		0x00
#define ERDPTH 		0x01
#define EWRPTL 		0x02
#define EWRPTH	 	0x03
#define ETXSTL		0x04
#define ETXSTH		0x05
#define ETXNDL		0x06
#define ETXNDH		0x07
#define ERXSTL		0x08
#define ERXSTH		0x09
#define ERXNDL		0x0A
#define ERXNDH		0x0B
#define ERXRDPTL	0x0C
#define ERXRDPTH	0x0D
#define ERXWRPTL	0x0E
#define ERXWRPTH	0x0F
#define EDMASTL		0x10
#define EDMASTH		0x11
#define EDMANDL		0x12
#define EDMANDH		0x13
#define EDMADSTL	0x14
#define EDMADSTH	0x15
#define EDMACSL		0x16
#define EDMACSH		0x17
//16-bit
#define ERDPT 		0x00
#define EWRPT 		0x02
#define ETXST		0x04
#define ETXND		0x06
#define ERXST		0x08
#define ERXND		0x0A
#define ERXRDPT		0x0C
#define ERXWRPT		0x0E
#define EDMAST		0x10
#define EDMAND		0x12
#define EDMADST		0x14
#define EDMACS		0x16



//banka 1
#define EHT0		0x00
#define EHT1		0x01
#define EHT2		0x02
#define EHT3		0x03
#define EHT4		0x04
#define EHT5		0x05
#define EHT6		0x06
#define EHT7		0x07
#define EPMM0		0x08
#define EPMM1		0x09
#define EPMM2		0x0A
#define EPMM3		0x0B
#define EPMM4		0x0C
#define EPMM5		0x0D
#define EPMM6		0x0E
#define EPMM7		0x0F
#define EPMCSL		0x10
#define EPMCSH		0x11
#define EPMOL		0x14
#define EPMOH		0x15
#define ERXFCON		0x18
#define EPKTCNT		0x19
//16-bit
#define EPMCS		0x10


//banka 2
#define MACON1 		0x00
#define MACON2 		0x01
#define MACON3 		0x02
#define MACON4 		0x03
#define MABBIPG 	0x04
#define MAIPGL		0x06
#define MAIPGH		0x07
#define MACLCON1	0x08
#define MACLCON2	0x09
#define MAMXFLL		0x0A
#define MAMXFLH		0x0B
#define MICMD		0x12
#define MIREGADR	0x14
#define MIWRL		0x16
#define MIWRH		0x17
#define MIRDL		0x18
#define MIRDH		0x19
//16-bit
#define MAIPG		0x06
#define MAMXFL		0x0A
#define MIWR		0x16
#define MIRD		0x18

//banka 3
#define MAADR5 		0x00
#define MAADR6 		0x01
#define MAADR3 		0x02
#define MAADR4	 	0x03
#define MAADR1		0x04
#define MAADR2		0x05
#define EBSTSD		0x06
#define EBSTCON		0x07
#define EBSTCSL		0x08
#define EBSTCSH		0x09
#define MISTAT		0xA0
#define EREVID		0x12
#define ECOCON		0x15
#define EFLOCON		0x17
#define EPAUSL		0x18
#define EPAUSH		0x19
//16-bit
#define EBSTCS		0x08


//spolecne registry
#define EIE			0x1B
#define EIR			0x1C
#define ESTAT		0x1D
#define ECON2		0x1E
#define ECON1		0x1F

// PHY registry
#define PHCON1      0x00
#define PHSTAT1     0x01
#define PHHID1      0x02
#define PHHID2      0x03
#define PHCON2      0x10
#define PHSTAT2     0x11
#define PHIE        0x12
#define PHIR        0x13
#define PHLCON      0x14

//***********************************
//BITY REGISTRU
//***********************************

// EIE
#define EIE_INTIE       0x80
#define EIE_PKTIE       0x40
#define EIE_DMAIE       0x20
#define EIE_LINKIE      0x10
#define EIE_TXIE        0x08
#define EIE_WOLIE       0x04
#define EIE_TXERIE      0x02
#define EIE_RXERIE      0x01

// EIR
#define EIR_PKTIF       0x40
#define EIR_DMAIF       0x20
#define EIR_LINKIF      0x10
#define EIR_TXIF        0x08
#define EIR_WOLIF       0x04
#define EIR_TXERIF      0x02
#define EIR_RXERIF      0x01

// ESTAT
#define ESTAT_INT       0x80
#define ESTAT_BUFER     0x40
#define ESTAT_LATECOL   0x10
#define ESTAT_RXBUSY    0x04
#define ESTAT_TXABRT    0x02
#define ESTAT_CLKRDY    0x01

// ECON2
#define ECON2_AUTOINC   0x80
#define ECON2_PKTDEC    0x40
#define ECON2_PWRSV     0x20
#define ECON2_VRPS      0x08

// ECON1
#define ECON1_TXRST     0x80
#define ECON1_RXRST     0x40
#define ECON1_DMAST     0x20
#define ECON1_CSUMEN    0x10
#define ECON1_TXRTS     0x08
#define ECON1_RXEN      0x04
#define ECON1_BSEL1     0x02
#define ECON1_BSEL0     0x01

// MACON1
#define MACON1_LOOPBK   0x10
#define MACON1_TXPAUS   0x08
#define MACON1_RXPAUS   0x04
#define MACON1_PASSALL  0x02
#define MACON1_MARXEN   0x01

// MACON2
#define MACON2_MARST    0x80
#define MACON2_RNDRST   0x40
#define MACON2_MARXRST  0x08
#define MACON2_RFUNRST  0x04
#define MACON2_MATXRST  0x02
#define MACON2_TFUNRST  0x01

// MACON3
#define MACON3_PADCFG2  0x80
#define MACON3_PADCFG1  0x40
#define MACON3_PADCFG0  0x20
#define MACON3_TXCRCEN  0x10
#define MACON3_PHDRLEN  0x08
#define MACON3_HFRMLEN  0x04
#define MACON3_FRMLNEN  0x02
#define MACON3_FULDPX   0x01

// MACON4
#define MACON4_DEFER	0x40

// MICMD
#define MICMD_MIISCAN   0x02
#define MICMD_MIIRD     0x01

// MISTAT
#define MISTAT_NVALID   0x04
#define MISTAT_SCAN     0x02
#define MISTAT_BUSY     0x01

//PHIR
#define PHIR_PLNKIF		0x0010

//PHIE
#define PHIE_PLNKIE		0x0010
#define PHIE_PGEIE		0x0002

// PHCON1
#define PHCON1_PRST     0x8000
#define PHCON1_PLOOPBK  0x4000
#define PHCON1_PPWRSV   0x0800
#define PHCON1_PDPXMD   0x0100

// PHCON2
#define PHCON2_FRCLINK  0x4000
#define PHCON2_TXDIS    0x2000
#define PHCON2_JABBER   0x0400
#define PHCON2_HDLDIS   0x0100

// PHSTAT1
#define PHSTAT1_PFDPX   0x1000
#define PHSTAT1_PHDPX   0x0800
#define PHSTAT1_LLSTAT  0x0004
#define PHSTAT1_JBSTAT  0x0002

// PHSTAT2
#define PHSTAT2_LSTAT	0x0400

// PKTCTRL
#define PKTCTRL_PHUGEEN     0x08
#define PKTCTRL_PPADEN      0x04
#define PKTCTRL_PCRCEN      0x02
#define PKTCTRL_POVERRIDE   0x01

/**
 \endcond
 **/

#endif /*_ENC28J60_SPI_*/
