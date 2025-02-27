/*******************************************************************************
   main.c: TELNET klient
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

//*************************************
// Tisk napovedy
//*************************************

void print_user_help(void){
  term_send_str_crlf("Jednoduchy TELNET klient");
  term_send_crlf();
  term_send_str_crlf("ip_adresa:port  ... pripojeni ke vzdalenemu pocitaci");
  term_send_str_crlf("                    po uspesnem pripojeni je kazdy prikaz napsany");
  term_send_str_crlf("                    do terminalu odesilan vzdalenemu pocitaci");
  term_send_str_crlf("DISCONNECT      ... ukonceni spojeni");
  term_send_crlf();
  term_send_str_crlf("77.75.76.46:110 ... pripojeni na POP3.seznam.cz");
  
}

TCP_APP_HANDLER(app_handler){
   
   char buffer[50];

   while(rx_left() > 0){            //test prichoziho bufferu, zda obsahuje nejaka data
      rx_getline(&buffer,50);       //precteni dat
      term_send_str(buffer);        //vypis na terminal
   }

   return;  
}

//*************************************
// Dekodovani prikazu
//*************************************
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){
   char cnt = 0, i = 0;
   unsigned long dest_ip;
   unsigned int dest_port;

   if (dhcp_ready()) {                 //nastaveni z DHCP je pripraveno

      if (tcp_connected()) {     //odeslani prikazu na server

         if (strcmp10(cmd_ucase, "DISCONNECT")){
            tcp_disconnect();
            return USER_COMMAND;
         }
     
         term_send_crlf();
         
         tx_init(TCP_PROTO);        //inicializace odesilaciho bufferu
         tx_write_str(cmd);
         tx_write_str("\r\n");      //dopsani ENTERu
         tx_close();
         tcp_send();             //odeslani datagramu
         return USER_COMMAND;

      } else {

         for (i=0; cmd[i] != 0; i++) 
             if (cmd[i] == '.') cnt++;

         if ((cnt == 3) && (str_to_ip_port(cmd, enc_strlen(cmd), &dest_ip, &dest_port) > 0)) { //prikaz obsahuje 4 tecky, IP adresa
   
            if(tcp_is_idle() == 1){
               if( dest_port > 0) { 
                  create_tcp_socket(53875,app_handler);  //vytvoreni socketu a registrace handleru na port 53875
                  tcp_connect(dest_ip,dest_port);        //pripojeni ke vzdalenemu pocitaci
               }
               else
                  term_send_str_crlf("port missing");
            } 
            else
               term_send_str_crlf("TCP not in idle state");

            return USER_COMMAND;
         }
     }
  }      
  else  {
     term_send_str_crlf("ERROR: DHCP not ready"); 
     return USER_COMMAND;
  }
  
  return CMD_UNKNOWN;
}


//*************************************
// Inicializace  periferii
//*************************************
void fpga_initialized(){
    ENC28J60_init();  //inicializace ENC28J60
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

   TIMERA_ISR_INIT()
  
  
  
   dhcp_init();                     //inicializace DHCP
   
   while (1){

      terminal_idle();              // obsluha terminalu
      ENC28J60_idle();              // obsluha modulu


      if(dhcp_ready()){          //nastaveni z DHCP je pripraveno
         if(dhcp_flag==0){
            dhcp_flag = 1;

            char ip_str[16];
            ip_to_str(&ip_str,16,get_ip());           //prevod ip na retezec
            term_send_str("DHCP ready, IP address: ");
            term_send_str_crlf(ip_str);               //vypis prirazene IP na terminal
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

