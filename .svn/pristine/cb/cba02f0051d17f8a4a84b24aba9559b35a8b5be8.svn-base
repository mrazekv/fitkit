.. article:: apps_modeth_sntp
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Jednoduchá aplikace umoòující zjistit aktuální èas prostøednictvím sítì Ethernet

=================================
Synchronizace èasu pomocí SNTP
=================================

Tato aplikace umoòuje synchronizaci èasu pomocí protokolu SNTP (Simple Network Time Protocol), co je zjednodušená verze NTP protokolu, která je urèená pro embedded zaøízení. 
Protokol NTP je navren tak, aby byl schopen zajistit, aby všechny poèítaèe v síti obdreli stejnou èasovou informaci i pøesto, e v doruèování paketù nástávají obecnì promìnlivá zpodìní. 
K tomuto úèelu se pouívá iterativní dotazování.
SNTP protokol pracuje nad protokolem UDP a nevyaduje proto sloité operace.
Nejjednodušší monost, jak získat aktuální èas je odeslání SNTP paketu s ádostí o aktuální stav na port s èíslem 123 a dle odpovìdi získat potøebné údaje (napø. datum, èas apod). 
K zjištìní potøebnıch údajù se pouívá SNTP server ntp.nic.cz.

Popis aplikace
=================
V hlavní funkci main aplikace nejprve nainicializuje DHCP klienta tak, abychom získali IP adresu v lokální síti z DHCP serveru a nemuseli ji konfigurovat ruènì. 
Poté následuje volání funkce `udp_bind(123, sntp)`, která zaregistruje callback funkci pojmenovanou `sntp`, která bude vyvolána pøi pøíchodu UDP paketu s cílovım portem èíslo 123 (èíslo cílového portu lze libovolnì zmìnit). 
V hlavní smyèce je postupnì volána funkce `ENC28J60_idle()` a následnì testován stav DHCP pomocí funkce `dhcp_ready()`.

Pøi zadání pøíkazu `GET TIME` do terminálu se vyvolá postupnì následující sekvence pøíkazù. 
Pomocí `tx_init(UDP_PROTO)` se zinicializuje UDP paket v odesílacím bufferu, poté se zapíše pomocí volání funkce `tx_write(&sntp_header, SNTP_HEADER_LEN)` SNTP paket do bufferu.
Zápis je následován dokonèením paketu voláním funkce `tx_close()` a odesláním UDP paketu na port èíslo 123 (zdrojovı port jsme nastavili té na hodnotu 123). 
Odeslání je realizováno pomocí volání `udp_send(123, 123, CHAR2IP(217,31,205,226))`. 
Makro CHAR2IP pøevádí IP adresu zadanou jako ètveøici bytù na èíslo typu long int.
Pøi pøíchodu odpovìdi na port èíslo 123 je naètena odpovìï do struktury sntp_header pomocí volání funkce `rx_read` a následuje zjištìní informací o datu a èase a jejich vıpis do terminálu.

Zprovoznìní aplikace
========================
1. pøipojte k FITkitu rozšiøující modul Ethernet obsahující ENC28J60 a pøipojte modul do sítì LAN   
2. pøelote aplikaci
3. naprogramujte MCU a FPGA a spuste terminálovı program
4. jakmile aplikace obdrí odpovìï od DHCP serveru, je odeslán poadavek na aktuální èas

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
