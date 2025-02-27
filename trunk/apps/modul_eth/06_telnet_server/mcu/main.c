/*******************************************************************************
   main.c: TELNET server
   Copyright (C) 2010 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Martin Musil <xmusil34 AT fit.vutbr.cz>
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

#include <fitkitlib.h>
#include <enc28j60/tcpip_stack.h>
#include <stdio.h>
#include <thermometer/thermometer.h>

//*************************************
// Tisk napovedy
//*************************************

void print_user_help(void){
  term_send_str_crlf("Simple TELNET server");
  term_send_crlf();
  term_send_str_crlf("Po ziskani nastaveni z DHCP serveru je server pristupny pres TELNET klient.");
  term_send_str_crlf("Server bezi na portu 23");
}



TCP_APP_HANDLER(tcp_handler){
   
   char temp_str[20];   
   char buffer[50];

   rx_getline(&buffer,50);
   
   //zpracovani prichozich prikazu
      
   if( strcmp6(buffer, "LED ON") ){
      P1OUT &= 0xFE;
      
      if( tx_init(TCP_PROTO) == 0){       //inicializace odesilaciho bufferu
         tx_write_str("led is on\r\n");      //zapis dat
         tx_close();                   //uzavreni bufferu
         tcp_send();                   //odeslani datagramu
      }
   }
   else if( strcmp7(buffer, "LED OFF") ){
      P1OUT |= 0x01;
            
      if( tx_init(TCP_PROTO) == 0){
         tx_write_str("led is off\r\n");
         tx_close();
         tcp_send();
      }
      
   }
   else if(strcmp8(buffer, "GET TEMP")){
      int temperature = thermometer_gettemp();              // zjisteni aktualni teploty
      sprintf (temp_str, "%02d.%01d°C", temperature/10, temperature%10);   //teplota --> retezec
      
      if( tx_init(TCP_PROTO) == 0){
         tx_write_str("current temperature: ");
         tx_write_str(temp_str);
         tx_write_str("\r\n");
         tx_close();
         tcp_send();
      }
   }
   else if( strcmp4(buffer, "QUIT") ){
      
      if( tx_init(TCP_PROTO) == 0){
         tx_write_str("bye bye\r\n");
         tx_close();
         tcp_send();
      }
      
      tcp_disconnect();          //aktivni ukonceni TCP spojeni
      
   }
   else{
      if( tx_init(TCP_PROTO) == 0){
         tx_write_str("unknown command\r\n");
         tx_close();
         tcp_send();
      }
      
   }
   
   return;  
}


//*************************************
// Dekodovani prikazu
//*************************************
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){
   return CMD_UNKNOWN;
}


//*************************************
// Inicializace  periferii
//*************************************
void fpga_initialized(){
   ENC28J60_init();     //inicializace ENC28J60
}

//*************************************
// Hlavni smycka programu
//*************************************
int main(void){
   char dhcp_flag = 0;
   char tcp_flag = 0;

   initialize_hardware();
   WDTCTL = WDTPW + WDTHOLD;                 // zastaveni watchdogu

   P1DIR |= LED0;
   P1OUT ^= 0x01;

   TIMERA_ISR_INIT()                //inicializace casovace
   
   create_tcp_socket(23,tcp_handler);     //vytvoreni socketu a registrace handleru na port 23
   tcp_bind();                      //TCP server
   
   dhcp_init();   
   
   while (1){

      terminal_idle();           // obsluha terminalu
      ENC28J60_idle();           // obsluha modulu


      if(dhcp_ready()){
         if(dhcp_flag==0){
            dhcp_flag = 1;

            term_send_str("DHCP ready, IP address: ");
            char ip_str[16];
            ip_to_str(ip_str, 16, get_ip());    //ip --> retezec
            term_send_str_crlf(ip_str);            //vypis IP na terminal
         }
         
         if(tcp_connected()){
            if(tcp_flag == 0){
               term_send_str_crlf("TCP CONNECTION ESTABILISHED");
               tcp_flag = 1;
            }
         }
         else{
            if(tcp_flag == 1){
               term_send_str_crlf("TCP CONNECTION CLOSED");
               tcp_flag = 0;
            }
         }  
      }
      else{
         dhcp_flag = 0;
      }
   }

   return 0;   
}

TIMERA_ISR_DHCP_TCP()

