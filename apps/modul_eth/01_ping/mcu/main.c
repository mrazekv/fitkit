/*******************************************************************************
   main.c: PING
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
  term_send_str_crlf("Ping");
  term_send_crlf();
  term_send_str_crlf("Program demonstrujici pouziti protokolu ICMP");
  term_send_str_crlf("Ping zarizeni provedete zadanim IP adresy do prikazove radky");
  term_send_str_crlf("napr. 147.229.176.19");
}



ICMP_APP_HANDLER(icmp_handler){
   if (icmp_header->type == ICMP_ECHO_REPLY) {  //typ ICMP zpravy - reply
      char dest_ip[20];
      ip_to_str(&dest_ip,20,NTOHL(ip_header->src_ip));
      
      term_send_str("odpoved na ping od: ");
      term_send_str_crlf(dest_ip);
   } else if (icmp_header->type == ICMP_ECHO) { 
      //chybi obsluha ECHO_REQUEST, fitkit nejde "pingnout"
      char src_ip[20];
      ip_to_str(&src_ip,20,NTOHL(ip_header->src_ip));

      term_send_str("pozadavek na ping od: ");
      term_send_str_crlf(src_ip);
   }
   return;  
}


//*************************************
// Dekodovani prikazu
//*************************************
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){
   unsigned long dest_ip;
   unsigned int dest_port;
   char dest_ip_[20];

   char cnt = 0, i = 0;
   for (i=0; cmd[i] != 0; i++) 
       if (cmd[i] == '.') cnt++;

   if ((cnt == 3) && (str_to_ip_port(cmd, enc_strlen(cmd), &dest_ip, &dest_port) > 0)) { //prikaz obsahuje 4 tecky, IP adresa
      ip_to_str(&dest_ip_, 20, dest_ip);
      term_send_str("odesilam ping na: ");
      term_send_str_crlf(dest_ip_);

      tx_init(ICMP_PROTO);
      tx_write_str("abcdefghijklmnopqrstuvw");
      tx_close();
      icmp_send(dest_ip,ICMP_ECHO,ICMP_ECHO_CODE,1,1);
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

   initialize_hardware();
   WDTCTL = WDTPW + WDTHOLD;                 // zastaveni watchdogu

   P1DIR |= LED0;
   P1OUT ^= 0x01;

   TIMERA_ISR_INIT()
   
   dhcp_init();                     //inicializace DHCP
   icmp_bind(icmp_handler);            //manualni kontrola nad ICMP
   
   term_send_str_crlf("Cekani na odpoved od DHCP");
   while (1){

      terminal_idle();              //obsluha terminalu
      ENC28J60_idle();              //obsluha modulu

      if(dhcp_ready()){
         if(dhcp_flag==0){
            dhcp_flag = 1;
                        
            char ip_str[16];
            term_send_str("DHCP ready, IP:");
            ip_to_str(&ip_str,16,get_ip());     //prevod IP na retezec
            term_send_str_crlf(ip_str);         //vypis prirazene IP
         }

      }
      else{
         dhcp_flag = 0;
      }
   }

   return 0;   
}


TIMERA_ISR_DHCP()
