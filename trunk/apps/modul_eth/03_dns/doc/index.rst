.. article:: apps_modeth_dns
    :author: Zden�k Va���ek <vasicek AT fit.vutbr.cz>
    :updated: 20110101

    Aplikace ukazuje, jak�m zp�sobem lze snadno implementovat protokoly vy���ch vrstev.

====================================
P�evod dom�nov�ch jmen pomoc� DNS
====================================

P�eklad dom�nov�ch jmen na IP adresy a zp�t je jedn�m z mechanism�, kter� usnad�uj� komunikaci v s�t�ch TCP/IP.
P�eklad je realizov�m pomoc� DNS (Domain Name System), co� je distribuovan� syst�m skl�daj�c� se z hierarchicky spojen�ch DNS server�. 

V z�kladn� verzi protokol DNS vyu��v� slu�eb protokolu UDP a v roz���en� verzi TCP (UDP limituje maxim�ln� d�lku paketu a proto�e DNS odpov�� m��e tuto d�lku v ur�it�ch p��padech mnohon�sobn� p�esahovat, pou��v� se TCP kan�l). 
Pro jednoduchost byla zvolena varianta vyu��vaj�c� protokol UDP. 
Pro p�evod dom�nov�ho jm�na na IP adresu lze pou��t DNS paket typu **DNS QUERY**, kter� obsahuje n�zev dom�ny s dal��mi parametry a tento paket odeslat na DNS server na port ��slo 53. 
Jakmile obdr��me odpov��, je nutn� odpov�� vyhodnotit. Odpov�� m��e b�t positivn� �i negativn�. 
V p��pad� negativn� odpov�di server informuje, �e dan� dom�nov� jm�no neexistuje nebo �e je nezn�. 

Popis aplikace
=================
V hlavn�m programu nakonfigurujeme za��zen� tak, aby vyu��valo slu�eb DHCP serveru. 
Pomoc� DHCP z�sk�m� automaticky IP adresu v lok�ln� s�ti z DHCP serveru a vyhneme se tak nutnosti ji konfigurovat ru�n�. 
Jednou z informac�, kter� je typicky sou��st� DHCP odpov�di, je adresa DNS serveru, kter� je schopen p�ekl�dat dom�nov� jm�na na IP adresy a naopak. 

N�sleduje vol�n� funkce ``udp_bind(53800,dns)``, kter� zaregistruje callback funkci pojmenovanou jako ``dns``, kter� bude vyvol�na p�i p��chodu UDP paketu s c�lov�m portem ��slo 53800 (��slo c�lov�ho portu lze libovoln� zm�nit). 
V hlavn� smy�ce je postupn� vol�na funkce ``ENC28J60_idle()`` a n�sledn� testov�n stav DHCP pomoc� funkce ``dhcp_ready()``.

Jakmile m�me p�i�azenu IP adresu, je mo�n� zad�n�m p��kazu ``QUERY`` n�sledovan�ho kvalifikovan�m dom�nov�m jm�nem do termin�lu p�elo�it dom�nov� jm�no na IP adresu.
Tento p��kaz vyvol� postupn� n�sleduj�c� sekvenci. Pomoc� ``tx_init(UDP_PROTO)`` se zinicializuje UDP paket v odes�lac�m bufferu, pot� se zap�e pomoc� vol�n� funkce ``tx_write(&dns_header, DNS_HEADER_LEN)`` DNS paket s p��slu�n�m dotazem do bufferu. 
Z�pis je n�sledov�n dokon�en�m paketu vol�n�m funkce ``tx_close()`` a odesl�n�m UDP paketu na port ��slo. 
Odesl�n� je realizov�no pomoc� vol�n� ``udp_send(53, 53800, get_dns_server())``. 
Funkce ``get_dns_server()`` vrac� IP adresu DNS serveru, kter� byla z�sk�na z DHCP odpov�di.
P�i p��chodu odpov�di na port ��slo 53800 je na�tena odpov�� do struktury dns_header pomoc� vol�n� funkce ``rx_read()`` a n�sleduje zji�t�n�, zda-li paket obsahuje informace o na�em dotazu �i nikoliv. 
Stav a zji�t�n� IP adresa jsou vyps�ny do termin�lu.

Zprovozn�n� aplikace
========================
1. p�ipojte k FITkitu roz�i�uj�c� modul Ethernet obsahuj�c� ENC28J60 a p�ipojte modul do s�t� LAN   
2. p�elo�te aplikaci
3. naprogramujte MCU a FPGA a spus�te termin�lov� program.
4. jakmile aplikace obdr�� odpov�� od DHCP serveru, je mo�n� pou��t p��kaz QUERY

Aplikace nevy�aduje ke sv� �innosti nastavit krom� propojek umo��uj�c�ch programov�n� dal�� propojky.
