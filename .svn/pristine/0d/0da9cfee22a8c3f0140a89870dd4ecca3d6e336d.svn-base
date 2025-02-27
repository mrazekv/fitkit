/*******************************************************************************
   main.c: HTTP server
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
  term_send_str_crlf("Simple HTTP server");
  term_send_crlf();
  term_send_str_crlf("Po ziskani nastaveni z DHCP serveru je server pristupny pres webove rozhrani.");
  term_send_str_crlf("Na strance je zobrazena aktualni teplota ziskana z MCU.");
}

unsigned char reqcnt;

TCP_APP_HANDLER(tcp_handler){
   
   char buffer[50];
   char temp_str[20];
   
   rx_getline(&buffer,50);
   //term_send_str(buffer);
   
   if( strcmp6(buffer, "GET / ") | strcmp16(buffer, "GET /index.html ") ){
      
      //************************************
      //INDEX.html
      if(tx_init(TCP_PROTO) == 0){                       //inicializace odesilaciho bufferu

         tx_write_str("HTTP/1.0 200 OK\r\n");               //zapis stranky do bufferu
         tx_write_str("Connection: close\r\n");
         tx_write_str("Server: Fitkit 2.0\r\n");
         tx_write_str("Content-Type: text/html\r\n");
         tx_write_str("Content-Length: 190\r\n");
         tx_write_str("\r\n");
         
         
            //hlavicka stranky
         tx_write_str("<html>\r\n<head>\r\n");     //16
         tx_write_str("<title>FITKit DEMO</title>");  //26
         tx_write_str("\r\n</head>\r\n<body>\r\n");   //19
                                          // = 61 
            //telo stranky
         tx_write_str("<h1 align=\"center\">FITKit HTTP server</h1>\r\n"); //44
         tx_write_str("<br><a href=\"teplota.html\">Aktuální teplota namìøená FITKitem</a>\r\n");  //67
            
            //konec stranky
         tx_write_str("</body>\r\n</html>\r\n");         //18
         tx_close();                            //uzavreni bufferu
         tcp_send();                            //odeslani stranky
      }
   }
   else if( strcmp18(buffer, "GET /teplota.html ") ){
      
      if(tx_init(TCP_PROTO) == 0){                       //inicializace odesilaciho bufferu
         tx_write_str("HTTP/1.0 200 OK\r\n");
         tx_write_str("Connection: close\r\n");
         tx_write_str("Server: Fitkit 2.0\r\n");
         tx_write_str("Content-Type: text/html\r\n");
         tx_write_str("Content-Length: 179\r\n");
         tx_write_str("\r\n");
         
            //hlavicka stranky
         tx_write_str("<html>\r\n<head>\r\n");     //16
         tx_write_str("<title>FITKit DEMO</title>");  //26
         tx_write_str("\r\n</head>\r\n<body>\r\n");   //19
                                          //  = 61 
            //telo stranky
         tx_write_str("<p>Teplota CPU je právì: ");   //29
         
         int temperature = thermometer_gettemp();              // zjisteni aktualni teploty
         sprintf (temp_str, "%02d.%01d°C", temperature/10, temperature%10);      //prevod teploty na retezec
         tx_write_str(temp_str);                   //6
         
         tx_write_str("<br>Dotaz cislo: "); //17
         sprintf (temp_str, "%03d", ++reqcnt);
         tx_write_str(temp_str); //3
        
         tx_write_str("</p>    \r\n");                //6

        
         tx_write_str("<br><br><a href=\"index.html\">zpìt</a>\r\n"); //39
         //konec stranky
         tx_write_str("</body>\r\n</html>\r\n");         //18
         tx_close();                            //uzavreni bufferu
         tcp_send();                            //odeslani stranky
      }
   }
   
   else{
      //stranka nenalezena
      if(tx_init(TCP_PROTO) == 0){                       //inicializace odesilaciho bufferu
         tx_write_str("HTTP/1.0 404 ERR\r\n");
         tx_write_str("Connection: close\r\n");
         tx_write_str("Server: Fitkit 2.0\r\n");
         tx_write_str("Content-Type: text/html\r\n");
         tx_write_str("Content-Length: 100\r\n");
         tx_write_str("\r\n");
         
         //hlavicka stranky
         tx_write_str("<html>\r\n<head>\r\n");
         tx_write_str("<title>FITKit DEMO - stránka nenalezena</title>");
         tx_write_str("\r\n</head>\r\n<body>\r\n");
         
         //telo stranky
         tx_write_str("<p>Stránka nenalezena</p>\r\n");
         
         //konec stranky
         tx_write_str("</body>\r\n</html>\r\n");
         tx_close();                            //uzavreni bufferu
         tcp_send();                            //odeslani stranky
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
   ENC28J60_init();  //inicializace ENC28J60
}

//*************************************
// Hlavni smycka programu
//*************************************
int main(void){
   char dhcp_flag = 0;
   char tcp_flag = 0;   
   reqcnt = 0;

   initialize_hardware();
   WDTCTL = WDTPW + WDTHOLD;                 // zastaveni watchdogu

   P1DIR |= LED0;
   P1OUT ^= 0x01;

   TIMERA_ISR_INIT()                //inicializace casovace

   create_tcp_socket(80,tcp_handler);  //vytvoreni socketu a registrace handleru na port 80
   tcp_bind();                         //tcp server - nasloucha na portu 80
   
   //konfigurace pomoci DHCP
   dhcp_init();            

   //konfigurace pomoci staticke adresy
   //set_ip(192,168,137,10);
   //set_netmask(255,255,255,0);
   
   while (1){

      terminal_idle();           // obsluha terminalu
      ENC28J60_idle();           // obsluha ENC28J60
      
      if(dhcp_ready()){
         if (dhcp_flag==0) {
            char ip_str[16];

            term_send_str("DHCP ready, IP adresa: ");
            ip_to_str(&ip_str,16,get_ip());
            term_send_str_crlf(ip_str);

            dhcp_flag = 1;
         }
      }
      else
         dhcp_flag = 0;    
   }

   return 0;   
}

TIMERA_ISR_DHCP_TCP()
