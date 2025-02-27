/*******************************************************************************
   main.c: DNS - prevod domenovych jmen na IP
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

#define DNS_HEADER_LEN 12

//DNS hlavicka
struct dns_h{
   unsigned int id;
   unsigned char rd:1;
   unsigned char tc:1;
   unsigned char aa:1;
   unsigned char opcode:4;
   unsigned char qr:1;
   unsigned char rcode:4;
   unsigned char zero:3;
   unsigned char ra:1;
   unsigned int qd_count;
   unsigned int an_count;
   unsigned int ns_count;
   unsigned int ar_count;
};

unsigned int trasaction_id = 0;

//*************************************
// Tisk napovedy
//*************************************
void print_user_help(void){
  term_send_str_crlf("DNS request demo");
  term_send_crlf();
  term_send_str_crlf("QUERY www.seznam.cz ... prevede domenove jmeno na IP");
  term_send_str_crlf("    Odesle DNS dotaz na DNS server ziskany protokolem DHCP.");
  term_send_str_crlf("    Na terminal se zobrazi vsechny zaznamy typu A.");
}

//*************************************
// Dekodovani prikazu
//*************************************
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){

   if(dhcp_ready()){                            //ziskano DHCP nastaveni
      if (strcmp6(cmd_ucase, "QUERY ") ) {
         
         if (tx_init(UDP_PROTO) == 0){          //inicializace bufferu
                  
            cmd += 6;                           //posunuti ukazatele za prikaz
            
            struct dns_h dns_header;
            enc_memset(&dns_header,0,DNS_HEADER_LEN);
            trasaction_id++;
            dns_header.id = (HTONS(trasaction_id));      //cislo transakce
            dns_header.rd = 1;                     //rekurzivni dotaz
            dns_header.qd_count = HTONS(1);           //1 dotaz
            tx_write(&dns_header,DNS_HEADER_LEN);     //zapsani hlavicky
            
            char *ptr = cmd;
            int len = enc_strlen(ptr);
      
               //zapis domenoveho jmena v pozadovanem formatu pro DNS      
            while(len-- > 0){
               
               if(*ptr == '.'){
                  tx_putc((unsigned char)(ptr - cmd));
                  tx_write(cmd, ptr - cmd);
                  ptr++;                           //preskoceni '.'
                  cmd = ptr;
               }
               else if(*ptr == '/'){                  //adresa za '/' se nezpracovava
                  tx_putc((unsigned char)(ptr - cmd));
                  tx_write(cmd, ptr - cmd);
                  cmd = ptr;                       //vraceni na pozici pred '/'
                  break;
               }
               else{
                  ptr++;
               }
            }
            
            if((ptr != cmd) & (*ptr != '/')){
               tx_putc((unsigned char)(ptr - cmd));
               tx_write(cmd, ptr - cmd);
            }
            
            tx_putc(0);                   //konec domenoveho jmena
            
            tx_putc(0);                   //type - A
            tx_putc(1);
            tx_putc(0);                   //class - IN
            tx_putc(1);
            tx_close();
            udp_send(53, 53800, get_dns_server()); //odeslani datagramu na port 53
         }
         return USER_COMMAND; 
      }
  } else {
     term_send_str_crlf("DHCP neni pripraveno");
  }
   
  return CMD_UNKNOWN;
}

#define A      1
#define CNAME  5

//*************************************
// Obsluha prichoziho datagramu na portu 53800
//*************************************
UDP_APP_HANDLER(DNS){

   struct dns_h dns_header;
   rx_read(&dns_header,DNS_HEADER_LEN);

   if(dns_header.qr != 1)     //filtr - prijem pouze odpovedi
      return;

   if(dns_header.ra != 1)     //filtr - demo podporuje pouze rekurzivni DNS odpovedi
      return;

   if (NTOHS(dns_header.an_count) == 0 ){
      term_send_str_crlf("domenove jmeno neexistuje");
      return;
   }
      
   int length;
   
   while((length = rx_getc()) != 0){         //preskoceni dns dotazu
      rx_skip(length);
   }
   rx_skip(4);                         //preskoceni Type a Class
   
   
      //odpovedi
   int answers = NTOHS(dns_header.an_count);
   
   while(answers-- > 0){
   
      length = rx_getc();
      if(length >= 0xC0){                    //komprese domenoveho jmena
         rx_getc();
      }
      else{                            //domenove jmeno zapsano primo
         do{
            rx_skip(length);
         }while((length = rx_getc()) != 0);     //preskoceni jmena
      }
      
      int type;                        //typ zaznamu
      rx_read(&type,2);
      type = NTOHS(type);
      
      int dns_class;                   //trida zaznamu (IN)
      rx_read(&dns_class,2);
      dns_class = NTOHS(dns_class);
      
      rx_skip(4);                      //preskoceni TTL (doba platnosti)
      
      unsigned long ip = 0;
      
      switch(type){
         case A:                       //zaznam typu A
            rx_read(&length,2);
            rx_read(&ip,4);               //precteni IP adresy
            ip = NTOHL(ip);
            
            char buffer[20];
            ip_to_str(buffer,20,ip);      //prevod IP adresy na retezec
            term_send_str("IP adresa: ");
            term_send_str_crlf(buffer);      //vypis na terminal
            break;
         
         case CNAME:                   //kanonicke jmeno
            rx_read(&length,2);
            rx_skip(NTOHS(length));       //preskakuje se
            break;
            
         default:                   //ostatni - ignorovano
            rx_read(&length,2);
            rx_skip(NTOHS(length));
            break;
      }
         
   }
   
   return;
}


//*************************************
// Inicializace  periferii
//*************************************
void fpga_initialized() {
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

   TIMERA_ISR_INIT()                   //inicializace casovace

   dhcp_init();                        //inicializace DHCP
   udp_bind(53800, DNS);               //registrace handleru na port 53800

   term_send_str_crlf("Cekani na odpoved od DHCP");
   while (1){

      terminal_idle();              //obsluha terminalu
      ENC28J60_idle();              //obsluha modulu

      if(dhcp_ready()){
         if (dhcp_flag==0){
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
}

TIMERA_ISR_DHCP()                      //obsluha casovace
