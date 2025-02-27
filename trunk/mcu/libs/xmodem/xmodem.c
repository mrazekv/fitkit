/*******************************************************************************
   xmodem.c:
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

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


   Popis:
   Implementace XModem protokolu.
   Odchylka od standardu:
   Pokud se v datech prvniho paketu na adrese 0, nachazi klicove slovo
   'HEAD' (4Byty), tak zbyvajicich 12Bytu ma nasledujici vyznam

   Data prvniho paketu:

     0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15    c.Bytu
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   |'S' 'I' 'Z' 'E'|   FILE_SIZE   |FF |FF |FF |FF |FF |FF |FF |FF |
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

   kde,
 - FILE_SIZE: celkova velikost dat prenasenych XModemem
*******************************************************************************/

/* Provedena revize */

#include <string.h>
#include "arch_specific.h"
#include "uart.h"
#include "define.h"
#include "crc16.h"
#include "timerB.h"
#include "xmodem.h"

/**
 \brief Callback fce pro pøíjem dat pomocí XModemu a zápis pøímo do FPGA SpartanIII.
        - fce je zavolána vždy pøi úspìšném obdržení paketu
        - data paketu velikosti bufsz jsou v buf
 \param bufsz - velikost dat v paketu
 \param buf - paket
 \return 0 - pokud vse OK
      \n -1 - pokud nastala chyba
 **/
int XM_ProgSpartan(unsigned char *buf, long int bufsz) {
  long ii;

  // zaslani dat do Spartanu
  for (ii = 0; ii < bufsz; ii++) {
    /*
    SW_WDG;
    */
    SPI_ReadWrite(buf[ii]);
  }
  return 0;
}

long XModemProgFPGA() {
  // programovani FPGA SpartanaIII
  return XModemRxBufCB(XM_ProgSpartan);
}

/**
 \brief Callback fce pro pøíjem dat pomocí XModemu a zápis dat do flash.
       - fce je zavolána vždy pøi úspìšném obdržení paketu
 \param buf - ukazatel na data
 \param bufsz - velikost dat - musí být menši nebo rovna 1040 Bytu
 \return 0 - v poøádku
      \n -1 - nastala chyba
 **/
int WriteToFlash(unsigned char *buf, long int bufsz) {
  // zapis dat do fpga
  FLASH_WriteToBuffer((unsigned char *)buf, bufsz);
  return 0;
}

/**
 \brief  Fce zajistí nahrání dat z PC do flash.
 \param reverse - pøíznak obracení poøadí bitù (REVERSE_BITS, NO_REVERSE)
 \param AdrFlash - adresa ve FLASH pamìti
 \return  0 - chyba
         \n 1 - v poøádku
 **/
unsigned char FLASH_WriteFile(unsigned short startPage) {
  long ret  = 0;

  RS232_SendString("Zapis dat do flash. ");

  // adresy
  //CountPage = AdrFlash / PAGE_SIZE;
  //AdrBufferFlash = AdrFlash % PAGE_SIZE;
  CountPage = startPage;
  AdrBufferFlash = 0;

  // prijem dat z PC, ktera jsou zpracovana fci WorkData
  ret = XModemRxBufCB(WriteToFlash);

  if (ret < 0) {
    RS232_SendString("Chyba ");
    RS232_SendInt(ret);
    RS232_SendStringCRLF(" pri zapisu dat do flash");
    return 0;
  }
  else {
    RS232_SendStringCRLF("Bezchybny zapis dat do flash");
    return 0;
  }
}


// Definice konstant
#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define ETB  0x17
#define CAN  0x18
#define C_CHAR 0x43

#define FILL_CHAR  0x00

// Mod prenosu
#define CRC_MODE  0                     ///< CRC mód
#define CKSM_MODE 1                     ///< checksum mód
#define NONE_MODE 2                     ///< nezvolený mód

#define DLY_1S      1000                ///< definice 1 sec
#define MAXRETRANS  5                   ///< maximalní poèet znovuodeslání (pro všechny pakety)

#define BUFSZ_128   134                 ///< 3B hlavièka + 128B data + 2B crc + 1B nul
#define BUFSZ_1k    1030                ///< 3B hlavièka + 1024B data + 2B crc + 1B nul

#define XModem1kCRC                     ///< používat XMODEM v režimu 1k-CRC


