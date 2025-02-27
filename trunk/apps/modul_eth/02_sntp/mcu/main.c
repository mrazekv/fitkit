/*******************************************************************************
   main.c: Synchronizace casu
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

#define SNTP_HEADER_LEN 48

//NTP hlavicka
struct sntp_h{
   unsigned char mode:3;
   unsigned char vn:3;
   unsigned char li:2;
   unsigned char stratum;
   unsigned char poll;
   unsigned char precision;
   unsigned long root_delay;
   unsigned long root_dispersion;
   unsigned long reference_identifier;
   unsigned long reference_timestamp;
   unsigned long reference_timestamp2;
   unsigned long originate_timestamp;
   unsigned long originate_timestamp2;
   unsigned long receive_timestamp;
   unsigned long receive_timestamp2;
   unsigned long transmit_timestamp;
   unsigned long transmit_timestamp2;
};

//*************************************
// Tisk napovedy
//*************************************
void print_user_help(void){
  term_send_str_crlf("Synchronizace casu pomoci SNTP");
  term_send_str_crlf("==============================");
  term_send_str_crlf("Aplikace ziskava aktualni cas v UTC ze SNTP serveru ntp.nic.cz");
  term_send_str_crlf("Cas je v UTC, bez vlivu casovych pasem a letniho/zimniho casu.");
  term_send_crlf();
  term_send_str_crlf("GET TIME ... odesle pozadavek na ziskani aktualniho casu");
}

//*************************************
// Dekodovani prikazu
//*************************************
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd){

   if(dhcp_ready()){
      if (strcmp8(cmd_ucase, "GET TIME") ){
         if(tx_init(UDP_PROTO) == 0){                    //inicializace odesilaciho bufferu
                        
            term_send_str_crlf("Odesilam SNTP paket");

            struct sntp_h sntp_header;                   //vytvoreni hlavicky
            enc_memset(&sntp_header,0,SNTP_HEADER_LEN);
            sntp_header.vn = 4;                          //zapsani hodnot do hlavicky - verze NTP
            sntp_header.mode = 3;                        //rezim klient
            tx_write(&sntp_header,SNTP_HEADER_LEN);            //zapis hlavicky do bufferu
            tx_close();                               //uzavreni bufferu
            udp_send(123, 123, CHAR2IP(217,31,205,226));    //server ntp.nic.cz 

         }
         return USER_COMMAND; 
      }
   } else {
      term_send_str_crlf("DHCP neni pripraveno");
   }
   
   return CMD_UNKNOWN;
}


#define sec_per_year    31536000UL              //predpocitane hodnoty
#define sec_per_leap_year  31622400UL

#define sec_per_day        86400UL
#define MINS            60
#define HOURS           24
#define mins_per_hours     3600
#define MONTHS          12
#define MAX_YEAR        2036
   
unsigned char not_leap_year[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};       //pocty dnu v mesicich normalniho roku
unsigned char leap_year[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};           // --||-- prestupneho roku

//*************************************
// Obsluha prichoziho datagramu na portu 123 (SNTP)
//*************************************
UDP_APP_HANDLER(SNTP){

   struct sntp_h header;
   rx_read(&header,SNTP_HEADER_LEN);

   unsigned long seconds = NTOHL(header.transmit_timestamp);//vteriny od 1.1.1900
   
   unsigned int year = 1900;                 //inicializace promennych
   unsigned char month = 1;
   unsigned char day = 1;
   unsigned char hour = 0;
   unsigned char min = 0;
   unsigned char sec = 0;
   unsigned char leap = 0;
   
      //odecet roku a prechodnych roku
   while (year <= MAX_YEAR){
      if(leap == 4)                          //prestupny rok
         if(seconds - sec_per_leap_year > seconds) //promenna seconds je unsigned -> kontrola podteceni
            break;
         else{
            seconds -= sec_per_leap_year; 
            year++;
            leap = 1;
         }
      else{                               //neprestupny rok
         if(seconds - sec_per_year > seconds)
            break;
         else{
            seconds -= sec_per_year;   
            year++;
            leap++;
         }
      }
   }
      //odecet mesicu
   while (month <= MONTHS){

      if(leap == 4)                 //mesice prestupneho roku
         if(seconds - (sec_per_day * leap_year[month]) > seconds)
            break;
         else{
            seconds -= (sec_per_day * leap_year[month]); 
            month++;
         }
      else{                      //mesice neprestupneho roku
         if(seconds - (sec_per_day * not_leap_year[month]) > seconds)
            break;
         else{
            seconds -= (sec_per_day * not_leap_year[month]);   
            month++;
         }
      }
   }

         
      //odecet dnu - prestupny rok
   if(leap == 4){
      while (day <= leap_year[month]){
         if((seconds - sec_per_day) > seconds)
            break;
         else{
            seconds -= sec_per_day; 
            day++;
         }
      }
   }
      //odecet dnu - neprestupny rok
   else{
      while (day <= not_leap_year[month]){

         if((seconds - sec_per_day) > seconds)
            break;
         else{
            seconds -= sec_per_day; 
            day++;
         }
      }
   }  
      //odecet hodin
   while (hour < HOURS){

         if((seconds - mins_per_hours) > seconds)
            break;
         else{
            seconds -= mins_per_hours; 
            hour++;
         }
      }
      //odecet minut
   while (min < MINS){

         if((seconds - MINS) > seconds)
            break;
         else{
            seconds -= MINS;  
            min++;
         }
      }
      
      //zbyly vteriny
   sec = seconds;
   
   //cas je v UTC
   //vypis na terminal
   term_send_str("Aktualni cas: ");
   term_send_num(day);
   term_send_str(".");
   term_send_num(month);
   term_send_str(".");
   term_send_num(year);
   term_send_str(" ");

   if(hour<10)
      term_send_char('0');
   term_send_num(hour);
   term_send_str(":");
   
   if(min<10)
      term_send_char('0');
   term_send_num(min);
   term_send_str(":");
   
   if(sec<10)
      term_send_char('0');
   term_send_num(sec);
   
   term_send_str(" UTC");
   term_send_crlf();
   
   return;
}


//*************************************
// Inicializace  periferii
//*************************************
void fpga_initialized(){
   ENC28J60_init();  //inicializace ENC28J60
}

interrupt (TIMERA0_VECTOR) Timer_A (void){      //preruseni kazdou 1s
   dhcp_timer();       //volani DHCP casovace
   CCR0 += 0x8000;     // preruseni kazdou 1sec
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

   //ukazka inicializace casovace a obsluhy udalosti (funkce Timer_A)
   CCTL0 = CCIE;                       // povoleni preruseni pro casovac, rezim vystupni komparace 
   CCR0 = 0x8000;                      // 32768 = 0x8000; preruseni kazdou 1s
   TACTL = TASSEL_1 + MC_2;            // ACLK (f_tiku = 32768 Hz = 0x8000 Hz), nepretrzity rezim

   dhcp_init();                        //inicializace DHCP
   udp_bind(123,SNTP);                    //registrace handleru na port 123

   term_send_str_crlf("Cekani na odpoved od DHCP");
   while (1){

      terminal_idle();              //obsluha terminalu
      ENC28J60_idle();              //obsluha modulu

      if(dhcp_ready()){
         if (dhcp_flag==0) {
            dhcp_flag = 1;
                        
            char ip_str[16];
            term_send_str("DHCP ready, IP:");
            ip_to_str(&ip_str,16,get_ip());     //prevod IP na retezec
            term_send_str_crlf(ip_str);         //vypis prirazene IP

            decode_user_cmd("GET TIME","GET TIME");
         }

      }
      else{
         dhcp_flag = 0;
      }
   }

}

