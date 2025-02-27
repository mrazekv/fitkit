/*******************************************************************************
   main.c: 
   Copyright (C) 2009 Brno University of Technology,
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
 
   This software is provided ``as is'', and any express or implied
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

#include <fitkitlib.h>
#include <lcd/display.h>
#include <string.h>

unsigned short mem_addr;

unsigned char udp_data[72-4] = { /*Ethernet preamble/SFD (synchronizer):*/ 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xD5, 
  /* Ethernet destination address*/ 0x00, 0x10, 0xA4, 0x7B, 0xEA, 0x80,
  /* Ethernet source address*/ 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 
  /* Ethernet type (IP)*/ 0x08, 0x00, 
  /* IP header*/ 0x45, 0x00, 0x00, 0x2E, 0xB3, 0xFE, 0x00, 0x00, 0x80, 
  /* IP protocol UDP*/ 0x11,
  /* IP checksum*/ 0x05,  0x40, 
  /* IP source (192.168.0.44)*/ 0xC0,  0xA8,  0x00,  0x2C, 
  /* IP destination (192.168.0.4)*/ 0xC0,  0xA8,  0x00,  0x04, 
  /* UDP source port (1024)*/ 0x04,  0x00, 
  /* UDP destination port (1024)*/ 0x04,  0x00, 
  /* UDP payload length (18)*/ 0x00,  0x1A,
  /* UDP checksum*/ 0x2D,  0xE8, 
  /* UDP payload (18 bytes)*/ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11//, 
//  /* Ethernet checksum*/ 0xB3, 0x31, 0x88, 0x1B
};

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
*******************************************************************************/
void print_user_help(void) 
{
   term_send_str_crlf(" ORG YYYY ... poc. adresa v pameti, addr=YYYY");
   term_send_str_crlf(" WR YY    ... zapis bytu do pameti (hexa), mem[addr++] = 0xYY");
   term_send_str_crlf(" TX       ... odeslani paketu");
}

/*******************************************************************************
 * Dekodovani uzivatelskych prikazu a jejich vykonavani
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
   unsigned char val[2];

   if ((strcmp3(cmd_ucase, "ORG")) && (strlen(cmd_ucase) == 8)) 
   {

      val[0] = hex2chr(cmd_ucase + 4);
      val[1] = hex2chr(cmd_ucase + 6);
		mem_addr = val[0]*256 + val[1];

		term_send_str(" ORG: 0x");
      term_send_hex(mem_addr);
      term_send_crlf();

   } 
   else if ((strcmp2(cmd_ucase, "WR")) && (strlen(cmd_ucase) == 5)) 
   {
      val[0] = hex2chr(cmd_ucase + 3);

		FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0x8000 + (mem_addr & 0x7FF), (unsigned char *)&val, 2, 1);
      //cteni zpet (verifikace)
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0x8000 + (mem_addr & 0x7FF), (unsigned char *)&val, 2, 1);

      term_send_str(" M[0x");
      term_send_hex(mem_addr);
      term_send_str("] = 0x");
      term_send_hex(val[0]);
      term_send_crlf();

      mem_addr++;
   } 
   else if (strcmp2(cmd_ucase, "RD")) 
   {
      //cteni zpet (verifikace)
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0x8000 + ((mem_addr-1) & 0x7FF), (unsigned char *)&val, 2, 1);

      term_send_str(" M[0x");
      term_send_hex(mem_addr-1);
      term_send_str("] = 0x");
      term_send_hex(val[0]);
      term_send_crlf();


   }
   else if (strcmp2(cmd_ucase, "TX")) 
   {
		term_send_str(" Tx: ");
      term_send_num(mem_addr);
		term_send_str_crlf(" bytes");


      val[0] = mem_addr >> 8;
      val[1] = mem_addr & 0xFF;
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0xC0, (unsigned char *)&val, 1, 2);

   } 
   else 
   {
      return (CMD_UNKNOWN);
	}
   return USER_COMMAND;     
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized() 
{
   unsigned char val[2];

   mem_addr = 0;
   while (mem_addr < 72-4) {
      val[0] = udp_data[mem_addr];

		FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_WRITE, 0x8000 + (mem_addr & 0x7FF), (unsigned char *)&val, 2, 1);
      //cteni zpet (verifikace)
      FPGA_SPI_RW_AN_DN(SPI_FPGA_ENABLE_READ, 0x8000 + (mem_addr & 0x7FF), (unsigned char *)&val, 2, 1);

      term_send_str(" M[0x");
      term_send_hex(mem_addr);
      term_send_str("] = 0x");
      term_send_hex(val[0]);
      term_send_crlf();

      mem_addr++;
  }

}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
   initialize_hardware();

   set_led_d5(1);  //rozsvitit LED D5 

   while (1) 
   {
      terminal_idle();  // obsluha terminalu
   }

}
