/*******************************************************************************
   filetransfer.c: data transfer routines
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
*******************************************************************************/

#include "arch_specific.h"
#include "fitkitlib.h"
#include "define.h"
#include "spi.h"
#include "uart.h"
   
char serial_rx_timeout(unsigned short timeout) {
  unsigned short tmout = timeout;

  while ((UART_RX_BUF_EMPTY) && (tmout != 0)) { 
    delay_ms(1);
    tmout--; 
  }

  if (tmout != 0)
    return UART_RX_BUF;

  return '\0';
}

#define flash_status_wait_cond(cond) \
 {SPI_set_cs_FLASH(1); SPI_write(OPCODE_STATUS); while (1) { ch=SPI_read_write(0); if (cond) break; }; SPI_set_cs_FLASH(0); }

#define flash_status(ch) \
 {SPI_set_cs_FLASH(1); SPI_write(OPCODE_STATUS); ch=SPI_read_write(0); SPI_set_cs_FLASH(0);}

void serial_tx_buf(char * buf) {
  char *ptr = buf;

  while (*ptr != '\0') {
    serial_tx(*ptr);
    ptr++;
  }
}

#define STX  0x01
#define ETX  0x03
#define SOT  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15

void send_header(char *title, char *mask, unsigned short maxPages) {
  char buf[10];
  short i;
  unsigned long maxSize = maxPages;
  maxSize = maxSize * (unsigned long)(PAGE_SIZE);

  serial_tx(STX);
  serial_tx_buf("UPLOADFILE;");
  if (title != 0) {
    serial_tx_buf("title=");
    serial_tx_buf(title);
    serial_tx_buf(";");
  }
  if (mask != 0) {
    serial_tx_buf("mask=");
    serial_tx_buf(mask);
    serial_tx_buf(";");
  }
  if (maxSize != 0) {
    serial_tx_buf("maxsize=");
    i = 0;
    while ((i<10) && (maxSize != 0)) {
      buf[i] = digit2chr(maxSize % 10);
      i++;
      maxSize = maxSize / 10;
    }
    do {
      i--;
      serial_tx(buf[i]);
    } while (i != 0);
  }
  serial_tx(ETX);
}

char writePageToFlash(unsigned short page, unsigned int* param) {
  unsigned char ch, bsum;
  unsigned char checksum;
  short i;

  flash_status_wait_cond((ch & 0x80) != 0);
  
  //WRITE TO BUFFER
  SPI_set_cs_FLASH(1);
  SPI_write(OPCODE_WRITE_BUF1);
  SPI_write(0);
  SPI_write(0); //addr HI
  SPI_write(0); //addr LO

  serial_tx(ACK);
  serial_rx(checksum);

  bsum = 0;
  for (i=0; i < 264; i++) { //data WR
    serial_rx(ch);
    SPI_write(ch);
    bsum += ch;
  }
  while (SPI_BUSY); //cekat, az je odeslano
  SPI_set_cs_FLASH(0);
  
  if (bsum != checksum) {
    serial_tx(NAK);
    return 0; //error
  }
  serial_tx(ACK);

  //WRITE BUFFER TO MEMORY WITH ERASE
  /*
  while ((TXEPT & U1TCTL) == 0);
  */
  flash_status_wait_cond((ch & 0x80) != 0);

  SPI_set_cs_FLASH(1);
  SPI_write(OPCODE_BUF1_MEMORY_WERASE);
  page = page << 1;
  SPI_write(page >> 8);
  SPI_write(page & 0xFF);
  SPI_write(0);
  SPI_set_cs_FLASH(0);

  /*
  return 0; //ERR
  */

  return 1; //OK
}

void transmittFile(unsigned short startPage, char *title, char *mask, 
      unsigned short maxPages, 
      char (*ProcessPageCB)(unsigned short page, unsigned int* param), //callback
      void* CBparam) {

  unsigned short page;
  unsigned short pagecnt;
  char try;
  char ch;

  WDG_stop();
 
  send_header(title, mask, maxPages);

  ch = serial_rx_timeout(2000);
  if (ch != ACK) {
    serial_tx(NAK);
    serial_tx_buf("init timeout");
    return;
  }
  serial_tx(ACK);

  page = startPage;
  pagecnt = 0;
  try = 3;
  while (1) {
    serial_rx(ch);

    if (ch != SOT) {
      serial_tx((ch == EOT) ? ACK : NAK);
      break;
    }

    if (ch == SOT) {
      if ((maxPages!=0) && (pagecnt == maxPages)) {
        /*
        serial_tx_buf("ERROR: file too big");
        */
        serial_tx(0x04); //EOT
        return;
      }
      ch = ProcessPageCB(page, CBparam);
      if (ch == 1) {
        page++;
        pagecnt++;
        try = 3;
      }
      else {
        if (try == 0) {
          serial_tx(0x04); //EOT
          return;
        }
        try--;
      }
    }

  }
}


