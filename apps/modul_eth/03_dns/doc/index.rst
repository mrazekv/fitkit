.. article:: apps_modeth_dns
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Aplikace ukazuje, jakım zpùsobem lze snadno implementovat protokoly vyšších vrstev.

====================================
Pøevod doménovıch jmen pomocí DNS
====================================

Pøeklad doménovıch jmen na IP adresy a zpìt je jedním z mechanismù, které usnadòují komunikaci v sítích TCP/IP.
Pøeklad je realizovám pomocí DNS (Domain Name System), co je distribuovanı systém skládající se z hierarchicky spojenıch DNS serverù. 

V základní verzi protokol DNS vyuívá slueb protokolu UDP a v rozšíøené verzi TCP (UDP limituje maximální délku paketu a protoe DNS odpovìï mùe tuto délku v urèitıch pøípadech mnohonásobnì pøesahovat, pouívá se TCP kanál). 
Pro jednoduchost byla zvolena varianta vyuívající protokol UDP. 
Pro pøevod doménového jména na IP adresu lze pouít DNS paket typu **DNS QUERY**, kterı obsahuje název domény s dalšími parametry a tento paket odeslat na DNS server na port èíslo 53. 
Jakmile obdríme odpovìï, je nutné odpovìï vyhodnotit. Odpovìï mùe bıt positivní èi negativní. 
V pøípadì negativní odpovìdi server informuje, e dané doménové jméno neexistuje nebo e je nezná. 

Popis aplikace
=================
V hlavním programu nakonfigurujeme zaøízení tak, aby vyuívalo slueb DHCP serveru. 
Pomocí DHCP získámì automaticky IP adresu v lokální síti z DHCP serveru a vyhneme se tak nutnosti ji konfigurovat ruènì. 
Jednou z informací, která je typicky souèástí DHCP odpovìdi, je adresa DNS serveru, kterı je schopen pøekládat doménové jména na IP adresy a naopak. 

Následuje volání funkce ``udp_bind(53800,dns)``, která zaregistruje callback funkci pojmenovanou jako ``dns``, která bude vyvolána pøi pøíchodu UDP paketu s cílovım portem èíslo 53800 (èíslo cílového portu lze libovolnì zmìnit). 
V hlavní smyèce je postupnì volána funkce ``ENC28J60_idle()`` a následnì testován stav DHCP pomocí funkce ``dhcp_ready()``.

Jakmile máme pøiøazenu IP adresu, je moné zadáním pøíkazu ``QUERY`` následovaného kvalifikovanım doménovım jménem do terminálu pøeloit doménové jméno na IP adresu.
Tento pøíkaz vyvolá postupnì následující sekvenci. Pomocí ``tx_init(UDP_PROTO)`` se zinicializuje UDP paket v odesílacím bufferu, poté se zapíše pomocí volání funkce ``tx_write(&dns_header, DNS_HEADER_LEN)`` DNS paket s pøíslušnım dotazem do bufferu. 
Zápis je následován dokonèením paketu voláním funkce ``tx_close()`` a odesláním UDP paketu na port èíslo. 
Odeslání je realizováno pomocí volání ``udp_send(53, 53800, get_dns_server())``. 
Funkce ``get_dns_server()`` vrací IP adresu DNS serveru, která byla získána z DHCP odpovìdi.
Pøi pøíchodu odpovìdi na port èíslo 53800 je naètena odpovìï do struktury dns_header pomocí volání funkce ``rx_read()`` a následuje zjištìní, zda-li paket obsahuje informace o našem dotazu èi nikoliv. 
Stav a zjištìná IP adresa jsou vypsány do terminálu.

Zprovoznìní aplikace
========================
1. pøipojte k FITkitu rozšiøující modul Ethernet obsahující ENC28J60 a pøipojte modul do sítì LAN   
2. pøelote aplikaci
3. naprogramujte MCU a FPGA a spuste terminálovı program.
4. jakmile aplikace obdrí odpovìï od DHCP serveru, je moné pouít pøíkaz QUERY

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
