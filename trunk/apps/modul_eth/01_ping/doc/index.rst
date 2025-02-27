.. article:: apps_modeth_ping
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Jednoduchá aplikace demonstrující vyuití knihovních funkcí pro implementaci funkce PING.

========================
Protokol ICMP a PING
========================
Jednou ze základních demonstraèních úloh je implementace zjednodušené verze pøíkazu ping, pomocí kterého lze napøíklad ovìøit dostupnost urèitého poèítaèe v síti. 
Pøíkaz vyuívá slueb protokolu ICMP, kterı je urèen k oznamování chybovıch stavù jako je napøíklad nedostupnost sluby, nedosaitelnost poèítaèe/smìrovaèe, ale kterı té podporuje tzv. echo request, pøi kterém je odeslán zpìt odesílateli paket s totonım datovım obsahem. 

Popis aplikace
=================
Aplikace se skládá z inicializaèní èásti (main) a dále funkce obsluhující pøijate ICMP poadavky. 
V hlavní èásti je nainicializován DHCP klient tak, abychom získali IP adresu z DHCP serveru a nemuseli ji konfigurovat ruènì. 
Poté následuje volání funkce `icmp_bind(icmp_handler)`, která zaregistruje callback funkci `icmp_handler`, která bude vyvolána pøi kadém pøíchodu ICMP paketu. 
V hlavní smyèce je postupnì volána funkce `ENC28J60_idle()` a následnì testován stav DHCP pomocí funkce `dhcp_ready()`.
Obsluha DHCP je realizována pomocí èasovaèe, pro jeho inicializaci a obsluhu slouí makra `TIMERA_ISR_INIT` a `TIMERA_ISR_DHCP`.

Pøi zadání valdiní IP adresy do terminálu se vyvolá postupnì následující sekvence pøíkazù. 
Pomocí `tx_init(ICMP_PROTO)` se zinicializuje ICMP paket v odesílacím bufferu, poté se zapíše 23B dat pomocí `tx_write_str("abcdefghijklmnopqrstuvw")` a následuje dokonèení paketu funkcí `tx_close()` a odeslání ICMP poadavku typu ICMP ECHO pomocí volání funkce `icmp_send(dest_ip,ICMP_ECHO,ICMP_ECHO_CODE,1,1)`. 

Pøi pøíchodu odpovìdi je vyvolána callback funkce `icmp_handler`, která otestuje ICMP hlavièku a vypíše IP adresu, která generovala odpovìï. 
Pøi pøíchodu poadavku na PING je vypsána do terminálu IP adresa odesílatele.
Poadavek stejnì tak jako ostatní ICMP pakety jsou zahozeny.

Zprovoznìní aplikace
========================
1. pøipojte k FITkitu rozšiøující modul Ethernet obsahující ENC28J60 a pøipojte modul do sítì LAN   
2. pøelote aplikaci
3. naprogramujte MCU a FPGA a spuste terminálovı program.
4. jakmile aplikace obdrí odpovìï od DHCP serveru, je moné pouít PING

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