/**
 \brief  Fce vyšle jeden znak na výstup (RS232)
 \param c - vysílaný znak
 **/
#define outbyte(c) RS232_SendChar((unsigned char)(c))


/**
 \brief Fce pøijme jeden byte ze vstupu (RS232), s timeoutem.
 \param timeout - maximální doba v ms
 \return -1 pøi chybì (RecvTimeout==TRUE)
      \n jinak naètený znak a RecvTimeout==FALSE
 **/
unsigned char inbyte(int timeout) {
  unsigned int Fl = RS232_ReceiveFlag(); // pocet bytu ve vyrovnacim bufferu RS232
  
  // dokud neprectu znak, nebo nevyprsi timeout mezi dvema znaky
  while (Fl == 0) {
    /*
    SW_WDG;
    */
    timeout -= 1;
    if (timeout<0)
      return -1;       // vyprsel timeout mezi dvema znaky.
    DelayMS(1);                     // cekam 1ms
    Fl = RS232_ReceiveFlag();       // pocet bytu ve vyrovnacim bufferu RS232
  }
  return RS232_ReceiveChar();        // zkusim precist znak
}

/**
 \brief Kontrola CRC souctu.
 \param crc - mod prenosu (crc: CRC_MODE, CKSM_MODE)
 \param buf - data
 \param sz  - velikost dat
 \return 1 - pokud OK
       \n 0 - pøi chybì
 **/
static int check_crc(int crc, unsigned char *buf, int sz) {
  if (crc == CRC_MODE) {
    unsigned short crc  = crc16_ccitt(buf, sz);
    unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
     
    if (crc == tcrc)
      return 1;
  }
  else {
    int i;
    unsigned char cks = 0;
    for (i = 0; i < sz; ++i) {
      /*
      SW_WDG;
      */
      cks += buf[i];
    }
    if (cks == buf[sz])
      return 1;
  }
  return 0;
}

/**
 \brief Fce èeká 100 ms pro vyprázdnìní bufferu.
 **/
#define flushinput RS232_FlushTxBuf()

/**
 \brief Fce pøijme celý soubor, pomocí protokolu XModem.
 \param func   - callback funkce, která se zavolá vždy po úspešném pøeètení paketu
               - musí vrátit 0 pøi úspìšném zpracování dat
 \param datasz - velikost cílových dat
 \param data   - pøijatá data
 \return  >=0 - bez chyby
      \n   <0 - chyba:
      \n     -1 - pøeruseni synchronizace vysílaèem.
      \n     -2 - chyba pøi synchronizaci s vysílaèem.
      \n     -3 - pøíliš mnohokrát opakováno pøeposlání paketu.
      \n     -4 - chyba v obslužné callback fci.
      \n     -6 - zrušení spojeni, pøišel BREAK.
      \n     -7 - velikost paketu 1024B není podporována.
 **/
