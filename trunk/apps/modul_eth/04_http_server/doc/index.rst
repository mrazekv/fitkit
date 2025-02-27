.. article:: apps_modeth_http_server
    :author: Zdenìk Vašíèek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Aplikace ukazuje jak je moné vyrobit jednoduchı webovı server.

========================
Webovı server
========================
Tato úloha ukazuje, jak lze vyuít knihovnu libenc28j60 tak, abychom na FITkitu vytvoøili jednoduchı HTTP server, kterı obsahuje dvì www stránky. 
Stránku index.htm, která obsahuje základní informace a odkaz na stránku teplota.htm, stránka teplota.htm zobrazuje aktuální stav interního teplotního èidla.

Webové stránky jsou pøenášeny pomocí pomìrnì jednoduchého textového protokolu oznaèovaného jako HTTP (Hyper Text Transfer Protocol).
Staení webové stránky probíhá následovnì. 
Nejprve je ustaveno TCP spojení na HTTP server, kterı bìí obvykle na portu èíslo 80. 
Poté je odeslán tzv. **GET** request, kterı spoèívá v odeslání textu napø. ``GET /index.htm HTTP/1.1`` následovaného dvìmy odøádkováními (CR LF). 
Server odpoví a vrátí stránku index.htm nacházející se v koøenovém adresáøi. Pokud stránka neexistuje, vrátí chybu.
Odpovìï se skládá ze dvou èásti, hlavièky a  vlastních dat. Hlavièka obsahuje informace jako je stavovı kód, datum poslední modifikace, apod.

Popis aplikace
=================
Ve funkci ``main()`` je nainicializován DHCP klient tak, abychom získali IP adresu z DHCP serveru a nemuseli ji konfigurovat ruènì. 
V poznámce je uveden kód umoòující té nastavit statickou IP adresu. 
Následuje volání funkce ``create_tcp_socket(80,tcp_handler)`` a ``tcp_bind()``, která zaregistruje callback funkci pojmenovanou ``tcp_handler``, která bude vyvolána pøi pøíchodu TCP paketu s cílovım portem èíslo 80. 
Funkce ``tcp_hadler`` dostává pøímo data, která jsou pøenášena v TCP paketu; o ustavení TCP spojení, jeho ukonèení a potvrzování paketù se stará navrená knihovna. 
Pro zjednodušení rutina ``tcp_handler`` obsahuje krátkı buffer a provádí jednoduché porovnání poadavku pomocí funkce strcmp. 

V hlavní smyèce je postupnì volána funkce ``ENC28J60_idle()`` a následnì testován stav DHCP a TCP socketu pomocí funkce ``tcp_connected()``.

Zprovoznìní aplikace
========================
1. pøipojte k FITkitu rozšiøující modul Ethernet obsahující ENC28J60 a pøipojte modul do sítì LAN   
2. pøelote aplikaci
3. naprogramujte MCU a FPGA a spuste terminálovı program.
4. jakmile aplikace obdrí odpovìï od DHCP serveru, je moné tuto IP adresu zadat v prohlíeèi

Aplikace nevyaduje ke své èinnosti nastavit kromì propojek umoòujících programování další propojky.