char sendPageSPI(unsigned short page, unsigned int* bytes) {
  unsigned char ch, bsum;
  unsigned char checksum;
  short i;

  serial_tx(ACK);
  serial_rx(checksum);

  bsum = 0;
  for (i=0; i < 264; i++) { //data WR
    serial_rx(ch);
    if (bytes > 0) {
      bytes--;
      SPI_read_write(ch);
    }
    bsum += ch;
  }
  while (SPI_BUSY); //cekat, az je odeslano

  if (bsum != checksum) {
    serial_tx(NAK);
    return 0; //error
  }
  serial_tx(ACK);

  return 1; //OK
}

void SPI_WriteFile(char *title, char *mask, unsigned long maxSize) {
  unsigned short oldIE;
  unsigned long len = maxSize;

  #ifdef MSP_16X
     oldIE = IE1;
     IE1   &= ~(URXIE0 | UTXIE0);          // zakaz preruseni pri prijmu a vysilani
     transmittFile(0, title, mask, (unsigned short)((maxSize + PAGE_SIZE-1) / PAGE_SIZE), &sendPageSPI, &len);
     IE1 = oldIE;
  #endif

  #ifdef MSP_261X
     oldIE = IE2;
     IE2 &= ~(UCA0RXIE | UCA0TXIE);         // zakaz preruseni pri prijmu/vysilani
     transmittFile(0, title, mask, (unsigned short)((maxSize + PAGE_SIZE-1) / PAGE_SIZE), &sendPageSPI, &len);
     IE2 = oldIE;
  #endif
}   

void FLASH_WriteFile(unsigned short startPage, char *title, char *mask,
                     unsigned short maxPages) {
  unsigned short oldIE;

  #ifdef MSP_16X
     oldIE = IE1;
     IE1   &= ~(URXIE0 | UTXIE0);          // zakaz preruseni pri prijmu a vysilani
     transmittFile(startPage, title, mask, maxPages, &writePageToFlash, 0);
     IE1 = oldIE;
  #endif

  #ifdef MSP_261X
     oldIE = IE2;
     IE2 &= ~(UCA0RXIE | UCA0TXIE);         // zakaz preruseni pri prijmu/vysilani
     transmittFile(startPage, title, mask, maxPages, &writePageToFlash, 0);
     IE2 = oldIE;
  #endif
}


char *cb_buf;
char (*cb_fun)(unsigned short page, unsigned char* buf, unsigned short len);

char cbPage(unsigned short page, unsigned int* bytes) {
  unsigned char ch, bsum;
  unsigned char checksum;
  short i;

  serial_tx(ACK);
  serial_rx(checksum);

  bsum = 0;
  for (i=0; i < 264; i++) { //data WR
    serial_rx(ch);
    if (bytes > 0) {
      bytes--;
      cb_buf[i] = ch;
    }
    bsum += ch;
  }
  while (SPI_BUSY); //cekat, az je odeslano
  
  if (bsum != checksum) {
    serial_tx(NAK);
    return 0; //error
  }
  serial_tx(ACK);

  cb_fun(page, cb_buf, i);

  return 1; //OK
}

void ReceiveFile(char *title, char *mask, unsigned long maxSize, char (*ProcessPageCB)(unsigned short page, unsigned char* buf, unsigned short len) ) {
  unsigned short oldIE;
  unsigned long len = maxSize;
  char sbuf[265];
  int i;
  for (i =0; i < 265; i++) sbuf[i] = 0xFF;
  cb_buf = sbuf;
  cb_fun = ProcessPageCB;

  #ifdef MSP_16X
     oldIE = IE1;
     IE1   &= ~(URXIE0 | UTXIE0);          // zakaz preruseni pri prijmu a vysilani
     transmittFile(0, title, mask, (unsigned short)((maxSize + PAGE_SIZE-1) / PAGE_SIZE), &cbPage, &len);
     IE1 = oldIE;
  #endif

  #ifdef MSP_261X
     oldIE = IE2;
     IE2 &= ~(UCA0RXIE | UCA0TXIE);         // zakaz preruseni pri prijmu/vysilani
     transmittFile(0, title, mask, (unsigned short)((maxSize + PAGE_SIZE-1) / PAGE_SIZE), &cbPage, &len);
     IE2 = oldIE;
  #endif
}   