long XModemRxBufCB(int (*func)(unsigned char* data, long datasz)) {
  long ErrNo    = -50;   // cislo chyby
  // alokace pameti na co nejkratsi dobu, jinak preteceni RAM pri vypisu logu
  // maximalni velikost souboru
  // pokud je v prvnim paketu na prvnim miste slovo 'HEAD', tak se hodnota destsz zmeni
  // na hodnotu 4Bytu za timto slovem
  long destsz    = 50000000;       // velikost prijatych dat

  #ifdef  XModem1kCRC
  unsigned char  xbuff[BUFSZ_1k];  // velikost prijimaciho bufferu
  #else
  unsigned char  xbuff[BUFSZ_128];  // velikost prijimaciho bufferu
  #endif

  unsigned char *p;                // ukazatel na prijimaci buffer
  unsigned char *tmpptr;           // pom. ukazatel
  unsigned char *startbuf;         // zacatek dat v paketu
  unsigned char  trychar  = 'C';   // znak k navazani spojeni
  unsigned char  packetno = 1;     // cislo paketu (od 1, az pretece rozsah 'char', tak od 0)
  long           len      = 0;     // velikost nactenych dat
  long           c;                // nacteny znak
  int            bufsz    = 0;     // velikost dat (128, 1024)
  int            crc      = 0;     // typ kontroly (CHKSM_MODE, CRC_MODE)
  int            retry,
                 retrans  = MAXRETRANS; // pocet opakovani chybnych paketu
  char           isFstPack= 1;     // 1..zpracovavam prvni paket


  for(;;) {
    /*
    SW_WDG;
    */

    // 15x se pokousim synchronizovat s vysilacem
    for (retry = 0; retry < 15; retry++) {
      /*
      SW_WDG;
      */

      if (trychar)
        outbyte(trychar); // vyslu synchronizacni znak C

      /*
      SW_WDG;
      */

      if ((c = inbyte((DLY_1S<<1))) >= 0) // cekam na odpoved
        switch (c) {
          case SOH:                  // budu prijimat pakety s velikosti dat 128B
            bufsz = 128;
            goto start_recv;
          case STX:                  // budu prijimat pakety s velikosti dat 1024B
            bufsz = 1024;
            #ifdef  XModem1kCRC
            goto start_recv;
            #else
            ErrNo = -7;        // velikost paketu neni podporovana
            goto stop_error;
            #endif
          case EOT:                  // vsechna data prijmuta
            flushinput;
            outbyte(ACK);         // potvrdim
            return (len);         // koncim bez chyby, vracim prijatych pocet bytu
          case CAN:                  // zadost o zruseni spojeni
            if ((c = inbyte(DLY_1S)) == CAN) {
              flushinput;
              outbyte(ACK);       // potvrzeni zadosti
              ErrNo = -1;
              goto stop_error;
            }
          break;
          default:
          break;
        }                          // switch
    }                              // for (retry = 0; retry < 16; retry++)

    // ustaveni spojeni pro CRC prenos se nezdarilo -> zkusim checksum
    if (trychar == 'C') {
      trychar = NAK;
      continue;
    }

    /*
    SW_WDG;
    */

    // nepodarilo se ustavit spojeni s vysilacem
    ErrNo = -2;
    goto stop_error;

    start_recv:

    /*
    SW_WDG;
    */

    if (trychar)
      crc = CRC_MODE;   // pokud se spojeni ustavilo znakem 'C',
                        // tak pouzivam CRC
    trychar = 0;
    p       = xbuff;
    *p++    = c;

    // prijmu paket
    if (RS232_ReceiveBlock(p, bufsz+ ((crc==CRC_MODE) ? 1 : 0) + 3) < 0) {
      // pozadam o preposlani paketu
      flushinput;
      outbyte(NAK);
    }
    else {
      // pokud jsou cisla paketu v poradku a
      // kontrolni soucet, nebo checksum je taky Ok
      if (xbuff[1] == (unsigned char)(~xbuff[2]) &&
         (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
         check_crc(crc, &xbuff[3], bufsz)) {
        /*
        SW_WDG;
        */
  
        if (xbuff[1] == packetno) {          // pokud prisel spravny paket
          register long int count = bufsz;   // delka noveho paketu
          if (destsz != 0) {                 // pokud mam co posilat
            startbuf = (unsigned char *)&xbuff[3];
  
            ////////////////////////////////////////////////////////////////////
            // NENI VE STANDARDU
            // - prectu prvnich 16B, pouze v prvnim paketu
            // - zkusim otestovat prvni 4B, a pokud se najde klicove slovo 'SIZE',
            //   prectu dalsi 4B, coz je celkova velikost prenasenych dat
            if ((xbuff[1] == 1) && (isFstPack) &&
                (xbuff[3] == 'S') && (xbuff[4] == 'I') &&
                (xbuff[5] == 'Z') && (xbuff[6] == 'E')) {
              isFstPack = 0;
              // kopie 4Bytu udavajici velikost souboru
              // - LITTLE ENDIAN->BIG ENDIAN
              tmpptr = (unsigned char *)&destsz;
              *tmpptr = xbuff[7]; tmpptr++;
              *tmpptr = xbuff[8]; tmpptr++;
              *tmpptr = xbuff[9]; tmpptr++;
              *tmpptr = xbuff[10];
              startbuf = (unsigned char *)&xbuff[19];
              count -= 16;
            }
            // KONEC ODCHYLKY OD STANDARDU
            ////////////////////////////////////////////////////////////////////
  
            if (destsz < bufsz)
              count = destsz;
            destsz -= count;
 
            /*
            SW_WDG;
            */
  
            // callback fce zpracujici spravny paket
            if (func(startbuf, count) < 0) {
              goto stop_error; //return (-4);
            }
            len += count;     // zvys pocet prectenych dat
          }                   // if (destsz != 0)
          packetno++;
          retrans = MAXRETRANS+1;
        }                     // if (xbuff[1] == packetno)
        if (--retrans <= 0) { // prekrocen limit pro preposilani
          ErrNo = -3;
          goto stop_error;
        }
        outbyte(ACK);         // potvrzeni spravneho paketu
        continue;             // pokracuji v prijmu
      }
    }    
  }                           // for (;;)
  stop_error:                 // zrusim spojeni

  /*
  SW_WDG;
  */
  flushinput;
  outbyte(CAN);
  outbyte(CAN);
  /*
  SW_WDG;
  */
  outbyte(CAN);
  flushinput;
  DelayMS(300);

/*
  {
//  #ifndef __WIN32__
  switch (ErrNo) {            // vypisi chybove hlaseni
  case -1: WriteLogError("Preruseni synchronizace vysilacem. (xmodem-Rx)"); break;
  case -2: WriteLogError("Chyba pri synchronizaci s vysilacem. (xmodem-Rx)"); break;
  case -3: WriteLogError("Prilis mnohokrat opakovani paketu. (xmodem-Rx)"); break;
  case -4: WriteLogError("Chyba v obsluzne callback fci. (xmodem-Rx)"); break;
  case -6: WriteLogError("Zruseni spojeni, prisel BREAK. (xmodem-Rx)"); break;
  case -7: WriteLogError("Velikost paketu 1024B neni podporovana. (xmodem-Rx)"); break;
  default: WriteLogError("Chyba pri prijmu dat. (xmodem-Rx)"); break;
  }
//  #endif
  }
*/
  return (ErrNo);               // vratim cislo chyby
}

/**
 \brief Fce odešle soubor, pomocí protokolu XModem.
 \param srcsz - velikost cílových dat
 \param PacketLen128 - !=0 velikost paketu 128B, jinak 1024B
 \param func - callback pro získání dat k odeslání
 \param data - odesílaná data
 \param datasz - velikost odesílaných dat
 \return  =0 - ok
      \n !=0 - chyba:
      \n    1 - chyba synchronizace vysílaèe s pøijímaèem.
      \n    2 - pøerušení pøenosu pøijímaèem.
      \n    3 - nastala chyba pøi pøenosu.
      \n    4 - nepøišel poslední potvrzovací znak.
      \n    5 - velikost paketu 1024B není podporována.
 **/
long XModemTxBufCB(long srcsz, char PacketLen128,
                   int (*func)(unsigned char* data, long datasz)) {
  #ifdef  XModem1kCRC
  unsigned char xbuff[BUFSZ_1k];     // odesilaci buffer
  #endif
  #ifndef XModem1kCRC
  unsigned char xbuff[BUFSZ_128];     // odesilaci buffer
  #endif
  int           bufsz;
  int           crc = NONE_MODE;
  unsigned char packetno = 1;
  long int      i, c, len = 0;
  int           retry;
  char          ErrNo = 0;            // zatim bez chyby

  for(;;) {
    /*
    SW_WDG;
    */
    // NAVAZANI SPOJENI
    for (retry=0; retry<16; retry++) {
      /*
      SW_WDG;
      */
      if ((c = inbyte(DLY_1S<<1)) >= 0) { // prectu znak s timeoutem 2sec
        switch (c) {
          case 'C':              // zadost o prenos v CRC modu
            crc = CRC_MODE;
            goto start_trans;
          case NAK:              // zadost o prenos s kontrolnim souctem
            crc = CKSM_MODE;
            goto start_trans;
          case CAN:              // zruseni prenosu prijimacem
            if ((c = inbyte(DLY_1S)) == CAN) {
              ErrNo = 2;
              goto stop_error;
            }
          break;
          default:
          break;
        }
      }                        // if ((c = inbyte((DLY_1S*2)<<1)) >= 0)
    }                          // for( retry = 0; retry < 16; ++retry)

    // chyba pri synchronizaci vysilace a prijimace
    ErrNo = 1;
    goto stop_error;

    // PRENOS SOUBORU
    for(;;) {
      /*
      SW_WDG;
      */

      start_trans:
      // ZACATEK PRENOSU
      if (PacketLen128 || (crc == CKSM_MODE)) {
        xbuff[0] = SOH;             // prenasi se pakety s velikosti dat 128B
        bufsz    = 128;             // velikost dat v paketu
      }
      else {
        xbuff[0] = STX;             // prenasi se pakety s velikosti dat 1024B
        bufsz    = 1024;
        #ifndef  XModem1kCRC
        ErrNo    = 5;
        goto stop_error;
        #endif
      }
      xbuff[1] =  packetno;         // cislo paketu
      xbuff[2] = ~packetno;         // a jeho negace
      c = srcsz - len;              // spocitam kolik zbyva vyslat dat
      if (c > bufsz)
        c = bufsz;                  // zarovnam pocet velikost bufferu
      if (c >= 0) {                 // pokud mam co posilat, tak
        memset (&xbuff[3], FILL_CHAR, bufsz); // vymazu buffer znakem

        if (c != 0)
          func(&xbuff[3], c); // nacteni dat

        // vlozim crc, nebo checksum
        if (crc == CRC_MODE) {      // spocitam CRC a ulozim do bufferu
          unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
          xbuff[bufsz+3] = (ccrc>>8) & 0xFF; // Hi CRC byte
          xbuff[bufsz+4] = ccrc & 0xFF;      // Lo CRC byte
        }
        else {                    // spocitam Checksum a ulozim do bufferu
          unsigned char ccks = 0;
          for (i = 3; i < bufsz+3; ++i)
            ccks += xbuff[i];
          xbuff[bufsz+3] = ccks;
        }

        // ODESILANI PAKETU
        for (retry=0; retry<MAXRETRANS; retry++) {
          /*
          SW_WDG;
          */
          for (i=0; i<bufsz+4+((crc==CRC_MODE)?1:0); ++i) // odesilam buffer po znaku
            outbyte(xbuff[i]);

          if ((c = inbyte(DLY_1S)) >= 0 ) { // cekam na odezvu prijimace
            switch (c) {
              case ACK:               // uspesne potvrzeni paketu prijimacem
                packetno++;
                len += bufsz;
                goto start_trans;
              case CAN:               // zadost prijimace o preruseni prenosu
                if ((c = inbyte(DLY_1S)) == CAN) {
                  ErrNo = 2;
                  goto stop_error;
                }
              break;
              case NAK:               // prijimac dostal paket poskozen,
              default:
              break;        // posilam znovu stejny paket
            }                       // switch (c)
          }                         // if ((c = inbyte(DLY_1S)) >= 0 )
        }                           // for (retry=0; retry<MAXRETRANS; retry++)
        ErrNo = 3;                  // NASTALA CHYBA PRI PRENOSU
        goto stop_error;

      }                             // if (c >= 0)
      else {                        // if (c >= 0) else
        // 10x zkusim odeslat znak ukoncujici spojeni
        for (retry = 0; retry < 5; ++retry) {
          /*
          SW_WDG;
          */
          outbyte(EOT);
          if ((c = inbyte((DLY_1S)<<1)) == ACK) // pokud prijde uspesne potvrzeni,
            break;                  // ukoncim odesilani ukoncovaciho znaku
        }
        if (c == ACK)
          return (0);   // prisel ACK -> prenos bez chyby
        ErrNo = 4;
        goto stop_error;            // neprisel posledni potvrzovaci znak
      }                             // if (c >= 0) else
    }                               // for(;;)
    /*
    SW_WDG;
    */
  }                               // for(;;)

  stop_error:                     // CHYBA - zrusim spojeni
  flushinput;
  outbyte(CAN);
  outbyte(CAN);
  outbyte(CAN);
  flushinput;

/*
#ifndef __WIN32__
  switch (ErrNo) {                // vypisi chybove hlaseni
  case 1: WriteLogError("Chyba synchronizace vysilace s prijimacem. (xmodem-Tx)"); break;
  case 2: WriteLogError("Preruseni prenosu prijimacem. (xmodem-Tx)"); break;
  case 3: WriteLogError("Nastala chyba pri prenosu. (xmodem-Tx)"); break;
  case 4: WriteLogError("Neprisel posledni potvrzovaci znak. (xmodem-Tx)"); break;
  case 5: WriteLogError("Velikost paketu neni podporovana. (xmodem-Tx)"); break;
  default: WriteLogError("Chyba pri odesilani dat. (xmodem-Tx)"); break;
  }
#endif
*/
  return ErrNo;                   // vratim cislo chyby
}
