/*******************************************************************************
   ticktacktoe.h: Tick tack toe header file
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Ladislav Capka <xcapka01 AT stud.fit.vutbr.cz>
              Zdenek Vasicek <vasicek AT fit.vutbr.cz>

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

#ifndef _TTT_H_
#define _TTT_H_

//#define TexOffset             0x2000
//#define MaskOffset            (TexOffset + 0x2000)

//Konstanty tykajici se VGA
#define MAXWIDTH        32 // max. pocet dlazdic v X-ove souradnici
#define TEXTURE_WIDTH   64
#define TEXTURE_PWIDTH  (TEXTURE_WIDTH / 2) 
#define TEXTURE_QWIDTH  (TEXTURE_WIDTH / 4) 
#define BOARD_XOFFSET   3
#define BOARD_YOFFSET   1
#define BOARD_WIDTH     14
#define BOARD_HEIGHT    13
#define SYMBOLS_TO_WIN  5 //pocet symbolu potrebnych pro vitezstvi

//Bazove adresy
#define BASE_ADDR_TEXTURE 0x1000 //bazova adresa pameti textur
#define BASE_ADDR_IDXMAP  0x2000 //bazova adresa mapy indexu
#define BASE_ADDR_KEYBOARD_ 0x0040 //bazova adresa klavesnice

//Makra pro prenos dat pres SPI
#define indexmap_write(x,y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_IDXMAP | (((y) * MAXWIDTH) + (x)), data, 2, 1)

#define indexmap_read(x,y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BASE_ADDR_IDXMAP | (((y) * MAXWIDTH) + (x)), data, 2, 1)

#define texture_write(x,y, data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_TEXTURE | (((y) * TEXTURE_WIDTH) + (x)), data, 2, 2)

#define cursor_position_write(data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, BASE_ADDR_KEYBOARD_, data, 1, 2);

#define keyboard_read(data) \
        FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, BASE_ADDR_KEYBOARD_, data, 1, 2);

//
#define CMD_NONE  0
#define CMD_UP    1
#define CMD_DOWN  2
#define CMD_LEFT  3 
#define CMD_RIGHT 4
#define CMD_PLACE 5
#define CMD_RESTART 9

//umisti krizek nebo kolecko do mrizky
void ticktacktoe_place(unsigned char x, unsigned char y, unsigned char id);

//zjisti, zda je na dane pozici krizek nebo kolecko
char ticktacktoe_check(unsigned char x, unsigned char y);

//inicializace textur
void ticktacktoe_init_textures(char gray);

//inicializace hraciho pole
void ticktacktoe_init_board();

//zmena souradnic kurzoru
void ticktacktoe_cursor_move(unsigned char x, unsigned char y);

//zjisteni stavu klavesnice
unsigned short ticktacktoe_kbstate(void);

//zjisti, zda-li nektery z hracu vyhral
int ticktacktoe_check_winner();

#endif  // _TTT_H_
